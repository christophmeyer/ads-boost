#ifndef ADSBOOST_SDR_HANDLER_H_
#define ADSBOOST_SDR_HANDLER_H_

#include <array>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "config.h"
#include "rtl-sdr.h"
#include "stddef.h"

struct SharedBuffer {
  std::array<unsigned char, BUFFER_LEN + BUFFER_OVERLAP> data;
  std::mutex mutex;
  std::condition_variable data_ready;
  bool has_data = false;
  bool has_more = true;
  size_t len = BUFFER_LEN + BUFFER_OVERLAP;
};

class SDRHandler {
 public:
  int sample_frequency;
  int n_buffers = 12;
  size_t buffer_len = BUFFER_LEN;
  rtlsdr_dev_t *dev = NULL;

  SDRHandler(int sample_frequency, int n_buffers, size_t buffer_len);
  void read_data_sync(char *buffer, size_t len, int *n_read);
  void read_data_async(SharedBuffer *buffer);
  void close();
  static void read_callback(unsigned char *buf, u_int32_t len, void *ctx);
};

#endif  // ADSBOOST_SDR_HANDLER_H_