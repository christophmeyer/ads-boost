#include <math.h>
#include <rtl-sdr.h>
#include <stdio.h>
#include <unistd.h>

#include <condition_variable>
#include <cxxopts.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

#include "adsb_message.h"
#include "config.h"
#include "contact.h"
#include "demodulator.h"
#include "sdr_handler.h"
#include "webserver.h"

void ingest_raw_iq_data(SharedBuffer *buffer) {
  int n_buffers = 12;
  int sample_frequency = 2000000;

  // Read:
  SDRHandler handler = SDRHandler(sample_frequency, n_buffers, BUFFER_LEN);
  if (handler.dev != nullptr) {
    handler.read_data_async(buffer);
  }
  // close device
  handler.close();
}

void ingest_from_file(std::string filename, SharedBuffer *buffer) {
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Cannot open file: " << filename << std::endl;
    return;
  }
  unsigned char byte;
  while (1) {
    {
      std::unique_lock<std::mutex> lock{buffer->mutex};
      for (size_t n = 0; n < buffer->data.size(); n++) {
        if (n < BUFFER_OVERLAP) {
          // fill first BUFFER_OVERLAP bytes with last BUFFER_OVERLAP from
          // previous buffer
          buffer->data.at(n) = buffer->data.at(BUFFER_LEN + n);
          continue;
        }
        if (file.read(reinterpret_cast<char *>(&byte), sizeof(byte))) {
          buffer->data.at(n) = byte;
        } else {
          // file ended
          buffer->has_data = true;
          buffer->has_more = false;
          buffer->len = n;
          buffer->data_ready.notify_one();
          file.close();
          return;
        }
      }
      buffer->has_data = true;
      buffer->data_ready.notify_one();
      buffer->data_ready.wait(lock, [buffer] { return !buffer->has_data; });
    }
  }
}

void append_demod_output_file(const std::string &filename,
                              std::vector<ADSBMessage> decoded_messages) {
  // Open file for output in append mode; create it if it does not exist
  std::ofstream file(filename,
                     std::ios::out | std::ios::binary | std::ios::app);

  if (!file) {
    std::cerr << "Error opening file for writing.\n";
    return;
  }

  // update the contactlist
  for (const auto &msg : decoded_messages) {
    file.write(reinterpret_cast<const char *>(msg.message.data()),
               msg.message.size());

    // Serialize and write the timestamp
    auto duration_since_epoch = msg.timestamp.time_since_epoch();

    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    duration_since_epoch)
                    .count();
    file.write(reinterpret_cast<const char *>(&time), sizeof(time));

    if (!file) {
      std::cerr << "Error writing to file.\n";
    }
  }
  file.close();
}

void append_raw_output_file(
    const std::string &filename,
    const std::array<unsigned char, BUFFER_LEN + BUFFER_OVERLAP> *data) {
  // Open file for output in append mode; create it if it does not exist
  std::ofstream file(filename,
                     std::ios::out | std::ios::binary | std::ios::app);

  if (!file) {
    std::cerr << "Error opening file for writing.\n";
    return;
  }

  // Write the contents of the array to the file
  file.write(reinterpret_cast<const char *>(data->data()), data->size());

  if (!file) {
    std::cerr << "Error writing to file.\n";
  }

  file.close();
}

std::string to_string_with_precision(double value, int precision) {
  std::ostringstream out;
  out << std::fixed << std::setprecision(precision) << value;
  return out.str();
}

std::string display_bool_value(BoolValue value) {
  switch (value) {
    case TRUE:
      return "ON";
      // break;
    case FALSE:
      return "OFF";
    case UNDETERMINED_BOOL:
      return "";
    default:
      return "";
  }
}

