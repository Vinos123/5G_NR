#include "nrphy/modulator.h"
#include <cassert>
#include <cmath>

namespace nrphy {

int modulation_order(ModScheme scheme) {
    switch (scheme) {
        case ModScheme::PI2_BPSK: return 1;
        case ModScheme::QPSK:     return 2;
        case ModScheme::QAM16:    return 4;
        case ModScheme::QAM64:    return 6;
        case ModScheme::QAM256:   return 8;
    }
    return 0;
}

// 把 Q 个比特 [b0..b(Q-1)] 映射为奇数整数值（NR 方形 QAM 的嵌套式 Gray 映射）。
//   level = (1-2b0) * ( 2^{Q-1} - (1-2b1) * ( 2^{Q-2} - ... ( 2 - (1-2b_{Q-1}) ) ... ) )
static int qam_level(const Bit* b, int Q) {
    if (Q == 1) return 1 - 2 * static_cast<int>(b[0]);   // QPSK：I = 1-2b0
    int inner = 2 - (1 - 2 * static_cast<int>(b[Q - 1]));
    for (int k = Q - 2; k >= 1; --k)
        inner = (1 << (Q - k)) - (1 - 2 * static_cast<int>(b[k])) * inner;
    return (1 - 2 * static_cast<int>(b[0])) * inner;
}

Symbols modulate(const Bits& bits, ModScheme scheme) {
    const int Qm = modulation_order(scheme);
    assert(Qm > 0);
    assert(bits.size() % static_cast<size_t>(Qm) == 0);
    const size_t nsym = bits.size() / Qm;

    if (scheme == ModScheme::PI2_BPSK) {
        Symbols out(nsym);
        for (size_t i = 0; i < nsym; ++i) {
            const float re = (1.0f - 2.0f * static_cast<float>(bits[i])) / kSqrt2;
            // π/2-BPSK：偶数符号在实轴，奇数符号在虚轴
            out[i] = (i % 2 == 0) ? Complex(re, 0.0f) : Complex(0.0f, re);
        }
        return out;
    }

    const int Q = Qm / 2;
    // 归一化：1 / sqrt( (2/3) * (2^{2Q} - 1) )
    const double norm = 1.0 / std::sqrt((2.0 / 3.0) * double((1u << (2 * Q)) - 1));

    Symbols out(nsym);
    Bit b[8];
    for (size_t i = 0; i < nsym; ++i) {
        for (int k = 0; k < Q; ++k) b[k] = bits[i * Qm + 2 * k];     // I 路：偶数位
        const int I = qam_level(b, Q);
        for (int k = 0; k < Q; ++k) b[k] = bits[i * Qm + 2 * k + 1]; // Q 路：奇数位
        const int Qv = qam_level(b, Q);
        out[i] = Complex(static_cast<float>(I * norm), static_cast<float>(Qv * norm));
    }
    return out;
}

} // namespace nrphy
