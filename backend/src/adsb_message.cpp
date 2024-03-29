#include "adsb_message.h"

#include <math.h>
#include <stdio.h>

#include <iomanip>
#include <iostream>
#include <sstream>

ADSBMessage::ADSBMessage(std::array<unsigned char, 14> message) {
  timestamp = std::chrono::system_clock::now();
  this->init(message, timestamp);
}

ADSBMessage::ADSBMessage(std::array<unsigned char, 14> message,
                         std::chrono::system_clock::time_point timestamp) {
  this->init(message, timestamp);
}

void ADSBMessage::init(std::array<unsigned char, 14> message,
                       std::chrono::system_clock::time_point timestamp) {
  this->message = message;
  this->timestamp = timestamp;
  downlink_format = decode_downlink_format(message);
  downlink_capability = decode_capability(message);
  icao = decode_icao(message);
  type_code = decode_msg_type(message);

  if (type_code >= 1 && type_code <= 4) {
    // aircraft identification
    callsign = decode_callsign(message);
    aircraft_category = decode_category(message);
  } else if (type_code >= 5 && type_code <= 8) {
    // surface position
    speed = decode_ground_movement(message, speed_type);
    heading = decode_ground_track(message, heading_type);

    cpr_format = decode_cpr_format(message);
    lat_cpr = get_lat_cpr(message);
    lon_cpr = get_lon_cpr(message);
    cpr_status = KNOWN;

  } else if (type_code >= 9 && type_code <= 18) {
    // airborne position /w barometric altitude
    cpr_format = decode_cpr_format(message);
    lat_cpr = get_lat_cpr(message);
    lon_cpr = get_lon_cpr(message);
    cpr_status = KNOWN;
    altitude = decode_barometric_altitude(message, altitude_type);

  } else if (type_code == 19) {
    // airborne velocities
    tc19_subtype = decode_tc19_subtype(message);

    intent_change_flag = decode_intent_change_flag(message);
    ifr_capability_flag = decode_ifr_capability_flag(message);
    nav_uncertainty_category = decode_nav_uncertainty_category(message);
    nav_uncertainty_category_status = KNOWN;
    vertical_rate_source = decode_vertical_rate_source(message);
    vertical_rate = decode_vertical_rate(message, vertical_rate_status);
    altitude_delta = decode_altitude_delta(message, altitude_delta_status);

    if (tc19_subtype == 1 || tc19_subtype == 2) {
      speed = decode_ground_speed(message, speed_type);
      heading = decode_track_angle(message);
      heading_type = TRACK_ANGLE;
    } else if (tc19_subtype == 3 || tc19_subtype == 4) {
      speed = decode_airspeed(message, speed_type);
      heading = decode_magnetic_heading(message);
      heading_type = MAGNETIC;
    }

  } else if (type_code >= 20 && type_code <= 22) {
    // airborne position w/ GNSS height
    cpr_format = decode_cpr_format(message);
    lat_cpr = get_lat_cpr(message);
    lon_cpr = get_lon_cpr(message);
    cpr_status = KNOWN;

    altitude = decode_gnss_altitude(message, altitude_type);

  } else if (type_code == 29) {
    int subtype = (message[4] >> 1) & 3;
    if (subtype == 1) {
      // ADSB version 2
      selected_altitude = decode_selected_altitude(message);
      selected_altitude_status = KNOWN;

      selected_altitude_source = decode_selected_altitude_source(message);
      vnav_mode = decode_vnav_mode(message);
      lnav_mode = decode_lnav_mode(message);
      autopilot = decode_autopilot(message);
      approach_mode = decode_approach_mode(message);
      altitude_hold_mode = decode_altitude_hold_mode(message);

      selected_heading =
          decode_selected_heading(message, selected_heading_status);
      baro_pressure_setting = decode_baro_pressure_setting(message);
      baro_pressure_setting_status = KNOWN;

    } else if (subtype == 0) {
      // TODO: ADSB version 1
    }

    tcas_operational = decode_tcas_operational(message);
  }
}
std::string ADSBMessage::HexString() {
  std::stringstream hex_stream;
  for (int n = 0; n < 14; n++) {
    int byte = static_cast<int>(message[n]);
    // Print each byte in hex format
    hex_stream << std::hex << std::setw(2) << std::setfill('0') << byte;
  }
  return hex_stream.str();
}

