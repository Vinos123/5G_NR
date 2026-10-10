#include "nrphy/crc.h"
#include "test_util.h"

using namespace nrphy;

int main() {
    // 手工推导向量：CRC16 单比特 '1' = D^16 mod g = D^12+D^5+1 = 0x1021
    CHECK_EQ(crc(Bits{1}, CrcType::CRC16), 0x1021u);
    // CRC16 序列 [1,0] = D^17 mod g = D^13+D^6+D = 0x2042
    CHECK_EQ(crc(Bits{1, 0}, CrcType::CRC16), 0x2042u);

    // 零余数自检：对任意消息，crc(msg || parity) 必须为 0
    const Bits msg = {1,0,1,1,0,0,1,0,1,0,0,0,1,1,1,0,1};
    for (CrcType t : {CrcType::CRC24A, CrcType::CRC24B, CrcType::CRC24C, CrcType::CRC16}) {
        Bits enc = msg;
        crc_attach(enc, t);
        CHECK_EQ(crc(enc, t), 0u);
        CHECK_EQ(enc.size(), msg.size() + crc_len(t));
    }

    // crc_attach 保留前缀
    Bits msg2 = {1,1,0,1,0};
    Bits enc2 = msg2;
    crc_attach(enc2, CrcType::CRC24B);
    for (size_t i = 0; i < msg2.size(); ++i) CHECK_EQ(enc2[i], msg2[i]);

    CHECK_EQ(crc_len(CrcType::CRC24A), 24u);
    CHECK_EQ(crc_len(CrcType::CRC16), 16u);

    TEST_END();
}
