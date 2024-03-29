#ifndef ADSBOOST_DEMODULATOR_H_
#define ADSBOOST_DEMODULATOR_H_

#include <array>
#include <cstdint>
#include <vector>

#include "config.h"

struct rawMessage {
  std::array<char, 14> bytes;
};

uint32_t calc_crc(std::array<unsigned char, 14> *msg);
bool check_crc(std::array<unsigned char, 14> *msg);

class Demodulator {
 public:
  int sample_frequency;
  std::array<uint16_t, 129 * 129 * 2> magnitude_lookup;

  Demodulator();
  void Demodulate(std::array<unsigned char, BUFFER_LEN + 480> *buffer,
                  uint32_t len,
                  std::vector<std::array<unsigned char, 14>> *messages);
};

#endif  // ADSBOOST_DEMODULATOR_H_