void ADSBMessage::PrintMessage() {
  std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
  std::cout << "0x" << this->HexString() << std::endl;
  std::cout << "Received "
            << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
            << std::endl;
  std::cout << "DF: " << downlink_format << std::endl;
  std::cout << "Capability: " << downlink_capability << std::endl;
  std::cout << "ICAO: " << icao << std::endl;
  std::cout << "Type code: " << type_code << " ("
            << detailed_type_code(type_code) << ")\n"
            << std::endl;

  if (type_code >= 1 && type_code <= 4) {
    // aircraft identification
    std::cout << "Aircraft category: " << aircraft_category << " ("
              << detailed_aircraft_category(type_code, aircraft_category) << ")"
              << std::endl;
    std::cout << "Callsign: " << callsign << std::endl;
  } else if (type_code >= 5 && type_code <= 8) {
    // surface position
    std::cout << "speed: " << speed << std::endl;
    std::cout << "speed type: " << speed_type_value_to_string(speed_type)
              << std::endl;
    std::cout << "heading: " << heading << std::endl;
    std::cout << "heading type: " << heading_type_value_to_string(heading_type)
              << std::endl;

    std::cout << "CPR format: " << cpr_format << std::endl;
    std::cout << "CPR LAT: " << lat_cpr << std::endl;
    std::cout << "CPR LON: " << lon_cpr << std::endl;

  } else if (type_code >= 9 && type_code <= 18) {
    // airborne position /w barometric altitude
    std::cout << "CPR format: " << cpr_format << std::endl;
    std::cout << "CPR LAT: " << lat_cpr << std::endl;
    std::cout << "CPR LON: " << lon_cpr << std::endl;
    std::cout << "ALT: " << altitude << " ft" << std::endl;
    std::cout << "ALT Type: " << altitude_type_to_string(altitude_type)
              << std::endl;

  } else if (type_code == 19) {
    // airborne velocities
    std::cout << "TC19 subtype: " << tc19_subtype << std::endl;
    std::cout << "Intent change flag: "
              << bool_value_to_string(intent_change_flag) << std::endl;
    std::cout << "IFR capability flag: "
              << bool_value_to_string(ifr_capability_flag) << std::endl;
    std::cout << "Nav uncertainty category: " << nav_uncertainty_category
              << std::endl;
    std::cout << "VR source: "
              << vertical_rate_source_to_string(vertical_rate_source)
              << std::endl;
    std::cout << "Vertical rate: " << vertical_rate << " ft/min" << std::endl;
    std::cout << "delta h: " << altitude_delta << " ft" << std::endl;

    std::cout << "speed: " << speed << " kt" << std::endl;
    std::cout << "speed type: " << speed_type_value_to_string(speed_type)
              << std::endl;
    std::cout << "heading: " << heading << std::endl;
    std::cout << "heading type: " << heading_type_value_to_string(heading_type)
              << std::endl;

  } else if (type_code >= 20 && type_code <= 22) {
    // airborne position w/ GNSS height
    std::cout << "CPR format: " << cpr_format << std::endl;
    std::cout << "CPR LAT: " << lat_cpr << std::endl;
    std::cout << "CPR LON: " << lon_cpr << std::endl;
    std::cout << "ALT: " << altitude << " ft" << std::endl;
    std::cout << "ALT Type: " << altitude_type_to_string(altitude_type)
              << std::endl;
  } else if (type_code == 29) {
    // airborne position w/ GNSS height
    std::cout << "selected ALT: " << selected_altitude << " ft" << std::endl;
    std::cout << "selected ALT source: "
              << selected_altitude_source_to_string(selected_altitude_source)
              << std::endl;
    std::cout << "selected HDG: " << selected_heading << std::endl;
    std::cout << "VNAV Mode: " << bool_value_to_string(vnav_mode) << std::endl;
    std::cout << "LNAV Mode: " << bool_value_to_string(lnav_mode) << std::endl;
    std::cout << "Autopilot: " << bool_value_to_string(autopilot) << std::endl;
    std::cout << "Approach Mode: " << bool_value_to_string(approach_mode)
              << std::endl;
    std::cout << "Altitude Hold Mode: "
              << bool_value_to_string(altitude_hold_mode) << std::endl;
    std::cout << "Barom. pressure setting: " << baro_pressure_setting
              << std::endl;
    std::cout << "TCAS operational: " << bool_value_to_string(tcas_operational)
              << std::endl;
  }
}

