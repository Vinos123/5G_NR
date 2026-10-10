#include "nrphy/layer_map.h"
#include "test_util.h"

using namespace nrphy;

int main() {
    Symbols d;
    for (int i = 0; i < 6; ++i) d.push_back(Complex(float(i), 0.0f));

    auto l1 = layer_map(d, 1);
    CHECK_EQ(l1.size(), 1u);
    CHECK(l1[0] == d);

    auto l2 = layer_map(d, 2);   // x^0 = d(0),d(2),d(4); x^1 = d(1),d(3),d(5)
    CHECK_EQ(l2.size(), 2u);
    CHECK(l2[0][0] == d[0] && l2[0][1] == d[2] && l2[0][2] == d[4]);
    CHECK(l2[1][0] == d[1] && l2[1][1] == d[3] && l2[1][2] == d[5]);

    auto l3 = layer_map(d, 3);   // x^0 = d(0),d(3); x^1 = d(1),d(4); x^2 = d(2),d(5)
    CHECK_EQ(l3.size(), 3u);
    CHECK(l3[0][0] == d[0] && l3[0][1] == d[3]);
    CHECK(l3[1][0] == d[1] && l3[1][1] == d[4]);
    CHECK(l3[2][0] == d[2] && l3[2][1] == d[5]);

    TEST_END();
}
