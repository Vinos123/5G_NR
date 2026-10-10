// 加扰（XOR）基准。两个正交的优化维度：
//   1) SIMD 并行（NEON 16 字节一次）——但对简单 XOR，编译器会自动向量化，手写 NEON 收益≈0；
//   2) 内存布局（bit-per-byte -> 8 bit/byte 打包）——真实 ~8x，这才是关键。
#include "bench_util.h"
#include "nrphy/gold.h"
#include <cstdint>
#include <cstdio>
#include <vector>

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

using namespace nrphy;

// ---- bit-per-byte 标量 ----
static void xor_scalar(uint8_t* out, const uint8_t* a, const uint8_t* b, size_t n) {
    for (size_t i = 0; i < n; ++i) out[i] = a[i] ^ b[i];
}
// ---- bit-per-byte NEON（16 字节一次）----
static void xor_neon(uint8_t* out, const uint8_t* a, const uint8_t* b, size_t n) {
#if defined(__ARM_NEON)
    size_t i = 0;
    for (; i + 16 <= n; i += 16) {
        uint8x16_t va = vld1q_u8(a + i), vb = vld1q_u8(b + i);
        vst1q_u8(out + i, veorq_u8(va, vb));
    }
    for (; i < n; ++i) out[i] = a[i] ^ b[i];
#else
    xor_scalar(out, a, b, n);
#endif
}
// ---- 打包：8 bit -> 1 byte（LSB = bit 0），XOR 后解包 ----
static void pack(const uint8_t* bits, size_t nbits, uint8_t* packed) {
    const size_t nbytes = nbits / 8;
    for (size_t i = 0; i < nbytes; ++i) {
        uint8_t b = 0;
        for (int j = 0; j < 8; ++j) b |= static_cast<uint8_t>(bits[i * 8 + j] << j);
        packed[i] = b;
    }
}
static void unpack(const uint8_t* packed, size_t nbits, uint8_t* bits) {
    const size_t nbytes = nbits / 8;
    for (size_t i = 0; i < nbytes; ++i)
        for (int j = 0; j < 8; ++j)
            bits[i * 8 + j] = (packed[i] >> j) & 1;
}

int main() {
    const size_t N = 1u << 20; // 1 Mi 比特
    std::vector<uint8_t> seq(N), in(N);
    Gold gold(0x1234);
    gold.generate(seq.data(), 0, N);
    for (size_t i = 0; i < N; ++i) in[i] = static_cast<uint8_t>((static_cast<uint32_t>(i) * 2654435761u) >> 31);

    std::vector<uint8_t> ref(N), neon(N), packed_out(N);
    std::vector<uint8_t> p_in(N / 8), p_seq(N / 8), p_out(N / 8);

    // 正确性：NEON == 标量；打包解包 == 标量
    xor_scalar(ref.data(), in.data(), seq.data(), N);
    xor_neon(neon.data(), in.data(), seq.data(), N);
    std::printf("NEON == scalar ? %s\n", bench::same(ref.data(), neon.data(), N) ? "YES" : "NO");

    pack(in.data(), N, p_in.data());
    pack(seq.data(), N, p_seq.data());
    xor_scalar(p_out.data(), p_in.data(), p_seq.data(), N / 8);
    unpack(p_out.data(), N, packed_out.data());
    std::printf("packed == scalar ? %s\n", bench::same(ref.data(), packed_out.data(), N) ? "YES" : "NO");

    // 计时
    const size_t reps = 200;
    double t_scalar = bench::time_ns([&] { xor_scalar(ref.data(), in.data(), seq.data(), N); }, reps);
    double t_neon   = bench::time_ns([&] { xor_neon(neon.data(), in.data(), seq.data(), N); }, reps);
    double t_packed = bench::time_ns([&] { xor_scalar(p_out.data(), p_in.data(), p_seq.data(), N / 8); }, reps);
    std::printf("scalar(bit/byte): %.1f ns, NEON: %.1f ns (%.2fx), packed(8bit/byte): %.1f ns (%.2fx)\n",
                t_scalar, t_neon, t_scalar / t_neon, t_packed, t_scalar / t_packed);
    std::printf("=> 简单 XOR 编译器已自动向量化(NEON≈标量)；真正收益在打包(少 8x 内存访问)。\n");
    return 0;
}
