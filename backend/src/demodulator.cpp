#include "demodulator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

#include "adsb_message.h"

Demodulator::Demodulator() {
  for (int i = 0; i <= 128; i++) {
    for (int q = 0; q <= 128; q++) {
      magnitude_lookup[i * 129 + q] =
          std::round(std::sqrt(i * i + q * q) * 360);
    }
  }
}
uint32_t modes_checksum_table[112] = {
    0x3935ea, 0x1c9af5, 0xf1b77e, 0x78dbbf, 0xc397db, 0x9e31e9, 0xb0e2f0,
    0x587178, 0x2c38bc, 0x161c5e, 0x0b0e2f, 0xfa7d13, 0x82c48d, 0xbe9842,
    0x5f4c21, 0xd05c14, 0x682e0a, 0x341705, 0xe5f186, 0x72f8c3, 0xc68665,
    0x9cb936, 0x4e5c9b, 0xd8d449, 0x939020, 0x49c810, 0x24e408, 0x127204,
    0x093902, 0x049c81, 0xfdb444, 0x7eda22, 0x3f6d11, 0xe04c8c, 0x702646,
    0x381323, 0xe3f395, 0x8e03ce, 0x4701e7, 0xdc7af7, 0x91c77f, 0xb719bb,
    0xa476d9, 0xadc168, 0x56e0b4, 0x2b705a, 0x15b82d, 0xf52612, 0x7a9309,
    0xc2b380, 0x6159c0, 0x30ace0, 0x185670, 0x0c2b38, 0x06159c, 0x030ace,
    0x018567, 0xff38b7, 0x80665f, 0xbfc92b, 0xa01e91, 0xaff54c, 0x57faa6,
    0x2bfd53, 0xea04ad, 0x8af852, 0x457c29, 0xdd4410, 0x6ea208, 0x375104,
    0x1ba882, 0x0dd441, 0xf91024, 0x7c8812, 0x3e4409, 0xe0d800, 0x706c00,
    0x383600, 0x1c1b00, 0x0e0d80, 0x0706c0, 0x038360, 0x01c1b0, 0x00e0d8,
    0x00706c, 0x003836, 0x001c1b, 0xfff409, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000};

uint32_t calc_crc(std::array<unsigned char, 14> *msg) {
  uint32_t crc = 0;
  int j;

  for (j = 0; j < 112; j++) {
    int byte = j / 8;
    int bit = j % 8;
    int bitmask = 1 << (7 - bit);

    /* If bit is set, xor with corresponding table entry. */
    if (msg->at(byte) & bitmask) crc ^= modes_checksum_table[j];
  }
  return crc; /* 24 bit checksum. */
}

bool check_crc(std::array<unsigned char, 14> *msg) {
  uint32_t crc = calc_crc(msg);
  uint32_t checksum = msg->at(11) << 16 | msg->at(12) << 8 | msg->at(13);
  return crc == checksum;
}

void Demodulator::Demodulate(
    std::array<unsigned char, BUFFER_LEN + BUFFER_OVERLAP> *buffer,
    uint32_t len, std::vector<std::array<unsigned char, 14>> *messages) {
  std::array<u_int16_t, (BUFFER_LEN + BUFFER_OVERLAP) / 2> magnitudes;

  size_t data_len = std::min(BUFFER_LEN + BUFFER_OVERLAP, (int)len);
  // Calculate magnitudes
  for (size_t n = 0; n < data_len; n += 2) {
    int i = std::abs(buffer->at(n) - 127);
    int q = std::abs(buffer->at(n + 1) - 127);
    magnitudes[n / 2] = magnitude_lookup[i * 129 + q];
  }
  for (size_t n = 0; n < data_len / 2 - 239; n++) {
    if (!(magnitudes[n] > magnitudes[n + 1] &&
          magnitudes[n + 1] < magnitudes[n + 2] &&
          magnitudes[n + 2] > magnitudes[n + 3] &&
          magnitudes[n + 3] < magnitudes[n] &&
          magnitudes[n + 4] < magnitudes[n] &&
          magnitudes[n + 5] < magnitudes[n] &&
          magnitudes[n + 6] < magnitudes[n] &&
          magnitudes[n + 7] > magnitudes[n + 8] &&
          magnitudes[n + 8] < magnitudes[n + 9] &&
          magnitudes[n + 9] > magnitudes[n + 6])) {
      continue;
    }
    uint32_t high = magnitudes[n] + magnitudes[n + 2] + magnitudes[n + 7] +
                    magnitudes[n + 9];

    if ((magnitudes[n + 4] >= high) || (magnitudes[n + 5] >= high)) {
      continue;
    }
    if ((magnitudes[n + 11] >= high) || (magnitudes[n + 12] >= high) ||
        (magnitudes[n + 13] >= high) || (magnitudes[n + 14] >= high)) {
      continue;
    }
    std::array<unsigned char, 14> message;
    bool error = false;
    for (int n_byte = 0; n_byte < 14; n_byte++) {
      unsigned char byte = 0;
      for (int n_bit = 0; n_bit < 8; n_bit++) {
        int i = 8 * n_byte + n_bit;
        uint16_t low = magnitudes[n + 2 * i + 16];
        uint16_t high = magnitudes[n + 2 * i + 16 + 1];
        uint32_t delta = std::abs(low - high);
        if (i > 0 && delta < 255) {
          if (n_bit == 0) {
            byte = byte | ((message[n_byte - 1] & 1) << 7);
          } else {
            byte = byte | ((byte & (1 << (7 - n_bit + 1))) >> 1);
          }
        } else if (low == high) {
          error = true;
        } else if (low > high) {
          byte = byte | (1 << (7 - n_bit));
        }
      }
      message[n_byte] = byte;
    }
    if (error) {
      continue;
    }

    // Only ADS-B messages for now
    int downlink_format = message[0] >> 3;
    if (!(downlink_format == 17 || downlink_format == 18)) {
      continue;
    }
    if (check_crc(&message)) {
      messages->push_back(message);
    }
  }
}
