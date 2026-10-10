#include "nrphy/pusch.h"
#include "nrphy/crc.h"
#include "nrphy/layer_map.h"
#include "nrphy/precode.h"
#include "nrphy/rate_match.h"
#include "nrphy/re_map.h"
#include "nrphy/scrambler.h"
#include "nrphy/transform_precoder.h"
#include <cassert>

namespace nrphy {

Bits pusch_encode_bits(const Bits& tb, const PuschConfig& cfg, int G) {
    // 步骤 1-3：分段（内含 TB CRC 附加、基图选择、码块分段 + CB CRC + filler）
    Segmentation seg = segment_tb(tb, cfg.bg);
    const int qm = modulation_order(cfg.mod);
    const int C = seg.C;

    // 每个码块的速率匹配长度 E_r（38.212 §5.4.2.1，按符号数均分）
    const int nof_sym_per_layer = G / (cfg.n_layers * qm);
    const int nof_short = C - (nof_sym_per_layer % C);

    Bits f;
    f.reserve(G);
    for (int r = 0; r < C; ++r) {
        // 步骤 4：LDPC 编码
        Bits codeword;
        ldpc_encode(seg.code_blocks[r], cfg.bg, seg.zc, codeword);

        // 步骤 5：速率匹配
        const int tmp = (r < nof_short) ? (nof_sym_per_layer / C)
                                        : (nof_sym_per_layer + C - 1) / C;
        const int E_r = tmp * cfg.n_layers * qm;
        Bits e(E_r);
        rate_match(codeword, cfg.bg, seg.zc, cfg.rv, qm, cfg.nref, seg.nof_filler, e);

        // 步骤 6：码块级联
        f.insert(f.end(), e.begin(), e.end());
    }
    assert(static_cast<int>(f.size()) == G);
    return f;
}

Symbols pusch_encode_symbols(const Bits& f, const PuschConfig& cfg) {
    // 步骤 8：加扰
    Bits scr;
    scramble(f, scr, cfg.n_rnti, cfg.n_id);

    // 步骤 9：调制
    Symbols d = modulate(scr, cfg.mod);

    // 步骤 10：层映射
    std::vector<Symbols> layers = layer_map(d, cfg.n_layers);

    // 步骤 11：变换预编码（DFT-s-OFDM）
    std::vector<Symbols> y = layers;
    if (cfg.transform_precoding) {
        for (auto& l : y) l = transform_precode(l);
    }

    // 步骤 12：预编码（单端口恒等）
    std::vector<Symbols> z = precode(y, cfg.n_layers == 1 ? 1 : cfg.n_layers);
    return z[0];
}

std::vector<std::vector<Complex>>
pusch_tx(const Bits& tb, const PuschConfig& cfg, int G, int n_rb, int n_symbols,
         const std::vector<std::pair<int,int>>& skip) {
    Bits f = pusch_encode_bits(tb, cfg, G);
    Symbols z = pusch_encode_symbols(f, cfg);

    // 步骤 13-14：VRB 映射 + VRB->PRB（上行恒等）
    std::vector<std::vector<Complex>> grid;
    const bool ok = map_to_grid(z, n_rb, n_symbols, skip, grid);
    assert(ok);
    return grid;
}

} // namespace nrphy
