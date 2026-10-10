#pragma once
// 码块分段（38.212 §6.2.2 基图选择 + §6.2.3 分段）。
//   把 TB 切成 C 个码块，每个含：数据 + TB CRC(仅末块) + 零填充 + CB CRC(仅 C>1) + filler，
//   长度为 K = kb*Zc（kb = 22/10 为完整值，Zc 的选择用缩减后的 Kb'）。
#include "nrphy/types.h"
#include "nrphy/ldpc.h"
#include <vector>

namespace nrphy {

struct Segmentation {
    int C = 0;              // 码块数
    int zc = 0;             // lifting size
    int kb = 0;             // 22 (BG1) / 10 (BG2)
    int seg_length = 0;     // K = kb*zc
    int nof_filler = 0;     // 每个码块的 filler 比特数
    int tb_crc_len = 0;     // TB CRC 长度 (16/24)
    std::vector<Bits> code_blocks; // C 个码块，各 K 比特
};

// LDPC 基图选择（38.212 §6.2.2，输入 TB 大小 A 与目标码率 R）。
LdpcBG select_base_graph(int A, double R);

// 码块分段（38.212 §6.2.3）。输入 TB（A 比特，不含 CRC），输出含 CRC/filler 的 C 个码块。
Segmentation segment_tb(const Bits& tb, LdpcBG bg);

} // namespace nrphy
