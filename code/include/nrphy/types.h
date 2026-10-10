#pragma once
// 公共类型与常量。比特序列约定（3GPP 惯例）：
//   用 std::vector<uint8_t> 存比特，每个元素 0 或 1；
//   顺序为 a0, a1, ..., a(A-1)，即「首比特优先 / MSB-first」（a0 对应首个字节的 MSB）。
#include <cstdint>
#include <cstddef>
#include <vector>
#include <complex>

namespace nrphy {

using Bit = uint8_t;                 // 单个比特，取值 0 或 1
using Bits = std::vector<Bit>;       // 比特序列
using Complex = std::complex<float>; // 调制符号 / 复数样本
using Symbols = std::vector<Complex>;

constexpr double kPi = 3.14159265358979323846;
constexpr float  kSqrt2 = 1.4142135623730950488f;

} // namespace nrphy
