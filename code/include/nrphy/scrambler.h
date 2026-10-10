#pragma once
// PUSCH 加扰，38.211 §6.3.1.1。
//   b~(i) = (b(i) + c(i)) mod 2
//   c_init = n_RNTI * 2^15 + n_ID   （n_ID 来自 dataScramblingIdentityPUSCH，缺省为 N_ID^cell）
#include "nrphy/types.h"

namespace nrphy {

// in/out 为同长度比特序列；输出 = 输入 XOR Gold(c_init)。
void scramble(const Bits& in, Bits& out, uint32_t n_rnti, uint16_t n_id);

} // namespace nrphy
