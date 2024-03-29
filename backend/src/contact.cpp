#include "contact.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

ContactList::ContactList(int timeout) { this->timeout = timeout; };
ContactList::ContactList(int timeout, double lat_ref, double lon_ref) {
  this->timeout = timeout;
  this->lon_ref = lon_ref;
  this->lat_ref = lat_ref;
  this->position_ref_status = KNOWN;
  this->contacts = {};
};

std::list<Contact>* ContactList::get_contacts() {
  this->contacts.remove_if(
      [this](Contact contact) { return contact.last_seen() >= this->timeout; });
  return &contacts;
}

void ContactList::update(ADSBMessage message) {
  Contact* existing_contact = this->get_contact(message.icao);
  if (existing_contact != nullptr) {
    existing_contact->update(message);
  } else {
    Contact* contact;
    if (this->position_ref_status == KNOWN) {
      contact = new Contact(message, this->lat_ref, this->lon_ref);
    } else {
      contact = new Contact(message);
    }
    this->contacts.push_front(*contact);
  }
}

Contact* ContactList::get_contact(std::string icao) {
  for (Contact& contact : this->contacts) {
    if (contact.icao.compare(icao) == 0) {
      return &contact;
    }
  }
  return nullptr;
}

std::string ContactList::to_json() {
  std::stringstream ss;

  ss << "{\"contacts\": [";

  if (this->contacts.size() > 0) {
    for (std::list<Contact>::iterator it = this->contacts.begin();
         it != this->contacts.end(); ++it) {
      ss << it->to_json();
      if (it != std::prev(contacts.end())) {
        ss << ",";
      }
    }
  }
  ss << "]}";
  return ss.str();
}

Contact::Contact(ADSBMessage message, double lat_ref, double lon_ref) {
  icao = message.icao;
  first_message = message.timestamp;
  this->lat_ref = lat_ref;
  this->lon_ref = lon_ref;
  this->position_ref_status = KNOWN;
  this->update(message);
}

Contact::Contact(ADSBMessage message) {
  icao = message.icao;
  first_message = message.timestamp;
  this->update(message);
}

void Contact::update(ADSBMessage message) {
  // return if icao does not match
  if (icao.compare(message.icao) != 0) {
    return;
  }

  if (message.type_code >= 1 && message.type_code <= 4) {
    // aircraft identification
    callsign = message.callsign;
    this->aircraft_category =
        short_aircraft_category(message.type_code, message.aircraft_category);
  } else if (message.type_code >= 5 && message.type_code <= 8) {
    // surface position
    if (position_ref_status == KNOWN) this->update_position(message);

    speed = message.speed;
    speed_type = message.speed_type;
    heading = message.heading;
    heading_type = message.heading_type;

  } else if (message.type_code >= 9 && message.type_code <= 18) {
    this->update_position(message);
    altitude = message.altitude;  // In feet
    altitude_type = message.altitude_type;

  } else if (message.type_code == 19) {
    // airborne velocities

    vertical_rate_source = message.vertical_rate_source;
    vertical_rate = message.vertical_rate;
    vertical_rate_status = message.vertical_rate_status;
    altitude_delta = message.altitude_delta;
    altitude_delta_status = message.altitude_delta_status;

    speed_type = message.speed_type;
    speed = message.speed;
    heading = message.heading;
    heading_type = message.heading_type;

    intent_change_flag = message.intent_change_flag;
    ifr_capability_flag = message.ifr_capability_flag;
    nav_uncertainty_category = message.nav_uncertainty_category;
    nav_uncertainty_category_status = message.nav_uncertainty_category_status;

  } else if (message.type_code >= 20 && message.type_code <= 22) {
    this->update_position(message);
    altitude = message.altitude;
    altitude_type = message.altitude_type;
  } else if (message.type_code == 29) {
    autopilot = message.autopilot;
    approach_mode = message.approach_mode;
    altitude_hold_mode = message.altitude_hold_mode;
    selected_heading = message.selected_heading;
    selected_heading_status = message.selected_heading_status;
    selected_altitude = message.selected_altitude;
    selected_altitude_status = message.selected_altitude_status;
    selected_altitude_source = message.selected_altitude_source;
    baro_pressure_setting = message.baro_pressure_setting;
    lnav_mode = message.lnav_mode;
    vnav_mode = message.vnav_mode;
    tcas_operational = message.tcas_operational;
  }

  // update stats
  n_messages++;
  last_message = message.timestamp;
}

