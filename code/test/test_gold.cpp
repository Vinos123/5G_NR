#include "nrphy/gold.h"
#include "test_util.h"
#include <vector>

using namespace nrphy;

int main() {
    // c_init=0 -> x2 全零 -> c(n) = x1(n+1600)。
    // 在测试里独立地用递推式重算 x1，再与 Gold 输出比对。
    std::vector<Bit> x1(1632, 0);
    x1[0] = 1;
    for (int n = 0; n <= 1600; ++n)
        x1[n + 31] = x1[n + 3] ^ x1[n];

    Gold g0(0);
    for (int n = 0; n < 32; ++n)
        CHECK_EQ(g0.next(), x1[1600 + n]);

    // 非零 c_init 产生不同序列
    bool differs = false;
    Gold ga(0), gb(1);
    for (int n = 0; n < 64; ++n)
        if (ga.next() != gb.next()) { differs = true; break; }
    CHECK(differs);

    // generate() 向前 seek 与从头顺序生成一致
    const uint32_t ci = 0x1234;
    Bit a[16], b[16], c[116];
    Gold(ci).generate(a, 100, 16);   // 临时对象，直接 seek 到 100
    Gold(ci).generate(b, 100, 16);
    for (int i = 0; i < 16; ++i) CHECK_EQ(a[i], b[i]);

    Gold g4(ci);
    g4.generate(c, 0, 116);
    for (int i = 0; i < 16; ++i) CHECK_EQ(a[i], c[100 + i]);

    TEST_END();
}
