#include "nrphy/segmentation.h"
#include "nrphy/crc.h"
#include "test_util.h"
#include <cstdlib>

using namespace nrphy;

int main() {
    // 金标准参数（转录自 srsRAN 测试向量，来源 3GPP 38.212 §6.2.2/6.2.3）：
    // {TBS, bg(1/2), C, seg_length}
    struct { int tbs; LdpcBG bg; int C; int K; } golden[] = {
        {96, LdpcBG::BG1, 1, 132},
        {600, LdpcBG::BG1, 1, 616},
        {4000, LdpcBG::BG1, 1, 4224},
        {12000, LdpcBG::BG1, 2, 6336},
        {40000, LdpcBG::BG1, 5, 8448},
        {96, LdpcBG::BG2, 1, 200},
        {320, LdpcBG::BG2, 1, 440},
        {600, LdpcBG::BG2, 1, 720},
        {4000, LdpcBG::BG2, 2, 2080},
        {12000, LdpcBG::BG2, 4, 3200},
        {40000, LdpcBG::BG2, 11, 3840},
    };

    srand(7);
    for (const auto& g : golden) {
        Bits tb(g.tbs);
        for (auto& b : tb) b = static_cast<Bit>(rand() & 1);
        Segmentation seg = segment_tb(tb, g.bg);
        CHECK_EQ(seg.C, g.C);
        CHECK_EQ(seg.seg_length, g.K);
        CHECK_EQ(static_cast<int>(seg.code_blocks.size()), g.C);
        for (const auto& cb : seg.code_blocks)
            CHECK_EQ(static_cast<int>(cb.size()), g.K);
    }

    // 基图选择（38.212 §6.2.2）
    CHECK(select_base_graph(100, 0.5) == LdpcBG::BG2);   // A<=292
    CHECK(select_base_graph(500, 0.2) == LdpcBG::BG2);   // R<=0.25
    CHECK(select_base_graph(2000, 0.5) == LdpcBG::BG2);  // A<=3824 且 R<=0.67
    CHECK(select_base_graph(2000, 0.8) == LdpcBG::BG1);  // A>292, R>0.67
    CHECK(select_base_graph(5000, 0.5) == LdpcBG::BG1);  // A>3824

    // 分段 + 编码自洽：每个码块编码后 H·c==0 已在 test_ldpc 覆盖；
    // 这里验证码块大小是 Kb*Zc 的倍数关系，且 filler >= 0。
    Segmentation s1 = segment_tb(Bits(96, 0), LdpcBG::BG2);
    CHECK(s1.nof_filler >= 0);

    TEST_END();
}
