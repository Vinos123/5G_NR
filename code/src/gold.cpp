#include "nrphy/gold.h"

namespace nrphy {

// 状态约定：s 的 bit i = x(n+i)。推进一步后新状态 bit i = x(n+1+i)：
//   s' = (s >> 1) | (x(n+31) << 30)
static uint32_t step_x1(uint32_t s) {
    const uint32_t fb = ((s >> 3) ^ s) & 1u;
    return (s >> 1) | (fb << 30);
}

static uint32_t step_x2(uint32_t s) {
    const uint32_t fb = ((s >> 3) ^ (s >> 2) ^ (s >> 1) ^ s) & 1u;
    return (s >> 1) | (fb << 30);
}

Gold::Gold(uint32_t c_init)
    : x1_(1u), x2_(c_init & 0x7FFFFFFFu), pos_(0) {
    // 推进 1600 步，使 bit0 = x(1600)，对应 c(0) = x(1600+0)
    for (int i = 0; i < 1600; ++i) advance();
}

void Gold::advance() {
    x1_ = step_x1(x1_);
    x2_ = step_x2(x2_);
}

Bit Gold::next() {
    const Bit b = static_cast<Bit>((x1_ ^ x2_) & 1u);
    advance();
    ++pos_;
    return b;
}

void Gold::generate(Bit* out, size_t n0, size_t count) {
    while (pos_ < n0) { next(); }
    for (size_t i = 0; i < count; ++i) out[i] = next();
}

} // namespace nrphy
