#include "nrphy/crc.h"

namespace nrphy {

unsigned crc_len(CrcType type) {
    switch (type) {
        case CrcType::CRC24A:
        case CrcType::CRC24B:
        case CrcType::CRC24C: return 24;
        case CrcType::CRC16:  return 16;
    }
    return 0;
}

// 多项式去掉隐含最高位 (x^L) 后的 L 位值
static uint32_t crc_poly(CrcType type) {
    switch (type) {
        case CrcType::CRC24A: return 0x864CFBu;
        case CrcType::CRC24B: return 0x800063u;
        case CrcType::CRC24C: return 0xB2B117u;
        case CrcType::CRC16:  return 0x1021u;
    }
    return 0;
}

uint32_t crc(const Bit* bits, size_t nbits, CrcType type) {
    const unsigned L = crc_len(type);
    const uint32_t poly = crc_poly(type);
    const uint32_t mask = (L == 24) ? 0xFFFFFFu : 0xFFFFu;

    uint32_t reg = 0;
    for (size_t i = 0; i < nbits; ++i) {
        const uint32_t msb = (reg >> (L - 1)) & 1u;
        const uint32_t bit = bits[i] & 1u;
        reg = (reg << 1) & mask;
        if ((msb ^ bit) != 0) reg ^= poly;
    }
    return reg;
}

uint32_t crc(const Bits& bits, CrcType type) {
    return crc(bits.data(), bits.size(), type);
}

void crc_attach(Bits& bits, CrcType type) {
    const unsigned L = crc_len(type);
    const uint32_t r = crc(bits, type);
    for (int i = L - 1; i >= 0; --i)
        bits.push_back(static_cast<Bit>((r >> i) & 1u));
}

} // namespace nrphy