int Contact::last_seen() {
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(now - last_message)
      .count();
}

void Contact::update_position(ADSBMessage message) {
  if (message.cpr_format == 0) {
    even_lat_cpr = message.lat_cpr;
    even_lon_cpr = message.lon_cpr;
    even_timestamp = message.timestamp;
    even_tc = message.type_code;
  } else if (message.cpr_format == 1) {
    odd_lat_cpr = message.lat_cpr;
    odd_lon_cpr = message.lon_cpr;
    odd_timestamp = message.timestamp;
    odd_tc = message.type_code;
  }

  // Do not update position if current state mixes ground and airborne position
  // tc 5-8 ground position
  // tc 9-18, 20-22 airborne position
  if ((odd_tc < 9 and even_tc >= 9) || (odd_tc >= 9 && even_tc < 9)) {
    return;
  }

  // Do not update position if messages are longer than max_cpr_delay_s apart
  auto abs_delay = std::abs(std::chrono::duration_cast<std::chrono::seconds>(
                                even_timestamp - odd_timestamp)
                                .count());
  if (abs_delay > max_cpr_delay_s) {
    return;
  }

  // calculate lat
  int j = std::floor((59.0 * even_lat_cpr - 60 * odd_lat_cpr) + 0.5);
  double lat_even = 6.0 * (pos_mod(j, 60) + even_lat_cpr);
  double lat_odd = 360.0 / 59.0 * (pos_mod(j, 59) + odd_lat_cpr);
  if (odd_tc < 9.0) {
    lat_even = (lat_ref > 0) ? lat_even / 4.0 : lat_even / 4.0 - 90.0;
    lat_odd = (lat_ref > 0) ? lat_odd / 4.0 : lat_odd / 4.0 - 90.0;
  } else {
    if (lat_even >= 270.0) lat_even -= 360.0;
    if (lat_odd >= 270.0) lat_odd -= 360.0;
  }

  if (compute_NL(lat_even) == compute_NL(lat_odd)) {
    if (even_timestamp > odd_timestamp) {
      lat = lat_even;
    } else {
      lat = lat_odd;
    }
  } else {
    return;
  }
  position_status = KNOWN;
  // calculate lon
  int NL_lat = compute_NL(lat);
  int NL_lat_odd = compute_NL(lat) - 1;

  int m =
      std::floor(even_lon_cpr * (NL_lat - 1.0) - odd_lon_cpr * NL_lat + 0.5);
  int n_even, n_odd;
  if (NL_lat >= 1.0) {
    n_even = NL_lat;
  } else {
    n_even = 1.0;
  }
  if (NL_lat_odd >= 1.0) {
    n_odd = NL_lat_odd;
  } else {
    n_odd = 1.0;
  }

  double lon_even = 360.0 / n_even * (pos_mod(m, n_even) + even_lon_cpr);
  double lon_odd = 360.0 / n_odd * (pos_mod(m, n_odd) + odd_lon_cpr);
  if (odd_tc < 9.0) {
    lon_even = lon_even / 4.0;
    lon_odd = lon_odd / 4.0;
  }
  if (even_timestamp > odd_timestamp) {
    lon = lon_even;
  } else {
    lon = lon_odd;
  }
  if (odd_tc < 9.0) {
    std::array<double, 4> lons = {lon, lon + 90, lon + 180, lon + 270};
    for (double& val : lons) {
      val = std::fmod(val + 180, 360) - 180;
    }
    double min_diff = std::abs(lon_ref - lons[0]);
    size_t imin = 0;
    for (size_t i = 1; i < lons.size(); ++i) {
      double diff = std::abs(lon_ref - lons[i]);
      if (diff < min_diff) {
        min_diff = diff;
        imin = i;
      }
    }
    lon = lons[imin];
  } else {
    if (lon >= 180) lon = lon - 360;
  }
}

