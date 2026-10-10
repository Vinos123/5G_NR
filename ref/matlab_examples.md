# MATLAB 5G Toolbox 官方示例对照（golden 参照）

MATLAB 的 **5G Toolbox** 完整实现了 38.211/212/213/214 的发收链，是你 C++ 实现的**权威 golden 参照**：跑同一个参数，逐点对比输出即可验证。

> 本教程 C++ 实现不用 Python/MATLAB 运行，仅把 MATLAB 作为**验证参照**。若你机器有 MATLAB + 5G Toolbox，可用下面的函数对拍。

## 关键函数

| MATLAB 函数 | 对应本教程 | 规范 |
|---|---|---|
| `nrCRCEncode` / `nrCRCDecode` | `crc.cpp` | 38.212 §5.1 |
| `nrCodeBlockSegmentLDPC` | `segmentation.cpp` | 38.212 §6.2.3 |
| `nrLDPCEncode` / `nrLDPCDecode` | `ldpc.cpp` | 38.212 §5.3.2 |
| `nrRateMatchLDPC` / `nrRateRecoverLDPC` | `rate_match.cpp` | 38.212 §5.4.2 |
| `nrPUSCH` / `nrPUSCHDecode` | `pusch.cpp`（顶层） | 38.211 §6.3.1 + 38.212 §6.2 |
| `nrPUSCHDMRS` | （收端信道估计用） | 38.211 §6.4.1.1 |
| `nrTBS` | （调度） | 38.214 §6.1.4.2 |

## 官方示例（文档内搜索这些标题）

- **"5G NR PUSCH Throughput"** —— PUSCH 端到端吞吐仿真，含链路级全部参数。
- **"5G NR PUSCH and PDSCH Throughput"** —— 上下行对照。
- **"Model 5G NR Communication Links"** —— 链路抽象。
- **"Generate Uplink DM-RS Symbols"**（`nrPUSCHDMRS` 文档）—— DMRS 图样。

## 对拍脚本思路（MATLAB 侧）

```matlab
% 用与本教程相同的参数跑一次 PUSCH，导出中间结果与 C++ 对比
nPRB = 2; tbs = 48; mod = 'QPSK';
[~, info] = nrPUSCHIndices( ... );     % RE 位置
sym = nrPUSCH(cw, mod, 1, nlayers, ...);% PUSCH 符号
```

> 注意：MATLAB 内部比特/符号的字节序、索引起点（0/1-based）与 C++ 不同，对拍时先对齐「比特序」和「RE 索引」约定。

## 为什么用 MATLAB 而非手算 golden

LDPC 编码、速率匹配、TBS 查表这些步骤参数多、手算 golden vector 极易错。MATLAB 5G Toolbox 是 3GPP 的「可执行规范」，用它生成 golden、用本教程 C++ 逐步对拍，是**最省力且最可信**的验证路径。

> 本教程已有的自验证（不依赖 MATLAB）：
> - CRC：`crc(msg||parity)==0` + 手工向量；
> - Gold/加扰：独立递推重算 + 自逆；
> - 调制/层映射/变换预编码：手工数学向量；
> - LDPC：独立构建 H，校验 `H·cᵀ==0`；
> - 分段：srsRAN 金标准参数；
> - 速率匹配：环形缓冲/交织结构校验。