int decode_downlink_format(std::array<unsigned char, 14> message) {
  return (message[0] >> 3);
}

int decode_capability(std::array<unsigned char, 14> message) {
  return (message[0] & 7);
}

std::string decode_icao(std::array<unsigned char, 14> message) {
  char buf[7];
  std::snprintf(buf, sizeof(buf), "%02X%02X%02X", message[1], message[2],
                message[3]);
  std::string icao(buf);
  return icao;
}

int decode_msg_type(std::array<unsigned char, 14> message) {
  return message[4] >> 3;
}

std::string decode_callsign(std::array<unsigned char, 14> message) {
  char callsign[9];
  std::string char_map =
      "#ABCDEFGHIJKLMNOPQRSTUVWXYZ##### ###############0123456789######";
  callsign[0] = char_map[message[5] >> 2];
  callsign[1] = char_map[(message[5] & 3) << 4 | (message[6] >> 4)];
  callsign[2] = char_map[(message[6] & 15) << 2 | (message[7] >> 6)];
  callsign[3] = char_map[message[7] & 63];
  callsign[4] = char_map[message[8] >> 2];
  callsign[5] = char_map[((message[8] & 3) << 4) | (message[9] >> 4)];
  callsign[6] = char_map[(message[9] & 15) << 2 | message[10] >> 6];
  callsign[7] = char_map[message[10] & 63];
  callsign[8] = '\0';

  return std::string(callsign);
}

int decode_category(std::array<unsigned char, 14> message) {
  return message[4] & 7;
}
int decode_cpr_format(std::array<unsigned char, 14> message) {
  return (message[6] & 4) >> 2;
}

double get_lat_cpr(std::array<unsigned char, 14> message) {
  return ((message[6] & 3) << 15 | (message[7] << 7) | (message[8] >> 1)) /
         131072.0;
}

double get_lon_cpr(std::array<unsigned char, 14> message) {
  return ((message[8] & 1) << 16 | (message[9] << 8) | (message[10])) /
         131072.0;
}

int compute_NL(double lat) {
  if (lat < 0) lat = -lat;
  if (lat < 10.4704713000) return 59;
  if (lat < 14.8281743687) return 58;
  if (lat < 18.1862635707) return 57;
  if (lat < 21.0293949260) return 56;
  if (lat < 23.5450448656) return 55;
  if (lat < 25.8292470706) return 54;
  if (lat < 27.9389871012) return 53;
  if (lat < 29.9113568573) return 52;
  if (lat < 31.7720970768) return 51;
  if (lat < 33.5399343630) return 50;
  if (lat < 35.2289959780) return 49;
  if (lat < 36.8502510759) return 48;
  if (lat < 38.4124189241) return 47;
  if (lat < 39.9225668433) return 46;
  if (lat < 41.3865183226) return 45;
  if (lat < 42.8091401224) return 44;
  if (lat < 44.1945495142) return 43;
  if (lat < 45.5462672266) return 42;
  if (lat < 46.8673325250) return 41;
  if (lat < 48.1603912810) return 40;
  if (lat < 49.4277643926) return 39;
  if (lat < 50.6715016555) return 38;
  if (lat < 51.8934246917) return 37;
  if (lat < 53.0951615280) return 36;
  if (lat < 54.2781747227) return 35;
  if (lat < 55.4437844450) return 34;
  if (lat < 56.5931875621) return 33;
  if (lat < 57.7274735387) return 32;
  if (lat < 58.8476377615) return 31;
  if (lat < 59.9545927669) return 30;
  if (lat < 61.0491777425) return 29;
  if (lat < 62.1321665921) return 28;
  if (lat < 63.2042747938) return 27;
  if (lat < 64.2661652257) return 26;
  if (lat < 65.3184530968) return 25;
  if (lat < 66.3617100838) return 24;
  if (lat < 67.3964677408) return 23;
  if (lat < 68.4232202208) return 22;
  if (lat < 69.4424263114) return 21;
  if (lat < 70.4545107499) return 20;
  if (lat < 71.4598647303) return 19;
  if (lat < 72.4588454473) return 18;
  if (lat < 73.4517744167) return 17;
  if (lat < 74.4389341573) return 16;
  if (lat < 75.4205625665) return 15;
  if (lat < 76.3968439079) return 14;
  if (lat < 77.3678946133) return 13;
  if (lat < 78.3337408292) return 12;
  if (lat < 79.2942822546) return 11;
  if (lat < 80.2492321328) return 10;
  if (lat < 81.1980134927) return 9;
  if (lat < 82.1395698051) return 8;
  if (lat < 83.0719944472) return 7;
  if (lat < 83.9917356298) return 6;
  if (lat < 84.8916619070) return 5;
  if (lat < 85.7554162094) return 4;
  if (lat < 86.5353699751) return 3;
  if (lat < 87.0000000000)
    return 2;
  else
    return 1;
}

