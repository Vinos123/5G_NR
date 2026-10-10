#include "nrphy/rate_match.h"
#include <algorithm>
#include <cassert>
#include <cmath>

namespace nrphy {

void rate_match(const Bits& codeword, LdpcBG bg, int zc, int rv, int qm,
                int nref, int nof_filler, Bits& out) {
    const int kb = (bg == LdpcBG::BG1) ? 22 : 10;
    const int n_short = (bg == LdpcBG::BG1) ? 66 : 50;
    const int N = n_short * zc;
    assert(static_cast<int>(codeword.size()) == N);
    assert(rv >= 0 && rv <= 3);

    const int E = static_cast<int>(out.size());
    const int n_cb = (nref > 0) ? std::min(nref, N) : N;

    // 起始位置 k0（38.212 §5.4.2.1）
    static const double sf_bg1[4] = {0, 17, 33, 56};
    static const double sf_bg2[4] = {0, 13, 25, 43};
    const double sf = (bg == LdpcBG::BG1) ? sf_bg1[rv] : sf_bg2[rv];
    const int k0 = static_cast<int>(std::floor(sf * n_cb / N)) * zc;

    // filler 比特范围（systematic 部分末尾），systematic 长度 = (kb-2)*zc
    const int nof_sys = (kb - 2) * zc;
    const int filler_start = nof_sys - nof_filler;

    // ① 从环形缓冲选比特，跳过 filler
    Bits e;
    e.reserve(E);
    int idx = k0 % n_cb;
    while (static_cast<int>(e.size()) < E) {
        if (nof_filler > 0 && idx >= filler_start && idx < nof_sys) {
            idx = nof_sys; // 跳过 filler 区
            continue;
        }
        e.push_back(codeword[idx]);
        idx = (idx + 1) % n_cb;
    }

    // ② 位交织（Qm > 2）：e 视为 Qm 行 × K 列，按列读出
    if (qm <= 2) {
        out = std::move(e);
        return;
    }
    const int K = E / qm;
    for (int i = 0; i < K; ++i)
        for (int j = 0; j < qm; ++j)
            out[static_cast<size_t>(i) * qm + j] = e[static_cast<size_t>(j) * K + i];
}

} // namespace nrphy