void draw_contact_table(ContactList contacts) {
  auto now = std::chrono::system_clock::now();
  auto now_in_seconds = std::chrono::system_clock::to_time_t(now);
  std::string spinner = "";
  switch (now_in_seconds % 6) {
    case 0:
      spinner = "   ";
      break;
    case 1:
      spinner = ".  ";
      break;
    case 2:
      spinner = ".. ";
      break;
    case 3:
      spinner = "...";
      break;
    case 4:
      spinner = " ..";
      break;
    case 5:
      spinner = "  .";
      break;
    default:
      spinner = "   ";
      break;
  }

  const int col_width = 12;
  std::cout << "\033[2J\033[1;1H";
  std::cout << std::right << std::setw(6) << "ICAO" << std::setw(col_width)
            << "CALLSIGN" << std::setw(col_width) << "LAT"
            << std::setw(col_width) << "LON" << std::setw(col_width) << "ALT"
            << std::setw(col_width) << "SELECT_ALT" << std::setw(col_width)
            << "SPEED" << std::setw(col_width) << "TRACK"
            << std::setw(col_width) << "SELECT_HDG" << std::setw(col_width)
            << "VRATE" << std::setw(col_width) << "TYPE" << std::setw(col_width)
            << "APILOT" << std::setw(col_width) << "APPRMODE"
            << std::setw(col_width) << "ALT_HOLD" << std::setw(col_width)
            << "TCAS" << std::setw(col_width) << "MESSAGES"
            << std::setw(col_width) << "SEEN" << std::setw(col_width)

            << spinner << std::endl;

  std::cout << std::setfill('-') << std::setw(18 * col_width) << ""
            << std::endl;
  std::cout << std::setfill(' ');

  for (Contact &contact : contacts.contacts) {
    std::cout << std::right << std::setprecision(4) << std ::fixed
              << std::setw(6) << contact.icao << std::setw(col_width)
              << contact.callsign << std::setw(col_width)
              << ((contact.position_status != UNDETERMINED)
                      ? to_string_with_precision(contact.lat, 4)
                      : "")
              << std::setw(col_width)
              << ((contact.position_status != UNDETERMINED)
                      ? to_string_with_precision(contact.lon, 4)
                      : "")
              << std::setw(col_width)
              << ((contact.altitude_type != UNDETERMINED_ALT)
                      ? std::to_string(contact.altitude)
                      : "")
              << std::setw(col_width)
              << ((contact.selected_altitude_status != UNDETERMINED)
                      ? std::to_string(contact.selected_altitude)
                      : "")
              << std::setw(col_width)
              << ((contact.speed_type != UNDETERMINED_SPEED)
                      ? to_string_with_precision(contact.speed, 2)
                      : "")
              << std::setw(col_width)
              << ((contact.heading_type != UNDETERMINED_HEADING)
                      ? to_string_with_precision(contact.heading, 2)
                      : "")
              << std::setw(col_width)
              << ((contact.selected_heading_status != UNDETERMINED)
                      ? to_string_with_precision(contact.selected_heading, 2)
                      : "")
              << std::setw(col_width)
              << ((contact.vertical_rate_source != UNDETERMINED_VR_SOURCE)
                      ? std::to_string(contact.vertical_rate)
                      : "")
              << std::setw(col_width) << contact.aircraft_category
              << std::setw(col_width) << display_bool_value(contact.autopilot)
              << std::setw(col_width)
              << display_bool_value(contact.approach_mode)
              << std::setw(col_width)
              << display_bool_value(contact.altitude_hold_mode)
              << std::setw(col_width)
              << display_bool_value(contact.tcas_operational)
              << std::setw(col_width) << contact.n_messages
              << std::setw(col_width) << contact.last_seen() << std::endl;
  }
}

