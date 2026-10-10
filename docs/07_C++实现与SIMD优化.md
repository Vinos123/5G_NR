# 07 · C++ 参考实现与 SIMD 指令集优化

> 目标：讲清 `code/` 工程结构，以及 PHY 层代码最值钱的「指令级优化」怎么落地。
> 本机 Apple Silicon(arm64) → **NEON 可编译可跑**；AVX-512(x86)/SVE(ARM服务器)/AMX 用 intrinsics 讲解 + 宏隔离。

---

## 7.1 工程结构

```
code/
├── include/nrphy/           # 头文件（每个模块一个）
│   ├── types.h              # 比特/复符号类型 + 约定
│   ├── crc.h  gold.h  scrambler.h  modulator.h
│   ├── layer_map.h  transform_precoder.h  precode.h  re_map.h
│   ├── ldpc.h  ldpc_base_graph.h  rate_match.h  segmentation.h
│   └── pusch.h              # 顶层 14 步串联
├── src/                     # 实现（+ pusch_tx_demo.cpp 端到端示例）
├── bench/                   # scalar vs SIMD 基准
└── test/                    # 单元测试（ctest）
```

**模块 ↔ 步骤对照**：

| 模块 | 步骤 | 规范 |
|---|---|---|
| `crc` | 1 | 38.212 §6.2.1 |
| `segmentation` (含 `select_base_graph`) | 2,3 | 38.212 §6.2.2/6.2.3 |
| `ldpc` | 4 | 38.212 §6.2.4 |
| `rate_match` | 5 | 38.212 §6.2.5 |
| `pusch` (concat) | 6,7 | 38.212 §6.2.6/6.2.7 |
| `scrambler` / `gold` | 8 | 38.211 §6.3.1.1 |
| `modulator` | 9 | 38.211 §6.3.1.2 |
| `layer_map` | 10 | 38.211 §6.3.1.3 |
| `transform_precoder` | 11 | 38.211 §6.3.1.4 |
| `precode` | 12 | 38.211 §6.3.1.5 |
| `re_map` | 13,14 | 38.211 §6.3.1.6/6.3.1.7 |

构建与测试：

```bash
cmake -B code/build -S code && cmake --build code/build -j
ctest --test-dir code/build                          # 9 个单元测试
./code/build/pusch_tx_demo                            # 端到端 14 步
./code/build/{scrambler,modulator,crc}_bench          # 基准
```

---

## 7.2 参考实现的取舍：bit-per-byte

参考实现里比特用 `vector<uint8_t>` 存，**每个元素一个比特（0/1）**——这是「最清晰」但「内存浪费 8x」的表示。好处是每个步骤的代码和规范公式一一对应、易读易验。

> **关键认知**：真实产品代码不会这么存。第一个也是最大的优化就是「**打包**」——8 比特塞进 1 字节，内存和带宽直接 ÷8。见 7.4 的基准。

---

## 7.3 三个正交的优化维度（先建立心智模型）

1. **数据布局（Data Layout）**：bit-per-byte → 8bit/byte 打包；SoA vs AoS（复数数组）。**往往比手写 SIMD 更值钱**。
2. **查表（LUT）**：把「算」换成「查」。调制映射、CRC 表、速率匹配的位操作都适用。
3. **SIMD（向量化）**：一条指令算多份数据。NEON(128b)/AVX-512(512b)/SVE(可变长)/AMX(矩阵)。

**教训（来自 bench）**：编译器会**自动向量化**简单的循环（比如逐字节 XOR），手写 NEON 收益有限（~1x）。真正的 SIMD 价值在编译器**不会自动向量化**的地方：gather/scatter、位操作、复数乘、查表 shuffle。

---

## 7.4 三个基准（`code/bench/`，本机可跑）

### 7.4.1 加扰 XOR（`scrambler_bench`）

```
NEON == scalar ? YES
packed == scalar ? YES
scalar(bit/byte): 84 µs,  NEON: 38 µs (2.2x),  packed(8bit/byte): 7 µs (11.6x)
```

- **NEON 16 字节一次**：`veorq_u8(vld1q_u8(a), vld1q_u8(b))`——但简单 XOR 编译器已自动向量化，手写≈自动。
- **打包 8bit/byte**：内存访问 ÷8，这才是 11.6x 的来源。

### 7.4.2 调制 LUT（`modulator_bench`）

```
QPSK 3.1x   16QAM 1.8x   64QAM 1.3x   256QAM 1.3x
```

把嵌套 Gray 公式换成「预计算 2^Qm 项的表 + 一次数组查找」。Qm 越大表越大（256QAM 是 256 项），收益趋于平稳，但 64/256QAM 里「位重排 + 查表」仍是标准做法。SIMD 版可用 NEON `vtbl`（8 位索引查表）一次查 16 个符号。

