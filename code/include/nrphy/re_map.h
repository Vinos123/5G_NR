#pragma once
// PUSCH 资源映射（38.211 §6.3.1.6 映射到 VRB + §6.3.1.7 VRB->PRB）。
//   上行非交织：n_PRB = n_VRB（恒等）。符号按「子载波 k 递增、再符号 l 递增」摆放，
//   跳过 DMRS/PT-RS 占用的 RE。
#include "nrphy/types.h"

namespace nrphy {

// 把某天线上的一串复符号映射到资源栅格（频率优先）。
//   grid[l][k]：l 为 OFDM 符号索引，k 为子载波索引（长度 = 12*n_rb）。
//   skip：DMRS/PT-RS 占用的 (l,k) 集合（本参考实现由调用方给定）。
//   返回是否摆满（symbols 数量必须等于可用 RE 数）。
bool map_to_grid(const Symbols& symbols, int n_rb, int n_symbols,
                 const std::vector<std::pair<int,int>>& skip,
                 std::vector<std::vector<Complex>>& grid);

} // namespace nrphy
