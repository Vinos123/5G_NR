#pragma once
// 5G NR LDPC 编码（38.212 §5.3.2）。
//   基图：BG1 (46x68, Kb=22) / BG2 (42x52, Kb=10)。
//   输入 message 为 K = Kb*Zc 比特（含 CRC / filler，占满全部 Kb 个信息列）；
//   输出 codeword 为 (Kb+Mb-2)*Zc = 66*Zc (BG1) / 50*Zc (BG2) 比特，
//   即去掉前 2 个 punctured 列（信息列 0,1）后的码字。
//   编码三步：① 4 个核心行的信息比特累积和；② 4 个核心校验块（双对角回代）；
//            ③ 扩展校验块（单对角）。
#include "nrphy/types.h"

namespace nrphy {

enum class LdpcBG { BG1, BG2 };

// 返回给定 Zc 的 lifting index (i_LS, 0..7)，非法返回 -1。
int lifting_index(int zc);

// LDPC 编码。
void ldpc_encode(const Bits& message, LdpcBG bg, int zc, Bits& codeword);

} // namespace nrphy