int pos_mod(int m, int n) {
  int mod = m % n;
  if (mod < 0) mod += n;
  return mod;
}

std::string Contact::to_json() {
  std::stringstream ss;
  ss << "{";
  ss << "\"icao\": \"" << icao << "\",";
  ss << "\"callsign\": \"" << callsign << "\",";
  ss << "\"aircraft_category\": \"" << aircraft_category << "\",";
  ss << "\"speed_type\": \"" << speed_type_value_to_string(speed_type) << "\",";
  ss << "\"speed\": \"" << speed << "\",";
  ss << "\"heading_type\": \"" << heading_type_value_to_string(heading_type)
     << "\",";
  ss << "\"heading\": " << heading << ",";
  ss << "\"altitude_type\": \"" << altitude_type_to_string(altitude_type)
     << "\",";
  ss << "\"altitude\": " << altitude << ",";
  ss << "\"vertical_rate_source\": \""
     << vertical_rate_source_to_string(vertical_rate_source) << "\",";
  ss << "\"vertical_rate_status\": \""
     << field_status_to_string(vertical_rate_status) << "\",";
  ss << "\"vertical_rate\": " << vertical_rate << ",";
  ss << "\"altitude_delta\": " << altitude_delta << ",";
  ss << "\"altitude_delta_status\": \""
     << field_status_to_string(altitude_delta_status) << "\",";
  ss << "\"position_status\": \"" << field_status_to_string(position_status)
     << "\",";
  ss << "\"lat\": " << lat << ",";
  ss << "\"lon\": " << lon << ",";
  ss << "\"position_ref_status\": \""
     << field_status_to_string(position_ref_status) << "\",";
  ss << "\"lat_ref\": " << lat_ref << ",";
  ss << "\"lon_ref\": " << lon_ref << ",";
  ss << "\"autopilot\": \"" << bool_value_to_string(autopilot) << "\",";
  ss << "\"lnav_mode\": \"" << bool_value_to_string(lnav_mode) << "\",";
  ss << "\"vnav_mode\": \"" << bool_value_to_string(vnav_mode) << "\",";
  ss << "\"approach_mode\": \"" << bool_value_to_string(approach_mode) << "\",";
  ss << "\"tcas_operational\": \"" << bool_value_to_string(tcas_operational)
     << "\",";
  ss << "\"altitude_hold_mode\": \"" << bool_value_to_string(altitude_hold_mode)
     << "\",";
  ss << "\"intent_change_flag\": \"" << bool_value_to_string(intent_change_flag)
     << "\",";
  ss << "\"ifr_capability_flag\": \""
     << bool_value_to_string(ifr_capability_flag) << "\",";
  ss << "\"nav_uncertainty_category_status\": \""
     << field_status_to_string(nav_uncertainty_category_status) << "\",";
  ss << "\"nav_uncertainty_category\": " << nav_uncertainty_category << ",";
  ss << "\"selected_altitude_source\": \""
     << selected_altitude_source_to_string(selected_altitude_source) << "\",";
  ss << "\"selected_altitude_status\": \""
     << field_status_to_string(selected_altitude_status) << "\",";
  ss << "\"selected_altitude\": " << selected_altitude << ",";
  ss << "\"selected_heading_status\": \""
     << field_status_to_string(selected_heading_status) << "\",";
  ss << "\"selected_heading\": " << selected_heading << ",";
  ss << "\"baro_pressure_setting_status\": \""
     << field_status_to_string(baro_pressure_setting_status) << "\",";
  ss << "\"baro_pressure_setting\": " << baro_pressure_setting << ",";
  ss << "\"n_messages\": " << n_messages << ",";
  ss << "\"last_seen\": " << this->last_seen();
  ss << "}";
  return ss.str();
}