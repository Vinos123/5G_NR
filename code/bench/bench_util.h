#pragma once
// 基准测试小工具：计时 + 校验。
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace bench {

inline double now_seconds() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

// 运行 f 共 reps 次，返回单次平均耗时（纳秒）。
template <typename F>
double time_ns(F&& f, size_t reps) {
    auto t0 = std::chrono::steady_clock::now();
    for (size_t i = 0; i < reps; ++i) f();
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(reps);
}

// 校验两段字节相等。
inline bool same(const uint8_t* a, const uint8_t* b, size_t n) {
    for (size_t i = 0; i < n; ++i)
        if (a[i] != b[i]) return false;
    return true;
}

} // namespace bench
