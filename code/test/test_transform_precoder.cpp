#include "nrphy/transform_precoder.h"
#include "test_util.h"
#include <cmath>

using namespace nrphy;

static bool c_near(Complex a, Complex b, float eps) {
    return std::fabs(a.real() - b.real()) < eps && std::fabs(a.imag() - b.imag()) < eps;
}

int main() {
    const float r2 = 1.0f / kSqrt2;

    // M=1：恒等
    CHECK(c_near(transform_precode({Complex(3,0)})[0], Complex(3,0), 1e-6f));

    // M=2：[1,0] -> [1/sqrt2, 1/sqrt2]
    auto y2 = transform_precode({Complex(1,0), Complex(0,0)});
    CHECK(c_near(y2[0], Complex( r2, 0), 1e-6f));
    CHECK(c_near(y2[1], Complex( r2, 0), 1e-6f));

    // M=2：[0,1] -> [1/sqrt2, -1/sqrt2]
    auto y3 = transform_precode({Complex(0,0), Complex(1,0)});
    CHECK(c_near(y3[0], Complex( r2, 0), 1e-6f));
    CHECK(c_near(y3[1], Complex(-r2, 0), 1e-6f));

    // M=4：[1,0,0,0] -> 全 1/2
    auto y4 = transform_precode({Complex(1,0), Complex(0,0), Complex(0,0), Complex(0,0)});
    for (int i = 0; i < 4; ++i) CHECK(c_near(y4[i], Complex(0.5f, 0), 1e-6f));

    TEST_END();
}
