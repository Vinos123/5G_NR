#include "nrphy/modulator.h"
#include "test_util.h"
#include <cmath>

using namespace nrphy;

static bool c_near(Complex a, Complex b, float eps) {
    return std::fabs(a.real() - b.real()) < eps && std::fabs(a.imag() - b.imag()) < eps;
}

int main() {
    const float r2   = 1.0f / kSqrt2;
    const float r10  = 1.0f / std::sqrt(10.0f);
    const float r42  = 1.0f / std::sqrt(42.0f);
    const float r170 = 1.0f / std::sqrt(170.0f);

    CHECK_EQ(modulation_order(ModScheme::PI2_BPSK), 1);
    CHECK_EQ(modulation_order(ModScheme::QPSK), 2);
    CHECK_EQ(modulation_order(ModScheme::QAM16), 4);
    CHECK_EQ(modulation_order(ModScheme::QAM64), 6);
    CHECK_EQ(modulation_order(ModScheme::QAM256), 8);

    // QPSK
    CHECK(c_near(modulate({0,0}, ModScheme::QPSK)[0], Complex( r2,  r2), 1e-6f));
    CHECK(c_near(modulate({0,1}, ModScheme::QPSK)[0], Complex( r2, -r2), 1e-6f));
    CHECK(c_near(modulate({1,0}, ModScheme::QPSK)[0], Complex(-r2,  r2), 1e-6f));
    CHECK(c_near(modulate({1,1}, ModScheme::QPSK)[0], Complex(-r2, -r2), 1e-6f));

    // 16QAM
    CHECK(c_near(modulate({0,0,0,0}, ModScheme::QAM16)[0], Complex(  r10,   r10), 1e-6f));
    CHECK(c_near(modulate({1,1,1,1}, ModScheme::QAM16)[0], Complex(-3*r10, -3*r10), 1e-6f));

    // 64QAM [0..0] -> (3+3j)/sqrt42
    CHECK(c_near(modulate({0,0,0,0,0,0}, ModScheme::QAM64)[0], Complex(3*r42, 3*r42), 1e-6f));

    // 256QAM [0..0] -> (5+5j)/sqrt170；[1..1] -> (-15-15j)/sqrt170
    CHECK(c_near(modulate({0,0,0,0,0,0,0,0}, ModScheme::QAM256)[0], Complex( 5*r170,  5*r170), 1e-6f));
    CHECK(c_near(modulate({1,1,1,1,1,1,1,1}, ModScheme::QAM256)[0], Complex(-15*r170, -15*r170), 1e-6f));

    // π/2-BPSK：实/虚轴交替
    Symbols p = modulate({0,0,0}, ModScheme::PI2_BPSK);
    CHECK(c_near(p[0], Complex( r2, 0), 1e-6f));
    CHECK(c_near(p[1], Complex(0,  r2), 1e-6f));
    CHECK(c_near(p[2], Complex( r2, 0), 1e-6f));
    Symbols p2 = modulate({0,1}, ModScheme::PI2_BPSK);
    CHECK(c_near(p2[1], Complex(0, -r2), 1e-6f));

    TEST_END();
}
