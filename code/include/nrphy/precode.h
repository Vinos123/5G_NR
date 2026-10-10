#pragma once
// PUSCH 预编码（38.211 §6.3.1.5）。
//   codebook-based：z = W·x，W 由 DCI/RRC 从 38.211 Table 6.3.1.5-1..7 选择；
//   non-codebook-based：W 由 SRS 测量得到。
// 本参考实现覆盖单天线端口（P=1, ν=1）的恒等预编码；多端口 codebook 表见文档第 04 章。
#include "nrphy/types.h"

namespace nrphy {

// 层 -> 天线端口。n_ports==1 时恒等（每层直通唯一端口）。
// 返回 per-port 符号（n_ports 个向量）。
std::vector<Symbols> precode(const std::vector<Symbols>& layers, int n_ports);

} // namespace nrphy
