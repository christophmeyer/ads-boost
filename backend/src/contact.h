#ifndef ADSBOOST_CONTACT_H_
#define ADSBOOST_CONTACT_H_

#include <condition_variable>
#include <list>
#include <mutex>

#include "adsb_message.h"

class Contact {
 public:
  std::string icao = "";
  std::string callsign = "";
  std::string aircraft_category = "";

  SpeedType speed_type = UNDETERMINED_SPEED;
  double speed = 0.0;

  HeadingType heading_type = UNDETERMINED_HEADING;
  double heading = 0;

  AltitudeType altitude_type = UNDETERMINED_ALT;
  int altitude = 0;

  VerticalRateSource vertical_rate_source = UNDETERMINED_VR_SOURCE;
  FieldStatus vertical_rate_status = UNDETERMINED;
  int vertical_rate = 0;
  FieldStatus altitude_delta_status = UNDETERMINED;
  int altitude_delta = 0;

  FieldStatus position_status = UNDETERMINED;
  double lat = 0;
  double lon = 0;

  FieldStatus position_ref_status = UNDETERMINED;
  double lat_ref = 0;
  double lon_ref = 0;

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

  SelectedAltitudeSource selected_altitude_source = UNDETERMINED_SEL_ALT_SOURCE;
  FieldStatus selected_altitude_status = UNDETERMINED;
  int selected_altitude = 0;

  FieldStatus selected_heading_status = UNDETERMINED;
  double selected_heading = 0.0;

  FieldStatus baro_pressure_setting_status = UNDETERMINED;
  int baro_pressure_setting = -1;

  // stats
  int n_messages = 0;
  std::chrono::system_clock::time_point first_message;
  std::chrono::system_clock::time_point last_message;

  Contact(ADSBMessage message);
  Contact(ADSBMessage message, double lat_ref, double lon_ref);
  void update(ADSBMessage message);
  std::string to_json();
  int last_seen();

 private:
  void update_position(ADSBMessage message);
  int max_cpr_delay_s = 10;
  double even_lat_cpr;
  double even_lon_cpr;
  int even_tc;
  std::chrono::system_clock::time_point even_timestamp;

  double odd_lat_cpr;
  double odd_lon_cpr;
  int odd_tc;
  std::chrono::system_clock::time_point odd_timestamp;
};

class ContactList {
 public:
  int timeout = 90;
  double lon_ref;
  double lat_ref;
  FieldStatus position_ref_status = UNDETERMINED;
  std::list<Contact> contacts = {};
  ContactList(int timeout);
  ContactList(int timeout, double lat_ref, double lon_ref);
  void update(ADSBMessage message);
  std::string to_json();
  Contact* get_contact(std::string icao);
  std::list<Contact>* get_contacts();
  static bool timed_out(Contact&);
};

struct SharedContactList {
  ContactList contact_list = ContactList(10);
  std::mutex mutex;
  std::condition_variable contacts_ready;
  bool contacts_updated = false;
};

int pos_mod(int m, int n);

#endif  // ADSBOOST_CONTACT_H_