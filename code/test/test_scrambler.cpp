#include "nrphy/scrambler.h"
#include "nrphy/gold.h"
#include "test_util.h"

using namespace nrphy;

int main() {
    // c_init=0 时，全零输入的加扰输出 = 完整 Gold 序列
    Bits zeros(64, 0), out;
    scramble(zeros, out, 0, 0);
    Gold g(0);
    for (size_t i = 0; i < out.size(); ++i) CHECK_EQ(out[i], g.next());

    // 自逆性：加扰两次恢复原序列
    const Bits msg = {1,0,1,1,0,0,1,1,0,1,0,0,1,1,1,0,0,1};
    Bits once, twice;
    scramble(msg, once, 0x1234, 55);
    scramble(once, twice, 0x1234, 55);
    CHECK(twice == msg);

    TEST_END();
}
