#include "nrphy/scrambler.h"
#include "nrphy/gold.h"

namespace nrphy {

void scramble(const Bits& in, Bits& out, uint32_t n_rnti, uint16_t n_id) {
    const uint32_t c_init = (n_rnti << 15) | static_cast<uint32_t>(n_id);
    Gold gold(c_init);
    out.resize(in.size());
    for (size_t i = 0; i < in.size(); ++i)
        out[i] = static_cast<Bit>(in[i] ^ gold.next());
}

} // namespace nrphy
