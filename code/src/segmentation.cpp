#include "nrphy/segmentation.h"
#include "nrphy/crc.h"
#include "nrphy/ldpc_base_graph.h"
#include <algorithm>
#include <cassert>

namespace nrphy {

LdpcBG select_base_graph(int A, double R) {
    // 38.212 §6.2.2：A<=292 或 R<=0.25 或 (A<=3824 且 R<=0.67) -> BG2，否则 BG1。
    if (A <= 292 || R <= 0.25 || (A <= 3824 && R <= 0.67)) return LdpcBG::BG2;
    return LdpcBG::BG1;
}

Segmentation segment_tb(const Bits& tb, LdpcBG bg) {
    const int A = static_cast<int>(tb.size());
    assert(A > 0);

    Segmentation seg;
    seg.tb_crc_len = (A <= 3824) ? 16 : 24;
    const int B = A + seg.tb_crc_len;

    const int kb_full = (bg == LdpcBG::BG1) ? 22 : 10;
    const int Kcb = (bg == LdpcBG::BG1) ? 8448 : 3840;

    // 码块数 C
    const int C = (B <= Kcb) ? 1 : (B + (Kcb - 24) - 1) / (Kcb - 24);

    // 缩减后的 Kb'（仅用于选 Zc）
    int kb_reduced;
    if (bg == LdpcBG::BG1) kb_reduced = 22;
    else if (B > 640)      kb_reduced = 10;
    else if (B > 560)      kb_reduced = 9;
    else if (B > 192)      kb_reduced = 8;
    else                   kb_reduced = 6;

    // lifting size Zc：最小 Zc 使 C*Kb'*Zc >= B + (C>1 ? 24*C : 0)
    const int B_out = B + (C > 1 ? 24 * C : 0);
    int zc = 0;
    for (int i = 0; i < kNumLiftingSizes; ++i) {
        if (kLiftingSizes[i] * C * kb_reduced >= B_out) { zc = kLiftingSizes[i]; break; }
    }
    assert(zc > 0);

    seg.C = C;
    seg.zc = zc;
    seg.kb = kb_full;
    seg.seg_length = kb_full * zc;

    const int cb_crc = (C > 1) ? 24 : 0;
    const int cb_info = (B_out + C - 1) / C - cb_crc;     // ceil(B_out/C) - cb_crc
    const int zero_pad = (cb_info + cb_crc) * C - B_out;
    seg.nof_filler = seg.seg_length - cb_info - cb_crc;
    assert(seg.nof_filler >= 0);

    // TB CRC
    Bits tb_crc_bits;
    {
        const CrcType t = (A <= 3824) ? CrcType::CRC16 : CrcType::CRC24A;
        const uint32_t r = crc(tb, t);
        for (int i = seg.tb_crc_len - 1; i >= 0; --i)
            tb_crc_bits.push_back(static_cast<Bit>((r >> i) & 1u));
    }

    // 逐段组装
    seg.code_blocks.assign(C, Bits(seg.seg_length, 0));
    size_t tb_pos = 0;
    for (int s = 0; s < C; ++s) {
        const bool last = (s == C - 1);
        Bits& cb = seg.code_blocks[s];
        int pos = 0;

        // 1) 数据
        const int data = std::min<int>(A - static_cast<int>(tb_pos), cb_info);
        std::copy(tb.begin() + tb_pos, tb.begin() + tb_pos + data, cb.begin());
        tb_pos += data;
        pos += data;

        // 2) 末段追加 TB CRC + 零填充
        if (last) {
            std::copy(tb_crc_bits.begin(), tb_crc_bits.end(), cb.begin() + pos);
            pos += seg.tb_crc_len;
            pos += zero_pad; // 零填充（cb 已初始化为 0）
        }

        // 3) CB CRC（仅 C>1），覆盖到目前为止的非 filler 比特
        if (C > 1) {
            const uint32_t r = crc(cb.data(), pos, CrcType::CRC24B);
            for (int i = 23; i >= 0; --i)
                cb[pos++] = static_cast<Bit>((r >> i) & 1u);
        }

        // 4) filler（cb 已初始化为 0，无需写）
        pos = seg.seg_length;
    }
    assert(tb_pos == static_cast<size_t>(A));

    return seg;
}

} // namespace nrphy
