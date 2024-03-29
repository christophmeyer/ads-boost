#include "demodulator.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>

#include "adsb_message.h"
#define BUFFER_LEN 16 * 16384

const std::string test_data_path = "../test/data/";

class DemodTest : public ::testing::Test {
 protected:
  DemodTest() {}
};

TEST_F(DemodTest, CRCTest) {
  std::array<unsigned char, 14> message = {0x8d, 0x3c, 0x64, 0x51, 0x99,
                                           0x0c, 0x03, 0x36, 0x18, 0x08,
                                           0x30, 0xdc, 0x81, 0xd2};
  uint32_t crc = calc_crc(&message);
  EXPECT_EQ(crc, 0xdc81d2);
}

TEST_F(DemodTest, CheckCRCTest) {
  std::array<unsigned char, 14> message = {0x8d, 0x4d, 0x24, 0x08, 0x99,
                                           0x08, 0xcb, 0x1b, 0xb8, 0x44,
                                           0x1c, 0x46, 0xa9, 0x9b};
  EXPECT_EQ(true, check_crc(&message));
}

TEST_F(DemodTest, CheckDemodulate) {
  std::string filename = test_data_path + "raw_iq_testdata.bin";
  std::ifstream file(filename, std::ios::binary);
  std::vector<std::array<unsigned char, 14>> messages;
  std::array<unsigned char, BUFFER_LEN + BUFFER_OVERLAP> data;
  if (!file) {
    std::cerr << "Cannot open file: " << filename << std::endl;
    return;
  }
  unsigned char byte;
  int n = 0;
  while (file.read(reinterpret_cast<char *>(&byte), sizeof(byte)) &&
         n < BUFFER_LEN + BUFFER_OVERLAP) {
    data[n] = byte;
    n++;
  }
  file.close();
  Demodulator demodulator = Demodulator();
  demodulator.Demodulate(&data, data.size(), &messages);
  EXPECT_EQ(ADSBMessage(messages[0]).HexString(),
            "8f4d2023587f345e35837e2218b2");
  EXPECT_EQ(ADSBMessage(messages[1]).HexString(),
            "8d4d2023991094ad487c14fc9e3d");
  EXPECT_EQ(ADSBMessage(messages[2]).HexString(),
            "8d4d202358792453ef858bae7fc9");
  EXPECT_EQ(ADSBMessage(messages[3]).HexString(),
            "8f4d20235877d0bc7d99551e27ca");
  EXPECT_EQ(ADSBMessage(messages[4]).HexString(),
            "8f4d20235877b0bc01996ff7b3f2");
  EXPECT_EQ(ADSBMessage(messages[5]).HexString(),
            "8f4d2023991093ad287c148accdc");
  EXPECT_EQ(ADSBMessage(messages[6]).HexString(),
            "8f4d20232004d0f4cb1820000d24");
  EXPECT_EQ(ADSBMessage(messages[7]).HexString(),
            "8f4d20235877a0bbbf997cdb827b");
  EXPECT_EQ(ADSBMessage(messages[8]).HexString(),
            "8f4d2023991093ad287c13751cf8");
  EXPECT_EQ(ADSBMessage(messages[9]).HexString(),
            "8f4d2023587790bba5998227c948");
  EXPECT_EQ(ADSBMessage(messages[10]).HexString(),
            "8f4d2023991093ad287c148accdc");
  EXPECT_EQ(ADSBMessage(messages[11]).HexString(),
            "8f4d202358779451f985edf9f21e");
  EXPECT_EQ(ADSBMessage(messages[12]).HexString(),
            "8f4d2023991093ad087c133060d1");
}