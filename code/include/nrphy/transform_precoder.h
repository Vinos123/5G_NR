#pragma once
// 变换预编码（DFT-s-OFDM 的 DFT 步骤），38.211 §6.3.1.4。
//   y(l) = (1/sqrt(M)) * sum_{m=0}^{M-1} x(m) * e^{-j 2π m l / M},  l = 0..M-1
// 仅当 transformPrecoding = enabled 时执行；CP-OFDM 跳过此步。
// 注意：M = 12 * N_RB（含因子 3），非 2 的幂，故参考实现用直接 DFT；
//       生产环境用混合基 FFT / 现成 FFT 库（见第 07 章）。
#include "nrphy/types.h"

namespace nrphy {

Symbols transform_precode(const Symbols& x);

} // namespace nrphy