double decode_lat_abs(std::array<unsigned char, 14> even_message,
                      std::array<unsigned char, 14> odd_message) {
  double lat_cpr_even = get_lat_cpr(even_message);
  double lat_cpr_odd = get_lat_cpr(odd_message);
  int j = floor((59 * lat_cpr_even - 60 * lat_cpr_odd) + 0.5);
  double lat_even = 6 * ((j % 60) + lat_cpr_even);
  double lat_odd = 360 / 59.0 * ((j % 59) + lat_cpr_odd);

  if (compute_NL(lat_even) != compute_NL(lat_odd)) {
    return -1;
  }
  // and return lat of more recent message
  return lat_even;
}

double decode_ground_lat_abs(std::array<unsigned char, 14> even_message,
                             std::array<unsigned char, 14> odd_message) {
  double lat_cpr_even = get_lat_cpr(even_message);
  double lat_cpr_odd = get_lat_cpr(odd_message);
  int j = floor((59 * lat_cpr_even - 60 * lat_cpr_odd) + 0.5);
  double lat_even = 6 * ((j % 60) + lat_cpr_even);
  double lat_odd = 90 / 59.0 * ((j % 59) + lat_cpr_odd);

  if (compute_NL(lat_even) != compute_NL(lat_odd)) {
    return -1;
  }
  // and return lat of more recent message
  return lat_even;
}

double decode_lon_abs(std::array<unsigned char, 14> even_message,
                      std::array<unsigned char, 14> odd_message, double lat) {
  double lon_cpr_even = get_lon_cpr(even_message);
  double lon_cpr_odd = get_lon_cpr(odd_message);
  int NL_lat = compute_NL(lat);
  int m = floor(lon_cpr_even * (NL_lat - 1.0) - lon_cpr_odd * NL_lat + 0.5);
  int n_even, n_odd;
  if (NL_lat >= 1) {
    n_even = NL_lat;
  } else {
    n_even = 1;
  }
  if (NL_lat >= 0) {
    n_odd = NL_lat - 1;
  } else {
    n_odd = 1;
  }
  double lon_even = 360 / n_even * (m % n_even + lon_cpr_even);
  double lon_odd = 360 / n_odd * (m % n_odd + lon_cpr_odd);
  double lon = lon_even;
  if (lon >= 180) lon = lon - 360;
  return lon;
}

double decode_ground_lon_abs(std::array<unsigned char, 14> even_message,
                             std::array<unsigned char, 14> odd_message,
                             double lat) {
  double lon_cpr_even = get_lon_cpr(even_message);
  double lon_cpr_odd = get_lon_cpr(odd_message);
  int NL_lat = compute_NL(lat);
  int m = floor(lon_cpr_even * (NL_lat - 1.0) - lon_cpr_odd * NL_lat + 0.5);
  int n_even, n_odd;
  if (NL_lat >= 1) {
    n_even = NL_lat;
  } else {
    n_even = 1;
  }
  if (NL_lat >= 0) {
    n_odd = NL_lat - 1;
  } else {
    n_odd = 1;
  }
  double lon_even = 90 / n_even * (m % n_even + lon_cpr_even);
  double lon_odd = 90 / n_odd * (m % n_odd + lon_cpr_odd);
  double lon = lon_even;
  if (lon >= 180) lon = lon - 360;
  return lon;
}

