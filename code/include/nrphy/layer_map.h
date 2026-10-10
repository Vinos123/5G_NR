#pragma once
// PUSCH 层映射，38.211 §6.3.1.3（单码字 -> ν 层，ν=1..4）。
//   x^v(i) = d(v*i + v),  v = 0..ν-1
#include "nrphy/types.h"

namespace nrphy {

// 返回 ν 个层向量，每个长度 = |d| / ν（要求 |d| 能被 ν 整除）。
std::vector<Symbols> layer_map(const Symbols& d, int v);

} // namespace nrphy
