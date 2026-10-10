// 调制：嵌套公式 vs 查表(LUT)。展示「把计算换成一次数组查找」的经典优化。
#include "bench_util.h"
#include "nrphy/modulator.h"
#include <cstdio>
#include <vector>

using namespace nrphy;

static Symbols build_lut(ModScheme scheme) {
    const int Qm = modulation_order(scheme);
    Symbols lut(1u << Qm);
    for (int i = 0; i < (1 << Qm); ++i) {
        Bits bits(Qm);
        for (int b = 0; b < Qm; ++b) bits[b] = (i >> (Qm - 1 - b)) & 1; // MSB 优先
        lut[i] = modulate(bits, scheme)[0];
    }
    return lut;
}

static void modulate_lut(const Bits& bits, const Symbols& lut, int Qm, Symbols& out) {
    const size_t nsym = bits.size() / Qm;
    out.resize(nsym);
    for (size_t i = 0; i < nsym; ++i) {
        int idx = 0;
        for (int b = 0; b < Qm; ++b) idx = (idx << 1) | bits[i * Qm + b];
        out[i] = lut[idx];
    }
}

static void run(const char* name, ModScheme scheme) {
    const int Qm = modulation_order(scheme);
    const size_t N = 1u << 18; // 比特数
    Bits bits(N);
    for (size_t i = 0; i < N; ++i) bits[i] = (i * 7 + 3) & 1;

    Symbols lut = build_lut(scheme);

    // 正确性（写入预分配缓冲，避免被优化）
    Symbols a, b;
    a = modulate(bits, scheme);
    modulate_lut(bits, lut, Qm, b);
    bool ok = (a.size() == b.size());
    for (size_t i = 0; ok && i < a.size(); ++i)
        ok = (a[i] == b[i]);
    std::printf("%-8s Qm=%d: LUT == formula ? %s\n", name, Qm, ok ? "YES" : "NO");

    // 计时（结果写入持久缓冲 a/b，防止死代码消除）
    const size_t reps = 200;
    double t_formula = bench::time_ns([&] { a = modulate(bits, scheme); }, reps);
    double t_lut     = bench::time_ns([&] { modulate_lut(bits, lut, Qm, b); }, reps);
    std::printf("  formula: %.1f ns, LUT: %.1f ns, speedup %.2fx\n", t_formula, t_lut, t_formula / t_lut);
}

int main() {
    run("QPSK", ModScheme::QPSK);
    run("16QAM", ModScheme::QAM16);
    run("64QAM", ModScheme::QAM64);
    run("256QAM", ModScheme::QAM256);
    return 0;
}
