#include "nrphy/re_map.h"
#include <algorithm>

namespace nrphy {

bool map_to_grid(const Symbols& symbols, int n_rb, int n_symbols,
                 const std::vector<std::pair<int,int>>& skip,
                 std::vector<std::vector<Complex>>& grid) {
    const int n_sc = 12 * n_rb;
    grid.assign(n_symbols, std::vector<Complex>(n_sc, Complex(0, 0)));

    auto is_skip = [&](int l, int k) {
        return std::find(skip.begin(), skip.end(), std::make_pair(l, k)) != skip.end();
    };

    size_t s = 0;
    // 频率优先：先 k 增（同一符号内），再 l 增
    for (int l = 0; l < n_symbols; ++l) {
        for (int k = 0; k < n_sc; ++k) {
            if (is_skip(l, k)) continue;
            if (s >= symbols.size()) return false; // 符号不够
            grid[l][k] = symbols[s++];
        }
    }
    return s == symbols.size();
}

} // namespace nrphy
