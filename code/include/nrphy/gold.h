#pragma once
// 3GPP 伪随机（Gold）序列发生器，38.211 §5.2.1。
//   c(n) = (x1(n+Nc) + x2(n+Nc)) mod 2,  Nc = 1600
//   x1(n+31) = (x1(n+3) + x1(n)) mod 2
//   x2(n+31) = (x2(n+3) + x2(n+2) + x2(n+1) + x2(n)) mod 2
//   x2 初值由 c_init = sum_{i=0}^{30} x2(i) * 2^i 给定。
#include "nrphy/types.h"

namespace nrphy {

class Gold {
public:
    explicit Gold(uint32_t c_init);

    // 顺序返回 c(n)（从 n=0 开始，每次调用推进一位）
    Bit next();

    // 顺序输出 c(n0 .. n0+count-1)。仅支持向前 seek。
    void generate(Bit* out, size_t n0, size_t count);

private:
    void advance();
    uint32_t x1_, x2_;   // 31 位 LFSR 状态，bit i = x(n+i)
    size_t pos_;         // 已输出的 c() 位数
};

} // namespace nrphy
