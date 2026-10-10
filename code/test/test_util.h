#pragma once
// 极简测试框架：CHECK 系列宏统计失败数，TEST_END() 返回进程退出码。
#include <cstdio>
#include <cmath>

namespace testutil { inline int& failures() { static int f = 0; return f; } }

#define CHECK(cond) \
    do { if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++testutil::failures(); } } while (0)

#define CHECK_EQ(a, b) \
    do { auto _a=(a); auto _b=(b); if (!(_a==_b)) { std::printf("FAIL %s:%d  %s == %s\n", __FILE__, __LINE__, #a, #b); ++testutil::failures(); } } while (0)

#define CHECK_NEAR(a, b, eps) \
    do { auto _a=(a); auto _b=(b); if (std::fabs((_a)-(_b)) > (eps)) { std::printf("FAIL %s:%d  %s ~= %s\n", __FILE__, __LINE__, #a, #b); ++testutil::failures(); } } while (0)

#define TEST_END() return testutil::failures() == 0 ? 0 : 1