int decode_barometric_altitude(std::array<unsigned char, 14> message,
                               AltitudeType& altitude_type) {
  int qbit = message[5] & 1;
  int n = (message[5] & 254) << 3 | message[6] >> 4;
  if (qbit) {
    altitude_type = BAROMETRIC_ALT;
    return 25 * n - 1000;
  } else {
    altitude_type = UNDETERMINED_ALT;
    // TODO: implement Gray code (used for altitudes > 50175 ft)
    return 0;
  }
}

int decode_gnss_altitude(std::array<unsigned char, 14> message,
                         AltitudeType& altitude_type) {
  int mbit = message[3] & (1 << 6);
  int qbit = message[3] & (1 << 4);
  if (!mbit && qbit) {
    int n = ((message[2] & 31) << 6) | ((message[3] & 0x80) >> 2) |
            ((message[3] & 0x20) >> 1) | (message[3] & 15);
    altitude_type = GNSS_ALT;
    // convert to feet
    return round((25 * n - 1000) * 3.28084);
  } else {
    altitude_type = UNDETERMINED_ALT;
    return 0;
  }
}

int decode_tc19_subtype(std::array<unsigned char, 14> message) {
  return message[4] & 3;
}

BoolValue decode_intent_change_flag(std::array<unsigned char, 14> message) {
  return (message[5] >> 7) ? TRUE : FALSE;
}

BoolValue decode_ifr_capability_flag(std::array<unsigned char, 14> message) {
  return ((message[5] >> 6) & 1) ? TRUE : FALSE;
}

int decode_nav_uncertainty_category(std::array<unsigned char, 14> message) {
  return (message[5] >> 3) & 7;
}

double decode_magnetic_heading(std::array<unsigned char, 14> message) {
  if (message[5] & (1 << 2)) {
    int n = ((message[5] & 3) << 8) | message[6];
    return n * 360.0 / 1024.0;
  } else {
    return -1;
  }
}

SpeedType decode_speed_type(std::array<unsigned char, 14> message) {
  int tc19_subtype = decode_tc19_subtype(message);
  if (tc19_subtype == 1 || tc19_subtype == 2) {
    return GROUND_SPEED;
  } else if (tc19_subtype == 3 || tc19_subtype == 4) {
    int subtype34_airspeed_type = message[7] >> 7;
    if (subtype34_airspeed_type) {
      return TRUE_AIRSPEED;
    } else {
      return INDICATED_AIRSPEED;
    }
  } else {
    return UNDETERMINED_SPEED;
  }
}

double decode_airspeed(std::array<unsigned char, 14> message,
                       SpeedType& speed_type) {
  int n = (message[7] & 127) << 3 | message[8] >> 5;

  if (n == 0) {
    speed_type = UNDETERMINED_SPEED;
    return 0;
  }

  int subtype34_airspeed_type = message[7] >> 7;
  if (subtype34_airspeed_type) {
    speed_type = TRUE_AIRSPEED;
  } else {
    speed_type = INDICATED_AIRSPEED;
  }

  if (decode_tc19_subtype(message) == 3) {
    return n - 1.0;
  } else if (decode_tc19_subtype(message) == 4) {
    return 4.0 * (n - 1.0);
  } else {
    throw std::invalid_argument("Message must have either subtype 3 or 4.");
  }
}

VerticalRateSource decode_vertical_rate_source(
    std::array<unsigned char, 14> message) {
  if ((message[8] >> 4) & 1) {
    return BAROMETER;
  } else {
    return GNSS;
  }
}

int decode_vertical_rate(std::array<unsigned char, 14> message,
                         FieldStatus& vertical_rate_status) {
  int n = (message[8] & 7) << 6 | message[9] >> 2;
  if (n == 0) {
    vertical_rate_status = UNDETERMINED;
    return 0;
  }
  vertical_rate_status = KNOWN;
  if ((message[8] >> 3) & 1) {
    return (-64) * (n - 1);
  } else {
    return 64 * (n - 1);
    ;
  }
}

int decode_altitude_delta(std::array<unsigned char, 14> message,
                          FieldStatus& altitude_delta_status) {
  int n = (message[10] & 127);
  if (n == 0) {
    altitude_delta_status = UNDETERMINED;
    return 0;
  }
  altitude_delta_status = KNOWN;
  if (message[10] >> 7) {
    return (-25) * (n - 1);
  } else {
    return 25 * (n - 1);
    ;
  }
}

