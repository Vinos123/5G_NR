#include "nrphy/ldpc.h"
#include "nrphy/ldpc_base_graph.h"
#include "test_util.h"
#include <cstdlib>
#include <vector>

using namespace nrphy;

static int get_shift_raw(LdpcBG bg, int ils, int m, int n, int zc) {
    const int v = (bg == LdpcBG::BG1) ? kBG1Shift[ils][m][n] : kBG2Shift[ils][m][n];
    return (v < 0) ? -1 : (v % zc);
}

// 编码后独立构建提升校验矩阵 H 并验证 H·c == 0。
static bool codeword_ok(LdpcBG bg, int zc) {
    const int kb = (bg == LdpcBG::BG1) ? 22 : 10;
    const int mb = (bg == LdpcBG::BG1) ? 46 : 42;
    const int n_full = kb + mb;
    const int ils = lifting_index(zc);
    if (ils < 0) return false;

    Bits msg(static_cast<size_t>(kb) * zc);
    for (auto& b : msg) b = static_cast<Bit>(rand() & 1);

    Bits codeword;
    ldpc_encode(msg, bg, zc, codeword);
    if (static_cast<int>(codeword.size()) != (n_full - 2) * zc) return false;

    // 还原完整码块：前 2 个 punctured 列 = msg 前 2*zc 比特
    std::vector<uint8_t> full;
    full.insert(full.end(), msg.begin(), msg.begin() + 2 * zc);
    full.insert(full.end(), codeword.begin(), codeword.end());
    if (static_cast<int>(full.size()) != n_full * zc) return false;

    // 校验每个 (校验节点 m, 位 l)：所有连接变量位 XOR == 0
    for (int m = 0; m < mb; ++m) {
        for (int l = 0; l < zc; ++l) {
            uint8_t sum = 0;
            for (int n = 0; n < n_full; ++n) {
                const int s = get_shift_raw(bg, ils, m, n, zc);
                if (s < 0) continue;
                const int j = (l + s) % zc;
                sum ^= full[static_cast<size_t>(n) * zc + j];
            }
            if (sum != 0) return false;
        }
    }
    return true;
}

int main() {
    srand(12345);
    // 覆盖两个基图、多个 lifting size（含核心校验的各种 i_LS 特殊情形）
    for (LdpcBG bg : {LdpcBG::BG1, LdpcBG::BG2}) {
        for (int zc : {2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 16, 26, 52, 64, 104, 208, 384}) {
            for (int trial = 0; trial < 3; ++trial) {
                CHECK(codeword_ok(bg, zc));
            }
        }
    }
    TEST_END();
}
