#include "nrphy/layer_map.h"
#include <cassert>

namespace nrphy {

std::vector<Symbols> layer_map(const Symbols& d, int v) {
    assert(v >= 1);
    const size_t M = d.size();
    assert(M % static_cast<size_t>(v) == 0);
    const size_t per = M / v;

    std::vector<Symbols> layers(v, Symbols(per));
    for (size_t i = 0; i < per; ++i)
        for (int l = 0; l < v; ++l)
            layers[l][i] = d[v * i + l];
    return layers;
}

} // namespace nrphy