void read_demod_messages(std::string filename,
                         std::vector<ADSBMessage> *messages) {
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file for reading.\n";
    return;
  }

  while (file) {
    std::array<unsigned char, 14> message;
    file.read(reinterpret_cast<char *>(message.data()), message.size());

    long long time_count;
    file.read(reinterpret_cast<char *>(&time_count), sizeof(time_count));
    std::chrono::system_clock::time_point timestamp =
        std::chrono::system_clock::time_point(
            std::chrono::milliseconds(time_count));
    messages->push_back(ADSBMessage(message, timestamp));
  }
  return;
}
int main(int argc, char **argv) {
  cxxopts::Options options("ads-boost", "Your awesome ads-b tracker.");

  options.add_options()("r, in_raw", "Path to file with raw IQ data.",
                        cxxopts::value<std::string>())(
      "i, in_demod", "Path to file with raw IQ data.",
      cxxopts::value<std::string>())("f, out_raw",
                                     "Path to dir where do dump raw IQ data.",
                                     cxxopts::value<std::string>())(
      "o, out_demod", "Path to dir where do dump demodulated messages.",
      cxxopts::value<std::string>())(
      "c,contacts", "Enable display of contacts table.",
      cxxopts::value<bool>()->default_value("false"))(
      "n,net", "Enable webserver to display contacts.",
      cxxopts::value<bool>()->default_value("false"))(
      "m,messages", "Print all decoded messages.",
      cxxopts::value<bool>()->default_value("false"))(
      "p,port", "Port for the websocket to broadcast contacts on.",
      cxxopts::value<int>()->default_value("9001"))(
      "t,timeout",
      "Seconds of timeout after which contact is removed from contacts "
      "table.",
      cxxopts::value<int>()->default_value("90"))(
      "b,lat_ref", "Latitude reference for ground position messages.",
      cxxopts::value<double>()->default_value("0.0"))(
      "l,lon_ref", "Longitude reference for ground position messages.",
      cxxopts::value<double>()->default_value("0.0"))("h,help",
                                                      "Usage of ads-boost.");

  cxxopts::ParseResult result;

  try {
    result = options.parse(argc, argv);
  } catch (const cxxopts::exceptions::no_such_option &e) {
    std::cout << e.what() << std::endl;
    exit(1);
  }

  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  bool print_contacts_table = result["contacts"].as<bool>();
  bool network = result["net"].as<bool>();
  bool print_messages = result["messages"].as<bool>();
  if (print_contacts_table && print_messages) {
    std::cout << "Can only either print messages OR display contacts table."
              << std::endl;
    exit(0);
  }
  std::string input_file_path;
  if (result.count("in_raw"))
    input_file_path = result["in_raw"].as<std::string>();
  std::string input_demod_file_path;
  if (result.count("in_demod"))
    input_demod_file_path = result["in_demod"].as<std::string>();
  std::string output_dir;
  std::string output_demod_dir;
  if (result.count("out_raw")) {
    output_dir = result["out_raw"].as<std::string>();
  }
  if (result.count("out_demod")) {
    output_demod_dir = result["out_demod"].as<std::string>();
  }
  int timeout_seconds = result["timeout"].as<int>();
  int port = result["port"].as<int>();
  double lat_ref = result["lat_ref"].as<double>();
  double lon_ref = result["lon_ref"].as<double>();

  SharedContactList contacts;
  contacts.contact_list = ContactList(timeout_seconds, lat_ref, lon_ref);
  SharedBuffer buffer;

  // Start websocket server thread
  std::thread network_thread;
  if (network) {
    network_thread = std::thread(run_webserver, &contacts, port);
  }

  // ingest raw data, either from file or rtlsdr buffer
  std::thread ingest_thread;
  if (!result.count("in_demod")) {
    if (result.count("in_raw")) {
      ingest_thread = std::thread(ingest_from_file, input_file_path, &buffer);
    } else {
      ingest_thread = std::thread(ingest_raw_iq_data, &buffer);
    }
  }

  // write demodulated messages and/or raw data to disk
  std::ostringstream oss;
  auto timestamp = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
  oss << std::put_time(std::localtime(&time), "%Y_%m_%d_%H_%M_%S");
  std::string full_output_path;
  if (result.count("out_raw")) {
    full_output_path = output_dir + oss.str() + ".bin";
  }
  std::string full_output_demod_path;
  if (result.count("out_demod")) {
    full_output_demod_path = output_demod_dir + oss.str() + "_demod.bin";
  }

  int counter = 0;
  while (1) {
    std::vector<ADSBMessage> decoded_messages;
    bool has_more = true;
    std::vector<std::array<unsigned char, 14>> messages;
    if (!result.count("in_demod")) {
      std::unique_lock<std::mutex> lock{buffer.mutex};
      buffer.data_ready.wait(lock, [&buffer] { return buffer.has_data; });

      // Demod: raw bytes -> raw messages
      Demodulator demodulator = Demodulator();
      demodulator.Demodulate(&buffer.data, buffer.len, &messages);

      if (result.count("out_raw")) {
        append_raw_output_file(full_output_path, &buffer.data);
      }
      if (buffer.has_more) {
        buffer.has_data = false;
        buffer.data_ready.notify_one();
      } else {
        has_more = false;
      }
      // decode messages
      for (std::size_t n_msg = 0; n_msg < messages.size(); n_msg++) {
        ADSBMessage msg = ADSBMessage(messages[n_msg]);
        decoded_messages.push_back(msg);
      }
    } else {
      // read full demod file
      read_demod_messages(input_demod_file_path, &decoded_messages);
      has_more = false;
    }

    // update the contactlist
    {
      std::unique_lock<std::mutex> lock{contacts.mutex};
      for (const auto &msg : decoded_messages) {
        if (msg.downlink_format == 17) {
          counter++;
          contacts.contact_list.update(msg);
        }
      }
      contacts.contacts_updated = true;
      contacts.contacts_ready.notify_all();
    }

    // draw contacts table
    if (print_contacts_table) {
      draw_contact_table(contacts.contact_list);
    }

    // decode and print decoded messages
    if (print_messages) {
      for (auto &msg : decoded_messages) {
        std::cout << "==============================================="
                  << std::endl;
        msg.PrintMessage();
        // }
      }
    }

    if (result.count("out_demod")) {
      append_demod_output_file(full_output_demod_path, decoded_messages);
    }

    if (!has_more) {
      break;
    }
  }

  std::cout << "Counter: " << counter << std::endl;
  if (network) {
    network_thread.join();
  }
  if (!result.count("in_demod")) {
    ingest_thread.join();
  }
  return 0;
}