double decode_ground_speed(std::array<unsigned char, 14> message,
                           SpeedType& speed_type) {
  int sign_ew = (message[5] & 4) >> 2;
  int n_ew = (message[5] & 3) << 8 | message[6];
  int v_ew;
  if (sign_ew) {
    v_ew = (-1) * (n_ew - 1);
  } else {
    v_ew = (n_ew - 1);
  }

  int sign_ns = (message[7] >> 7);
  int n_ns = (message[7] & 127) << 3 | message[8] >> 5;
  int v_ns;
  if (sign_ns) {
    v_ns = (-1) * (n_ns - 1);
  } else {
    v_ns = (n_ns - 1);
  }
  if (n_ew == 0 || n_ns == 0) {
    speed_type = UNDETERMINED_SPEED;
  }
  speed_type = GROUND_SPEED;
  return sqrt(v_ns * v_ns + v_ew * v_ew);
}

double decode_track_angle(std::array<unsigned char, 14> message) {
  int sign_ew = (message[5] & 4) >> 2;
  int n_ew = (message[5] & 3) << 8 | message[6];
  int v_ew;
  if (sign_ew) {
    v_ew = (-1) * (n_ew - 1);
  } else {
    v_ew = (n_ew - 1);
  }

  int sign_ns = (message[7] >> 7);
  int n_ns = (message[7] & 127) << 3 | message[8] >> 5;
  int v_ns;
  if (sign_ns) {
    v_ns = (-1) * (n_ns - 1);
  } else {
    v_ns = (n_ns - 1);
  }
  double track =
      std::fmod(atan2(v_ew, v_ns) * 360.0 / (2.0 * M_PI) + 360.0, 360.0);
  return track;
}

double decode_ground_movement(std::array<unsigned char, 14> message,
                              SpeedType& speed_type) {
  int encoded_speed = ((message[4] & 7) << 4) | (message[5] >> 4);
  if (encoded_speed == 0) {
    speed_type = UNDETERMINED_SPEED;
    return -1;
  }
  speed_type = GROUND_MOVEMENT;
  if (encoded_speed == 1) {
    return 0.0;
  }
  if (encoded_speed <= 8) {
    return 0.125 + (encoded_speed - 2) * 0.125;
  }
  if (encoded_speed <= 12) {
    return 1 + 0.25 * (encoded_speed - 9);
  }
  if (encoded_speed <= 38) {
    return 2 + 0.5 * (encoded_speed - 13);
  }
  if (encoded_speed <= 93) {
    return 15 + (encoded_speed - 39);
  }
  if (encoded_speed <= 108) {
    return 70 + (encoded_speed - 94) * 2;
  }
  if (encoded_speed <= 123) {
    return 100 + (encoded_speed - 109) * 5;
  }
  if (encoded_speed == 124) {
    return 175;
  }
  speed_type = UNDETERMINED_SPEED;
  return -1;
}

int decode_ground_track_status(std::array<unsigned char, 14> message) {
  return ((message[5] >> 3) & 1);
}

double decode_ground_track(std::array<unsigned char, 14> message,
                           HeadingType& heading_type) {
  if (decode_ground_track_status(message)) {
    heading_type = GROUND_HEADING;
    int n = ((message[5] & 7) << 4) | (message[6] >> 4);
    return 360.0 * n / 128.0;
  } else {
    heading_type = UNDETERMINED_HEADING;
    return 0;
  }
}

SelectedAltitudeSource decode_selected_altitude_source(
    std::array<unsigned char, 14> message) {
  if (message[5] >> 7) {
    return FMS;
  } else {
    return MCPFCU;
  }
}

BoolValue decode_autopilot(std::array<unsigned char, 14> message) {
  if ((message[9] >> 1 & 1) == 0) {
    return UNDETERMINED_BOOL;
  }
  return (message[9] & 1) ? TRUE : FALSE;
}

BoolValue decode_lnav_mode(std::array<unsigned char, 14> message) {
  if (!(message[9] >> 1 & 1)) {
    return UNDETERMINED_BOOL;
  }
  return (message[10] & 32) ? TRUE : FALSE;
}

