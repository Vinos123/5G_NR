#pragma once
// 调制，38.211 §6.3.1.2 / Table 5.1.3.1-1（普通）、5.1.3.1-2（变换预编码下 π/2-BPSK）。
//   支持 QPSK / 16QAM / 64QAM / 256QAM（Qm=2/4/6/8）与 π/2-BPSK（Qm=1）。
#include "nrphy/types.h"

namespace nrphy {

enum class ModScheme { PI2_BPSK, QPSK, QAM16, QAM64, QAM256 };

// 调制阶数 Qm = 1 / 2 / 4 / 6 / 8
int modulation_order(ModScheme scheme);

// 比特序列 -> 复数符号序列。长度必须是 Qm 的整数倍。
// 方形 QAM 的 I 路取偶数位比特、Q 路取奇数位比特（LTE/NR 统一的 Gray 映射）。
Symbols modulate(const Bits& bits, ModScheme scheme);

} // namespace nrphy
