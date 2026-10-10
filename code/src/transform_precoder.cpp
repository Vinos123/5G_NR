#include "nrphy/transform_precoder.h"
#include <cmath>
#include <cstdint>

namespace nrphy {

Symbols transform_precode(const Symbols& x) {
    const size_t M = x.size();
    Symbols y(M);
    const double inv = 1.0 / std::sqrt(double(M));

    for (size_t l = 0; l < M; ++l) {
        Complex acc(0.0f, 0.0f);
        for (size_t m = 0; m < M; ++m) {
            // 先把 m*l 对 M 取模，提升大 M 下的数值稳定性
            const double k = std::fmod(double(uint64_t(m) * uint64_t(l)), double(M));
            const double ang = -2.0 * kPi * k / double(M);
            acc += x[m] * Complex(float(std::cos(ang)), float(std::sin(ang)));
        }
        y[l] = acc * float(inv);
    }
    return y;
}

} // namespace nrphy
