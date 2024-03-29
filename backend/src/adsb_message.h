#ifndef ADSBOOST_ADSB_MESSAGE_H_
#define ADSBOOST_ADSB_MESSAGE_H_

#include <array>
#include <chrono>
#include <string>

enum HeadingType {
  TRACK_ANGLE,
  MAGNETIC,
  GROUND_HEADING,
  UNDETERMINED_HEADING
};

enum AltitudeType { GNSS_ALT, BAROMETRIC_ALT, UNDETERMINED_ALT };

enum SpeedType {
  INDICATED_AIRSPEED,
  TRUE_AIRSPEED,
  GROUND_SPEED,
  GROUND_MOVEMENT,
  UNDETERMINED_SPEED
};

enum BoolValue { TRUE, FALSE, UNDETERMINED_BOOL };

enum VerticalRateSource { GNSS, BAROMETER, UNDETERMINED_VR_SOURCE };

enum FieldStatus { KNOWN, UNDETERMINED };

enum SelectedAltitudeSource { FMS, MCPFCU, UNDETERMINED_SEL_ALT_SOURCE };

int decode_downlink_format(std::array<unsigned char, 14> message);
int decode_capability(std::array<unsigned char, 14> message);
std::string decode_icao(std::array<unsigned char, 14> message);
int decode_msg_type(std::array<unsigned char, 14> message);
std::string decode_callsign(std::array<unsigned char, 14> message);
int decode_category(std::array<unsigned char, 14> message);

int decode_cpr_format(std::array<unsigned char, 14> message);
double get_lat_cpr(std::array<unsigned char, 14> message);
double get_lon_cpr(std::array<unsigned char, 14> message);

int compute_NL(double lat);
double decode_lat_abs(std::array<unsigned char, 14> even_message,
                      std::array<unsigned char, 14> odd_message);
double decode_lon_abs(std::array<unsigned char, 14> even_message,
                      std::array<unsigned char, 14> odd_message, double lat);
int decode_barometric_altitude(std::array<unsigned char, 14> message,
                               AltitudeType& altitude_type);
int decode_gnss_altitude(std::array<unsigned char, 14> message,
                         AltitudeType& altitude_type);
int decode_tc19_subtype(std::array<unsigned char, 14> message);
BoolValue decode_intent_change_flag(std::array<unsigned char, 14> message);
BoolValue decode_ifr_capability_flag(std::array<unsigned char, 14> message);
int decode_nav_uncertainty_category(std::array<unsigned char, 14> message);
double decode_magnetic_heading(std::array<unsigned char, 14> message);
SpeedType decode_speed_type(std::array<unsigned char, 14> message);
double decode_airspeed(std::array<unsigned char, 14> message,
                       SpeedType& speed_type);
VerticalRateSource decode_vertical_rate_source(
    std::array<unsigned char, 14> message);
int decode_vertical_rate(std::array<unsigned char, 14> message,
                         FieldStatus& vertical_rate_status);
int decode_altitude_delta(std::array<unsigned char, 14> message,
                          FieldStatus& altitude_delta_status);
double decode_ground_speed(std::array<unsigned char, 14> message,
                           SpeedType& speed_type);
double decode_track_angle(std::array<unsigned char, 14> message);
double decode_ground_movement(std::array<unsigned char, 14> message,
                              SpeedType& speed_type);
int decode_ground_track_status(std::array<unsigned char, 14> message);
double decode_ground_track(std::array<unsigned char, 14> message,
                           HeadingType& heading_type);
BoolValue decode_autopilot(std::array<unsigned char, 14> message);
BoolValue decode_lnav_mode(std::array<unsigned char, 14> message);
BoolValue decode_vnav_mode(std::array<unsigned char, 14> message);
BoolValue decode_approach_mode(std::array<unsigned char, 14> message);
BoolValue decode_altitude_hold_mode(std::array<unsigned char, 14> message);
BoolValue decode_tcas_operational(std::array<unsigned char, 14> message);
double decode_baro_pressure_setting(std::array<unsigned char, 14> message);
int decode_selected_altitude(std::array<unsigned char, 14> message);
SelectedAltitudeSource decode_selected_altitude_source(
    std::array<unsigned char, 14> message);
double decode_selected_heading(std::array<unsigned char, 14> message,
                               FieldStatus& selected_heading_status);
std::string detailed_aircraft_category(int type_code, int category);
std::string short_aircraft_category(int type_code, int category);
std::string detailed_type_code(int type_code);

class ADSBMessage {
 public:
  std::array<unsigned char, 14> message;
  int downlink_format = -1;
  int downlink_capability = -1;
  std::string icao = "";
  int type_code = -1;
  int tc19_subtype = -1;
  std::string callsign = "";
  int aircraft_category = -1;

  SpeedType speed_type = UNDETERMINED_SPEED;
  double speed = 0.0;

  HeadingType heading_type = UNDETERMINED_HEADING;
  double heading = 0.0;

  AltitudeType altitude_type = UNDETERMINED_ALT;
  int altitude = 0;

  VerticalRateSource vertical_rate_source = UNDETERMINED_VR_SOURCE;
  FieldStatus vertical_rate_status = UNDETERMINED;
  int vertical_rate = 0;
  FieldStatus altitude_delta_status = UNDETERMINED;
  int altitude_delta = 0;

  BoolValue autopilot = UNDETERMINED_BOOL;
  BoolValue lnav_mode = UNDETERMINED_BOOL;
  BoolValue vnav_mode = UNDETERMINED_BOOL;
  BoolValue approach_mode = UNDETERMINED_BOOL;
  BoolValue tcas_operational = UNDETERMINED_BOOL;
  BoolValue altitude_hold_mode = UNDETERMINED_BOOL;

  BoolValue intent_change_flag = UNDETERMINED_BOOL;
  BoolValue ifr_capability_flag = UNDETERMINED_BOOL;

  FieldStatus nav_uncertainty_category_status = UNDETERMINED;
  int nav_uncertainty_category = -1;

  FieldStatus cpr_status = UNDETERMINED;
  double lat_cpr = 0.0;
  double lon_cpr = 0.0;
  int cpr_format = -1;

  SelectedAltitudeSource selected_altitude_source = UNDETERMINED_SEL_ALT_SOURCE;
  FieldStatus selected_altitude_status = UNDETERMINED;
  int selected_altitude = 0;

  FieldStatus selected_heading_status = UNDETERMINED;
  double selected_heading = 0.0;

  FieldStatus baro_pressure_setting_status = UNDETERMINED;
  double baro_pressure_setting = -1;

  std::chrono::system_clock::time_point timestamp;

  void init(std::array<unsigned char, 14> message,
            std::chrono::system_clock::time_point timestamp);
  ADSBMessage(std::array<unsigned char, 14> message);
  ADSBMessage(std::array<unsigned char, 14> message,
              std::chrono::system_clock::time_point timestamp);
  void PrintMessage();
  std::string HexString();
};

std::string heading_type_value_to_string(HeadingType value);
std::string speed_type_value_to_string(SpeedType value);
std::string vertical_rate_source_to_string(VerticalRateSource value);
std::string field_status_to_string(FieldStatus value);
std::string selected_altitude_source_to_string(SelectedAltitudeSource value);
std::string altitude_type_to_string(AltitudeType value);
std::string bool_value_to_string(BoolValue value);

#endif  // ADSBOOST_ADSB_MESSAGE_H_