BoolValue decode_vnav_mode(std::array<unsigned char, 14> message) {
  if (!(message[9] >> 1 & 1)) {
    return UNDETERMINED_BOOL;
  }
  return (message[10] >> 7) ? TRUE : FALSE;
}
BoolValue decode_approach_mode(std::array<unsigned char, 14> message) {
  if (!(message[9] >> 1 & 1)) {
    return UNDETERMINED_BOOL;
  }
  return ((message[10] >> 4) & 1) ? TRUE : FALSE;
}
BoolValue decode_altitude_hold_mode(std::array<unsigned char, 14> message) {
  if (!(message[9] >> 1 & 1)) {
    return UNDETERMINED_BOOL;
  }
  return ((message[10] >> 6) & 1) ? TRUE : FALSE;
}
BoolValue decode_tcas_operational(std::array<unsigned char, 14> message) {
  int subtype = (message[4] >> 1) & 3;
  if (subtype == 0) {
    return (!((message[10] >> 4) & 1)) ? TRUE : FALSE;
  } else {
    return ((message[10] >> 3) & 1) ? TRUE : FALSE;
  }
}

double decode_baro_pressure_setting(std::array<unsigned char, 14> message) {
  int pressure = ((message[6] & 15) << 5) | (message[7] >> 3);
  return 800 + (pressure - 1) * 0.8;
}

int decode_selected_altitude(std::array<unsigned char, 14> message) {
  int altitude = (message[5] & 127) << 4 | (message[6] >> 4);
  return (altitude - 1) * 32;
}

double decode_selected_heading(std::array<unsigned char, 14> message,
                               FieldStatus& selected_heading_status) {
  if (((message[7] >> 2) & 1) == 0) {
    selected_heading_status = UNDETERMINED;
    return 0.0;
  }
  int heading = ((message[7] & 3) << 7) | (message[8] >> 1);
  selected_heading_status = KNOWN;
  return (heading * (180.0 / 256.0));
}

std::string short_aircraft_category(int type_code, int category) {
  std::string detailed_category;
  if (type_code == 1) {
    detailed_category = "RSVD";
  }
  if (category == 0) {
    detailed_category = "    ";
  }
  if (type_code == 2) {
    if (category == 1) {
      detailed_category = "SEMR";
    } else if (category == 2) {
      detailed_category = "SSRV";
    } else if (category >= 3 && category <= 7) {
      detailed_category = "GOBS";
    }
  } else if (type_code == 3) {
    if (category == 1) {
      detailed_category = "GLDR";
    } else if (category == 2) {
      detailed_category = "LGTA";
    } else if (category == 3) {
      detailed_category = "PCHT";
    } else if (category == 4) {
      detailed_category = "ULGT";
    } else if (category == 5) {
      detailed_category = "RESD";
    } else if (category == 6) {
      detailed_category = "UNMD";
    } else if (category == 7) {
      detailed_category = "SPCV";
    }
  } else if (type_code == 4) {
    if (category == 1) {
      detailed_category = "LGHT";
    } else if (category == 2) {
      detailed_category = "MED1";
    } else if (category == 3) {
      detailed_category = "MED2";
    } else if (category == 4) {
      detailed_category = "HVTX";
    } else if (category == 5) {
      detailed_category = "HEVY";
    } else if (category == 6) {
      detailed_category = "HPRF";
    } else if (category == 7) {
      detailed_category = "ROTR";
    }
  }

  return detailed_category;
}
std::string detailed_aircraft_category(int type_code, int category) {
  std::string detailed_category;
  if (type_code == 1) {
    detailed_category = "reserved";
  }
  if (category == 0) {
    detailed_category = "No category information";
  }
  if (type_code == 2) {
    if (category == 1) {
      detailed_category = "Surface emergency vehicle";
    } else if (category == 2) {
      detailed_category = "Surface service vehicle";
    } else if (category >= 3 && category <= 7) {
      detailed_category = "Ground obstruction";
    }
  } else if (type_code == 3) {
    if (category == 1) {
      detailed_category = "Glider / sailplane";
    } else if (category == 2) {
      detailed_category = "Lighter-than-air";
    } else if (category == 3) {
      detailed_category = "Parachutist / skydiver";
    } else if (category == 4) {
      detailed_category = "Ultralight, hang-glider, paraglider";
    } else if (category == 5) {
      detailed_category = "Reserved";
    } else if (category == 6) {
      detailed_category = "Unmanned aerial vehicle";
    } else if (category == 7) {
      detailed_category = "Space or transatmospheric vehicle";
    }
  } else if (type_code == 4) {
    if (category == 1) {
      detailed_category = "Light (< 7000kg)";
    } else if (category == 2) {
      detailed_category = "Medium 1 (7000-34000kg)";
    } else if (category == 3) {
      detailed_category = "Medium 2 (34000-136000kg)";
    } else if (category == 4) {
      detailed_category = "High vortex aircraft";
    } else if (category == 5) {
      detailed_category = "Heavy (>136000kg)";
    } else if (category == 6) {
      detailed_category = "High performance (>5g and >400kt)";
    } else if (category == 7) {
      detailed_category = "Rotorcraft";
    }
  }

  return detailed_category;
}