### 7.4.3 CRC 表（`crc_bench`）

```
bitwise: 625 µs,  table: 143 µs (4.4x)
```

逐位 → 字节表（一次处理 8 bit）。更高阶是 **无进位乘法并行 CRC**：x86 `PCLMULQDQ` / arm64 `vmull_p64`，可一次处理 128/512 bit。这是「编译器不会自动做」的典型——数据依赖链短、无进位乘是专用指令。

---

## 7.5 各步骤的 SIMD 化路线（对齐岗位 JD）

| 步骤/模块 | 优化手法 | 指令 |
|---|---|---|
| 加扰 | 打包 + 按 128/512 bit XOR | NEON / AVX-512 |
| Gold 序列生成 | 位并行 LFSR（一次推 128 bit） | 矩阵化状态转移 |
| CRC | 字节表 → 无进位乘法并行 | PCLMULQDQ / `vmull_p64` |
| 调制 | LUT + shuffle 查表 | NEON `vtbl` / AVX-512 `_mm512_permutexvar` |
| 速率匹配 | 环形缓冲 gather/scatter、字节化 | 位操作 + 掩码 |
| 预编码 | 复数乘加 | NEON `vcmlaq` / AVX-512 `_mm512_fmadd` |
| 变换预编码 DFT | FFT 库（已 SIMD） | FFTW / Intel MKL / KissFFT |
| LDPC 译码（收端） | 稀疏矩阵 + 软信息 | **AMX**（矩阵 tile）最有价值 |

---

## 7.6 AVX-512 / SVE / AMX 代码对照（学习参考，宏隔离）

### 加扰 XOR

```cpp
// NEON (arm64, 本机可跑)
#include <arm_neon.h>
for (; i + 16 <= n; i += 16) {
    uint8x16_t a = vld1q_u8(in + i), b = vld1q_u8(seq + i);
    vst1q_u8(out + i, veorq_u8(a, b));
}

// AVX-512 (x86 服务器，交叉编译目标)
#include <immintrin.h>
for (; i + 64 <= n; i += 64) {
    __m512i a = _mm512_loadu_si512(in + i), b = _mm512_loadu_si512(seq + i);
    _mm512_storeu_si512(out + i, _mm512_xor_si512(a, b));
}

// SVE (ARM 服务器，可变向量长，写一次适配 128~2048 bit)
#include <arm_sve.h>
for (; i + svcntb() <= n; i += svcntb()) {
    svbool_t pg = svwhilelt_b8(i, n);
    svst1_u8(pg, out + i, sveor_u8_m(pg, svld1_u8(pg, in + i), svld1_u8(pg, seq + i)));
}
```

### 预编码复数乘（NEON 复数乘加）

```cpp
// z = x * w（逐元素复数乘）：NEON 的 vcmlaq 一条指令算 2 个复数乘加
float32x4_t xr = ..., xi = ..., wr = ..., wi = ...;
// (xr + j xi)(wr + j wi) = (xr wr - xi wi) + j(xr wi + xi wr)
float32x4_t rr = vmulq_f32(xr, wr), ii = vmulq_f32(xi, wi);
float32x4_t ri = vmulq_f32(xr, wi), ir = vmulq_f32(xi, wr);
float32x4_t zr = vsubq_f32(rr, ii), zi = vaddq_f32(ri, ir);
// 或用 vcmlaq（complex multiply accumulate）更省指令
```

### AMX（LDPC 译码的矩阵加速，概念）

Intel AMX 提供 `_tile_*` 内建：把「校验节点 × 变量节点」的稀疏更新表达成 tile 矩阵乘，一条 `tdpbf16ps` 算一个 16×N tile。这是收端 LDPC BP 译码吞吐的关键（对应 JD「AMX 优化经验」）。

> **平台判断宏**：`__ARM_NEON` / `__AVX512F__` / `__ARM_FEATURE_SVE`，编译期隔离。本机 arm64 走 NEON，x86 目标交叉编译走 AVX-512。

---

## 7.7 基准方法论（写 benchmark 的纪律）

1. **正确性先行**：SIMD 结果必须 bit-exact == 标量（bench 里都校验了）。
2. **防死代码消除**：结果写入**持久缓冲**或做 checksum，否则编译器把整个函数删掉（本教程调制 bench 踩过的坑）。
3. **数据布局**是头号变量，先想「能不能少读内存」，再想「能不能并行」。
4. **对比编译器自动向量化**：先开 `-O3` 看编译器是否已向量化，再决定是否手写 intrinsics。

---

## 7.8 一句话总结

> **先打包，再查表，最后 SIMD。** 顺序不能反——数据布局的收益（8x）远大于在错误布局上硬写 SIMD（~1x）。
