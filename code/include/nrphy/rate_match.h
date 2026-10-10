#pragma once
// LDPC 速率匹配（38.212 §6.2.5 / §5.4.2.1）+ 位交织（§5.4.2.2）。
//   codeword：编码输出（66*Zc / 50*Zc 比特，已去掉 2 个 punctured 列）。
//   从环形缓冲（长度 N_cb = min(N_ref, N)）自 k0 起选 E 比特，跳过 filler，再做 Qm 位交织。
#include "nrphy/types.h"
#include "nrphy/ldpc.h"

namespace nrphy {

// codeword -> out (E 比特，由 out.size() 指定长度)。
//   qm：调制阶数（1/2 不交织；4/6/8 做行-列转置交织）。
//   nref：N_ref（受限缓冲，0 表示不限）。
//   nof_filler：本码块的 filler 比特数（位于 systematic 部分末尾）。
void rate_match(const Bits& codeword, LdpcBG bg, int zc, int rv, int qm,
                int nref, int nof_filler, Bits& out);

} // namespace nrphy
