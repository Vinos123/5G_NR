#include "nrphy/rate_match.h"
#include "test_util.h"
#include <cstdlib>

using namespace nrphy;

int main() {
    const int zc = 2;
    const int N = 66 * zc; // BG1, 132 比特

    // 随机码字
    srand(99);
    Bits cw(N);
    for (auto& b : cw) b = static_cast<Bit>(rand() & 1);

    // rv=0, QPSK(不交织), E<=N：输出 = 码字前 E 比特
    {
        Bits out(100);
        rate_match(cw, LdpcBG::BG1, zc, 0, 2, 0, 0, out);
        for (int i = 0; i < 100; ++i) CHECK_EQ(out[i], cw[i]);
    }

    // rv=0, E>N：环形回绕
    {
        Bits out(200);
        rate_match(cw, LdpcBG::BG1, zc, 0, 2, 0, 0, out);
        for (int i = 0; i < 200; ++i) CHECK_EQ(out[i], cw[i % N]);
    }

    // rv=1：k0 = floor(17*N/N)*zc = 34
    {
        Bits out(100);
        rate_match(cw, LdpcBG::BG1, zc, 1, 2, 0, 0, out);
        for (int i = 0; i < 100; ++i) CHECK_EQ(out[i], cw[(34 + i) % N]);
    }

    // 16QAM 交织：out[i*4+j] = e[j*K + i]
    {
        Bits out(16); // K = 4
        rate_match(cw, LdpcBG::BG1, zc, 0, 4, 0, 0, out);
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                CHECK_EQ(out[i * 4 + j], cw[j * 4 + i]);
    }

    // filler 跳过：filler 位于 [nof_sys-nof_filler, nof_sys) = [36,40)，置 1，其余为 0。
    // 输出应全为 0（filler 被跳过）。
    {
        Bits cw2(N, 0);
        for (int i = 36; i < 40; ++i) cw2[i] = 1; // filler 标记
        Bits out(N);
        rate_match(cw2, LdpcBG::BG1, zc, 0, 2, 0, 4, out);
        for (int i = 0; i < N; ++i) CHECK_EQ(out[i], 0);
    }

    TEST_END();
}