std::string detailed_type_code(int type_code) {
  std::string detailed_type_code = "";
  if (type_code >= 1 && type_code <= 4) {
    detailed_type_code = "Aircraft identification";
  } else if (type_code >= 5 && type_code <= 8) {
    detailed_type_code = "Surface position";
  } else if (type_code >= 9 && type_code <= 18) {
    detailed_type_code = "Airborne position w/ barom. altitude";
  } else if (type_code == 19) {
    detailed_type_code = "Airborne velocity";
  } else if (type_code >= 20 && type_code <= 22) {
    detailed_type_code = "Airborne position w/ GNSS height";
  } else if (type_code >= 23 && type_code <= 27) {
    detailed_type_code = "Reserved";
  } else if (type_code == 28) {
    detailed_type_code = "Aircraft status";
  } else if (type_code == 29) {
    detailed_type_code = "Target states and status";
  } else if (type_code == 31) {
    detailed_type_code = "Operational status";
  }
  return detailed_type_code;
}

std::string heading_type_value_to_string(HeadingType value) {
  switch (value) {
    case TRACK_ANGLE:
      return "TRACK_ANGLE";
    case MAGNETIC:
      return "MAGNETIC";
    case GROUND_HEADING:
      return "GROUND_HEADING";
    case UNDETERMINED_HEADING:
      return "UNDETERMINED";
    default:
      return "UNDETERMINED";
  }
}

std::string speed_type_value_to_string(SpeedType value) {
  switch (value) {
    case INDICATED_AIRSPEED:
      return "INDICATED_AIRSPEED";
    case TRUE_AIRSPEED:
      return "TRUE_AIRSPEED";
    case GROUND_SPEED:
      return "GROUND_SPEED";
    case GROUND_MOVEMENT:
      return "GROUND_MOVEMENT";
    case UNDETERMINED_SPEED:
      return "UNDETERMINED";
    default:
      return "UNDETERMINED";
  }
}

std::string vertical_rate_source_to_string(VerticalRateSource value) {
  switch (value) {
    case GNSS:
      return "GNSS";
    case BAROMETER:
      return "BAROMETER";
    case UNDETERMINED_VR_SOURCE:
      return "UNDETERMINED";
    default:
      return "UNDETERMINED";
  }
}

std::string field_status_to_string(FieldStatus value) {
  switch (value) {
    case KNOWN:
      return "KNOWN";
    case UNDETERMINED:
      return "UNDETERMINED";
    default:
      return "UNDETERMINED";
  }
}

std::string selected_altitude_source_to_string(SelectedAltitudeSource value) {
  switch (value) {
    case FMS:
      return "FMS";
    case MCPFCU:
      return "MCPFCU";
    case UNDETERMINED_SEL_ALT_SOURCE:
      return "UNDETERMINED";
    default:
      return "UNDETERMINED";
  }
}

std::string altitude_type_to_string(AltitudeType value) {
  switch (value) {
    case GNSS_ALT:
      return "GNSS";
    case BAROMETRIC_ALT:
      return "BAROMETRIC";
    case UNDETERMINED_ALT:
      return "UNDETERMINED";
    default:
      return "UNDETERMINED";
  }
}

std::string bool_value_to_string(BoolValue value) {
  switch (value) {
    case UNDETERMINED_BOOL:
      return "NA";
    case TRUE:
      return "true";
    case FALSE:
      return "false";
    default:
      return "NA";
  }
}
