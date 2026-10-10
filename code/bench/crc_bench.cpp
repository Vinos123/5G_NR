// CRC：逐位（bit-per-byte，参考实现所用）vs 字节表（8 bit/byte 打包）。
//   逐位版对 8N 个比特逐位处理；字节表版对 N 个字节处理，约 8x。
//   更高阶：x86 PCLMULQDQ / arm64 vmull_p64 无进位乘法并行 CRC，见文档 07 章。
#include "bench_util.h"
#include "nrphy/crc.h"
#include <cstdint>
#include <cstdio>
#include <vector>

using namespace nrphy;

// 字节表驱动 CRC24A（MSB-first 无反射）。table[byte] = 该字节进移位寄存器后的余数。
static uint32_t crc24a_table(const uint8_t* msg, size_t nbytes) {
    static uint32_t table[256];
    static bool built = false;
    if (!built) {
        const uint32_t poly = 0x864CFBu;
        for (int i = 0; i < 256; ++i) {
            uint32_t c = static_cast<uint32_t>(i) << 16;
            for (int j = 0; j < 8; ++j) {
                c = (c & 0x800000u) ? ((c << 1) ^ poly) : (c << 1);
                c &= 0xFFFFFFu; // 保持 24 位，避免 bit24 溢出
            }
            table[i] = c;
        }
        built = true;
    }
    uint32_t crc = 0;
    for (size_t i = 0; i < nbytes; ++i)
        crc = ((crc << 8) & 0xFFFFFFu) ^ table[((crc >> 16) ^ msg[i]) & 0xFFu];
    return crc & 0xFFFFFFu;
}

// 字节 -> 比特（MSB-first：bit 7 先）
static void bytes_to_bits(const std::vector<uint8_t>& bytes, Bits& bits) {
    bits.clear();
    bits.reserve(bytes.size() * 8);
    for (uint8_t b : bytes)
        for (int i = 7; i >= 0; --i)
            bits.push_back((b >> i) & 1);
}

int main() {
    const size_t NB = 1u << 16; // 64 KiB
    std::vector<uint8_t> msg(NB);
    for (size_t i = 0; i < NB; ++i) msg[i] = static_cast<uint8_t>(i * 31 + 7);

    Bits bits;
    bytes_to_bits(msg, bits);

    // 正确性：逐位（比特向量）与字节表一致
    uint32_t ref = crc(bits.data(), bits.size(), CrcType::CRC24A);
    uint32_t fast = crc24a_table(msg.data(), NB);
    std::printf("CRC24A: table == bitwise ? %s (0x%06x vs 0x%06x)\n",
                (ref == fast) ? "YES" : "NO", ref, fast);

    // 计时：逐位处理 8N 比特 vs 表处理 N 字节
    const size_t reps = 1000;
    double t_bit = bench::time_ns([&] { volatile auto _ = crc(bits.data(), bits.size(), CrcType::CRC24A); }, reps);
    double t_tab = bench::time_ns([&] { volatile auto _ = crc24a_table(msg.data(), NB); }, reps);
    std::printf("  bitwise: %.1f ns, table: %.1f ns, speedup %.2fx\n", t_bit, t_tab, t_bit / t_tab);
    return 0;
}
