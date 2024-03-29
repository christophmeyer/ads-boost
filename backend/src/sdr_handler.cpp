#include "sdr_handler.h"

#include <stdio.h>

#include <cstring>
#include <iomanip>
#include <iostream>

#include "rtl-sdr.h"

rtlsdr_dev_t *init_rtlsdr(uint32_t center_frequency, uint32_t sample_rate)
{
  char vendor[256], product[256], serial[256];
  int device_count = rtlsdr_get_device_count();
  printf("Number of rtl-sdr devices found: %i\n", device_count);

  for (int i = 0; i < device_count; i++)
  {
    rtlsdr_get_device_usb_strings(i, vendor, product, serial);
    printf("Vendor: %s\n", vendor);
    printf("Product: %s\n", product);
    printf("Serial: %s\n", serial);
  }

  rtlsdr_dev_t *dev = nullptr;
  if (device_count > 0)
  {
    rtlsdr_open(&dev, 0);
    rtlsdr_set_tuner_gain_mode(dev, 1);
    int gains[100];
    int n_gain_values;
    n_gain_values = rtlsdr_get_tuner_gains(dev, gains);
    int max_gain = gains[n_gain_values - 1];
    printf("Setting max gain value: %i\n", max_gain);
    rtlsdr_set_tuner_gain(dev, max_gain);
    rtlsdr_set_freq_correction(dev, 0);
    rtlsdr_set_center_freq(dev, center_frequency);
    rtlsdr_set_sample_rate(dev, sample_rate);
    rtlsdr_reset_buffer(dev);
    printf("Tuner gain at: %i\n", rtlsdr_get_tuner_gain(dev));
  }
  return dev;
}

void SDRHandler::read_data_sync(char *buffer, size_t len, int *n_read)
{
  int status = rtlsdr_read_sync(this->dev, buffer, len, n_read);
  if (status != 0)
  {
    throw std::runtime_error("rtlsdr_read_asyc returned with non-zero value.");
  }
}

void SDRHandler::read_data_async(SharedBuffer *buffer)
{
  int status = rtlsdr_read_async(this->dev, this->read_callback, buffer,
                                 this->n_buffers, this->buffer_len);
  if (status != 0)
  {
    throw std::runtime_error("rtlsdr_read_asyc returned with non-zero value.");
  }
}

void SDRHandler::read_callback(unsigned char *buf, uint32_t len, void *ctx)
{
  SharedBuffer *buffer = static_cast<SharedBuffer *>(ctx);

  {
    std::unique_lock<std::mutex> lock{buffer->mutex};

    std::copy(buffer->data.end() - BUFFER_OVERLAP, buffer->data.end(),
              buffer->data.begin());
    std::copy(buf, buf + len, buffer->data.begin() + BUFFER_OVERLAP);
  }
  buffer->has_data = true;
  buffer->len = len;
  buffer->data_ready.notify_one();
}

void SDRHandler::close() { rtlsdr_close(this->dev); }

SDRHandler::SDRHandler(int sample_frequency, int n_buffers, size_t buffer_len)
{
  this->sample_frequency = sample_frequency;
  this->buffer_len = buffer_len;
  this->dev = init_rtlsdr(1090000000, 2000000);
}
