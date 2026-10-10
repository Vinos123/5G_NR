#include "nrphy/ldpc.h"
#include "nrphy/ldpc_base_graph.h"
#include <algorithm>
#include <cassert>
#include <vector>

namespace nrphy {

int lifting_index(int zc) {
    for (int i = 0; i < kNumLiftingSizes; ++i)
        if (kLiftingSizes[i] == zc) return kLiftingIndex[i];
    return -1;
}

// 取 (bg, i_LS, m, n) 的循环移位，已对 Zc 取模；-1 表示无边。
static int get_shift(LdpcBG bg, int ils, int m, int n, int zc) {
    const int v = (bg == LdpcBG::BG1) ? kBG1Shift[ils][m][n] : kBG2Shift[ils][m][n];
    return (v < 0) ? -1 : (v % zc);
}

// y[l] = x[(l + s) % zc]  （循环移位，s 可为负）
static void circ_shift(const uint8_t* x, int s, int zc, uint8_t* y) {
    s = ((s % zc) + zc) % zc;
    for (int l = 0; l < zc; ++l) y[l] = x[(l + s) % zc];
}

// dst ^= x
static void xor_accum(uint8_t* dst, const uint8_t* x, int len) {
    for (int i = 0; i < len; ++i) dst[i] ^= x[i];
}

void ldpc_encode(const Bits& message, LdpcBG bg, int zc, Bits& codeword) {
    const int kb = (bg == LdpcBG::BG1) ? 22 : 10;
    const int mb = (bg == LdpcBG::BG1) ? 46 : 42;
    const int n_full = kb + mb; // 68 / 52
    const int ils = lifting_index(zc);
    assert(ils >= 0);
    assert(static_cast<int>(message.size()) == kb * zc);

    // 完整码块（含 2 个待 punctured 的信息列），信息位在前、校验位清零。
    std::vector<uint8_t> cb(static_cast<size_t>(n_full) * zc, 0);
    std::copy(message.begin(), message.end(), cb.begin());

    // ---- ① 4 个核心行的信息比特累积和 aux[m] ----
    std::vector<std::vector<uint8_t>> aux(4, std::vector<uint8_t>(zc, 0));
    std::vector<uint8_t> tmp(zc);
    for (int m = 0; m < 4; ++m) {
        for (int k = 0; k < kb; ++k) {
            const int s = get_shift(bg, ils, m, k, zc);
            if (s < 0) continue;
            circ_shift(message.data() + static_cast<size_t>(k) * zc, s, zc, tmp.data());
            xor_accum(aux[m].data(), tmp.data(), zc);
        }
    }

    // ---- ② 4 个核心校验块（列 Kb..Kb+3）----
    uint8_t* p0 = cb.data() + static_cast<size_t>(kb + 0) * zc;
    uint8_t* p1 = cb.data() + static_cast<size_t>(kb + 1) * zc;
    uint8_t* p2 = cb.data() + static_cast<size_t>(kb + 2) * zc;
    uint8_t* p3 = cb.data() + static_cast<size_t>(kb + 3) * zc;
    const uint8_t* a0 = aux[0].data();
    const uint8_t* a1 = aux[1].data();
    const uint8_t* a2 = aux[2].data();
    const uint8_t* a3 = aux[3].data();

    // 核心校验子矩阵的 0/1/105 移位结构随 i_LS 变化（见基图数据），分 4 种情形回代。
    if (bg == LdpcBG::BG1) {
        if (ils == 6) { // 子矩阵 row0.p0=0, row1.p0=105, row3.p0=0
            for (int k = 0; k < zc; ++k) {
                int i = (k - 105) % zc; if (i < 0) i += zc;
                p0[k] = a0[i] ^ a1[i] ^ a2[i] ^ a3[i];
            }
            for (int k = 0; k < zc; ++k) {
                p1[k] = a0[k] ^ p0[k];
                p3[k] = a3[k] ^ p0[k];
                p2[k] = a2[k] ^ p3[k];
            }
        } else {       // row0.p0=1, row1.p0=0, row3.p0=1
            for (int k = 0; k < zc; ++k) p0[k] = a0[k] ^ a1[k] ^ a2[k] ^ a3[k];
            for (int k = 0; k < zc; ++k) {
                p1[k] = a0[k] ^ p0[(k + 1) % zc];
                p3[k] = a3[k] ^ p0[(k + 1) % zc];
                p2[k] = a2[k] ^ p3[k];
            }
        }
    } else { // BG2
        if (ils == 3 || ils == 7) { // row0.p0=1, row2.p0=0, row3.p0=1
            for (int k = 0; k < zc; ++k) p0[k] = a0[k] ^ a1[k] ^ a2[k] ^ a3[k];
            for (int k = 0; k < zc; ++k) {
                p1[k] = a0[k] ^ p0[(k + 1) % zc];
                p2[k] = a1[k] ^ p1[k];
                p3[k] = a3[k] ^ p0[(k + 1) % zc];
            }
        } else {       // row0.p0=0, row2.p0=1, row3.p0=0
            for (int k = 0; k < zc; ++k) {
                int i = (k - 1) % zc; if (i < 0) i += zc;
                p0[k] = a0[i] ^ a1[i] ^ a2[i] ^ a3[i];
                p1[k] = a0[k] ^ p0[k];
                p2[k] = a1[k] ^ p1[k];
                p3[k] = a3[k] ^ p0[k];
            }
        }
    }

    // ---- ③ 扩展校验块（列 Kb+4 .. N_full-1，单对角）----
    for (int m = 4; m < mb; ++m) {
        uint8_t* p = cb.data() + static_cast<size_t>(kb + m) * zc;
        std::fill(p, p + zc, 0);
        // 信息列贡献
        for (int k = 0; k < kb; ++k) {
            const int s = get_shift(bg, ils, m, k, zc);
            if (s < 0) continue;
            circ_shift(message.data() + static_cast<size_t>(k) * zc, s, zc, tmp.data());
            xor_accum(p, tmp.data(), zc);
        }
        // 核心校验列贡献
        for (int k = 0; k < 4; ++k) {
            const int s = get_shift(bg, ils, m, kb + k, zc);
            if (s < 0) continue;
            circ_shift(cb.data() + static_cast<size_t>(kb + k) * zc, s, zc, tmp.data());
            xor_accum(p, tmp.data(), zc);
        }
    }

    // ---- 输出：跳过前 2 个 punctured 列 ----
    codeword.assign(cb.begin() + 2 * zc, cb.end());
}

} // namespace nrphy
