#pragma once
// PUSCH 发送链顶层（38.212 §6.2 比特级 + 38.211 §6.3.1 符号级）。
//   把各模块串成完整 14 步，供端到端示例与文档交叉引用。
#include "nrphy/types.h"
#include "nrphy/ldpc.h"
#include "nrphy/modulator.h"
#include "nrphy/segmentation.h"

namespace nrphy {

struct PuschConfig {
    LdpcBG bg = LdpcBG::BG2;          // 基图（可先 select_base_graph 选定）
    int rv = 0;                        // 冗余版本
    ModScheme mod = ModScheme::QPSK;   // 调制
    int n_layers = 1;                  // 层数（参考实现支持 1）
    bool transform_precoding = false;  // true = DFT-s-OFDM
    uint32_t n_rnti = 1;               // 加扰 RNTI
    uint16_t n_id = 0;                 // 加扰 ID
    int nref = 0;                      // 受限缓冲 N_ref（0=不限）
};

// 步骤 1-7（比特级）：TB(A 比特) -> 拼接码字 f(G 比特)。
//   返回的 PuschConfig 不变；中间参数（C/Zc/K）可经下面的结果结构拿到。
Bits pusch_encode_bits(const Bits& tb, const PuschConfig& cfg, int G);

// 步骤 8-12（符号级）：码字 f -> 预编码符号 z（单天线端口）。
Symbols pusch_encode_symbols(const Bits& f, const PuschConfig& cfg);

// 完整 14 步：TB -> 资源栅格（单天线端口）。返回每 OFDM 符号每子载波的复符号。
//   skip 为 DMRS/PT-RS 位置；n_symbols 为 PUSCH 占用的 OFDM 符号数。
std::vector<std::vector<Complex>>
pusch_tx(const Bits& tb, const PuschConfig& cfg, int G, int n_rb, int n_symbols,
         const std::vector<std::pair<int,int>>& skip);

} // namespace nrphy
