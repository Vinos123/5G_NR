#pragma once
// CRC 计算（38.212 §5.1）。
//   CRC24A = 0x1864CFB  (g = D^24+D^23+D^18+D^17+D^14+D^11+D^10+D^7+D^6+D^5+D^4+D^3+D+1)
//   CRC24B = 0x1800063  (g = D^24+D^23+D^6+D^5+D+1)         — 码块 CRC
//   CRC24C = 0x1B2B117  (g = D^24+D^23+D^21+D^20+D^17+D^15+D^13+D^12+D^8+D^4+D^2+D+1) — DCI
//   CRC16  = 0x11021    (g = D^16+D^12+D^5+1)
#include "nrphy/types.h"

namespace nrphy {

enum class CrcType { CRC24A, CRC24B, CRC24C, CRC16 };

// CRC 长度（24 或 16）
unsigned crc_len(CrcType type);

// 计算输入比特序列的 CRC 余数（MSB-first 无反射，init=0，无 xor-out）。
// 返回值：p0 在最高位、p(L-1) 在最低位。
uint32_t crc(const Bit* bits, size_t nbits, CrcType type);
uint32_t crc(const Bits& bits, CrcType type);

// 在序列末尾附加 L 个校验比特 p(0..L-1)（p(0) 先附）。
// 满足：crc(msg || parity, type) == 0。
void crc_attach(Bits& bits, CrcType type);

} // namespace nrphy
