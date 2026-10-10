// 端到端 PUSCH 发送链示例：一个小 TB 走完 14 步，打印各阶段中间结果。
#include "nrphy/pusch.h"
#include "nrphy/segmentation.h"
#include "nrphy/modulator.h"
#include <cstdio>
#include <cstdlib>
#include <complex>

using namespace nrphy;

static void dump_bits(const char* name, const Bits& b, int maxn = 16) {
    std::printf("%-22s (%zu bits): ", name, b.size());
    for (size_t i = 0; i < b.size() && i < (size_t)maxn; ++i) std::printf("%d", b[i]);
    if (b.size() > (size_t)maxn) std::printf("...");
    std::printf("\n");
}

int main() {
    // ---- 配置：48 比特 TB，QPSK，1 层，2 RB，12 个 PUSCH 符号 ----
    const int A = 48;
    Bits tb(A);
    for (int i = 0; i < A; ++i) tb[i] = (i * 7 + 3) % 2; // 确定性伪随机

    const int n_rb = 2, n_symbols = 12, n_layers = 1;
    const ModScheme mod = ModScheme::QPSK;
    const int qm = modulation_order(mod);
    const int G = n_rb * 12 * n_symbols * n_layers * qm; // 总可用比特

    PuschConfig cfg;
    cfg.bg = select_base_graph(A, 0.5);
    cfg.mod = mod;
    cfg.n_rnti = 0x1234;
    cfg.n_id = 42;

    std::printf("=== PUSCH TX: A=%d bits, %s, %s, %d RB, %d syms, G=%d ===\n",
                A, (cfg.bg == LdpcBG::BG1) ? "BG1" : "BG2",
                "QPSK", n_rb, n_symbols, G);

    // 步骤 1-3：分段
    Segmentation seg = segment_tb(tb, cfg.bg);
    std::printf("segmentation: C=%d, Zc=%d, K=%d, filler=%d\n",
                seg.C, seg.zc, seg.seg_length, seg.nof_filler);
    dump_bits("TB", tb);
    dump_bits("codeblock[0]", seg.code_blocks[0], 32);

    // 步骤 1-7：比特级
    Bits f = pusch_encode_bits(tb, cfg, G);
    dump_bits("codeword f", f, 32);

    // 步骤 8-12：符号级
    Symbols z = pusch_encode_symbols(f, cfg);
    std::printf("symbols z (%zu): ", z.size());
    for (size_t i = 0; i < z.size() && i < 8; ++i)
        std::printf("(%.2f%+.2fj) ", z[i].real(), z[i].imag());
    std::printf("...\n");

    // 步骤 13-14：RE 映射（无 DMRS 简化，示范性）
    std::vector<std::pair<int,int>> skip; // 空：不跳过（演示）
    auto grid = pusch_tx(tb, cfg, G, n_rb, n_symbols, skip);
    std::printf("resource grid: %zu symbols x %zu subcarriers\n", grid.size(), grid[0].size());
    std::printf("grid[0][0] = (%.2f%+.2fj)\n", grid[0][0].real(), grid[0][0].imag());

    std::printf("=== done ===\n");
    return 0;
}
