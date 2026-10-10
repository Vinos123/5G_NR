#include "nrphy/precode.h"
#include <cassert>

namespace nrphy {

std::vector<Symbols> precode(const std::vector<Symbols>& layers, int n_ports) {
    assert(n_ports == 1 && "reference impl covers single antenna port only");
    assert(layers.size() == 1);
    // P=1, ν=1：W = [1]，z = x
    return layers;
}

} // namespace nrphy
