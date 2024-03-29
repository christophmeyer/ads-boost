
#include "contact.h"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "demodulator.h"

class ContactTest : public ::testing::Test {
 protected:
  ContactTest() {}
};

TEST_F(ContactTest, ContactTestInitialization) {
  std::array<unsigned char, 14> message = {0x8d, 0x3c, 0x65, 0x85, 0x23,
                                           0x10, 0xc2, 0x34, 0x04, 0x88,
                                           0x20, 0x5a, 0x8f, 0xaf};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  Contact contact = Contact(msg);
  EXPECT_EQ(contact.icao, "3C6585");
  EXPECT_EQ(contact.callsign, "DLH4AH  ");
}

TEST_F(ContactTest, ContactTestToJson) {
  std::array<unsigned char, 14> message = {0x8d, 0x3c, 0x65, 0x85, 0x23,
                                           0x10, 0xc2, 0x34, 0x04, 0x88,
                                           0x20, 0x5a, 0x8f, 0xaf};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  Contact contact = Contact(msg);
  EXPECT_EQ(contact.icao, "3C6585");
  EXPECT_EQ(contact.callsign, "DLH4AH  ");
  EXPECT_EQ(
      contact.to_json(),
      "{\"icao\": \"3C6585\",\"callsign\": \"DLH4AH  \",\"aircraft_category\": "
      "\"MED2\",\"speed_type\": \"UNDETERMINED\",\"speed\": "
      "\"0\",\"heading_type\": \"UNDETERMINED\",\"heading\": "
      "0,\"altitude_type\": \"UNDETERMINED\",\"altitude\": "
      "0,\"vertical_rate_source\": \"UNDETERMINED\",\"vertical_rate_status\": "
      "\"UNDETERMINED\",\"vertical_rate\": 0,\"altitude_delta\": "
      "0,\"altitude_delta_status\": \"UNDETERMINED\",\"position_status\": "
      "\"UNDETERMINED\",\"lat\": 0,\"lon\": 0,\"position_ref_status\": "
      "\"UNDETERMINED\",\"lat_ref\": 0,\"lon_ref\": 0,\"autopilot\": "
      "\"NA\",\"lnav_mode\": \"NA\",\"vnav_mode\": \"NA\",\"approach_mode\": "
      "\"NA\",\"tcas_operational\": \"NA\",\"altitude_hold_mode\": "
      "\"NA\",\"intent_change_flag\": \"NA\",\"ifr_capability_flag\": "
      "\"NA\",\"nav_uncertainty_category_status\": "
      "\"UNDETERMINED\",\"nav_uncertainty_category\": "
      "-1,\"selected_altitude_source\": "
      "\"UNDETERMINED\",\"selected_altitude_status\": "
      "\"UNDETERMINED\",\"selected_altitude\": 0,\"selected_heading_status\": "
      "\"UNDETERMINED\",\"selected_heading\": "
      "0,\"baro_pressure_setting_status\": "
      "\"UNDETERMINED\",\"baro_pressure_setting\": -1,\"n_messages\": "
      "1,\"last_seen\": 0}");
}

TEST_F(ContactTest, ContactTestAirbornePositionUpdate1) {
  std::array<unsigned char, 14> message_even = {0x8d, 0x4b, 0xce, 0x15, 0x60,
                                                0x09, 0x02, 0xea, 0x94, 0xb7,
                                                0xdc, 0xb2, 0x52, 0x58};

  std::array<unsigned char, 14> message_odd = {0x8d, 0x4b, 0xce, 0x15, 0x60,
                                               0x09, 0x06, 0x55, 0x9e, 0xa4,
                                               0x8d, 0x88, 0xec, 0xe0};
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  Contact contact = Contact(msg_odd);
  contact.update(msg_even);
  EXPECT_EQ(contact.icao, "4BCE15");
  EXPECT_EQ(contact.altitude, 600);
  EXPECT_EQ(contact.altitude_type, BAROMETRIC_ALT);
  EXPECT_NEAR(contact.lon, 13.5910, 1e-4);
  EXPECT_NEAR(contact.lat, 52.3745, 1e-4);
}

TEST_F(ContactTest, ContactTestAirbornePositionUpdate2) {
  std::array<unsigned char, 14> message_even = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                                0xc3, 0x93, 0xbc, 0x05, 0xfd,
                                                0x7f, 0xf0, 0x81, 0x1e};

  std::array<unsigned char, 14> message_odd = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                               0xc3, 0x97, 0x10, 0x99, 0x97,
                                               0x75, 0x32, 0xc6, 0x1e};
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  Contact contact = Contact(msg_odd);
  contact.update(msg_even);
  EXPECT_EQ(contact.icao, "4D2414");
  EXPECT_EQ(contact.altitude, 38025);
  EXPECT_EQ(contact.altitude_type, BAROMETRIC_ALT);
  EXPECT_NEAR(contact.lon, 71.9413, 1e-4);
  EXPECT_NEAR(contact.lat, 59.6017, 1e-4);
}

TEST_F(ContactTest, ContactTestAirbornePositionUpdate3) {
  std::array<unsigned char, 14> message_even = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                                0xc3, 0x92, 0xbf, 0x88, 0xd3,
                                                0xc3, 0x5d, 0xc5, 0x3e};

  std::array<unsigned char, 14> message_odd = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                               0xc3, 0x96, 0x3d, 0x5f, 0x7e,
                                               0x3c, 0xe0, 0x03, 0xc2};
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  Contact contact = Contact(msg_odd);
  contact.update(msg_even);
  EXPECT_EQ(contact.icao, "4D2414");
  EXPECT_EQ(contact.altitude, 38025);
  EXPECT_EQ(contact.altitude_type, BAROMETRIC_ALT);
  EXPECT_NEAR(contact.lon, -119.2952, 1e-4);
  EXPECT_NEAR(contact.lat, 46.1223, 1e-4);
}

TEST_F(ContactTest, ContactTestAirbornePositionUpdate4) {
  std::array<unsigned char, 14> message_even = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                                0xc3, 0x93, 0xde, 0x60, 0xb1,
                                                0x7a, 0xe6, 0x3a, 0x43};

  std::array<unsigned char, 14> message_odd = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                               0xc3, 0x94, 0xb9, 0x69, 0xff,
                                               0xc0, 0x0e, 0x56, 0xfc};
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  Contact contact = Contact(msg_odd);
  contact.update(msg_even);
  EXPECT_EQ(contact.icao, "4D2414");
  EXPECT_EQ(contact.altitude, 38025);
  EXPECT_EQ(contact.altitude_type, BAROMETRIC_ALT);
  EXPECT_NEAR(contact.lon, 130.3990, 1e-4);
  EXPECT_NEAR(contact.lat, -78.1970, 1e-4);
}

TEST_F(ContactTest, ContactTestAirbornePositionUpdate5) {
  std::array<unsigned char, 14> message_even = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                                0xc3, 0x90, 0xb0, 0x37, 0x83,
                                                0x31, 0x8c, 0x67, 0x10};

  std::array<unsigned char, 14> message_odd = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                               0xc3, 0x94, 0xfc, 0xd8, 0x2c,
                                               0xef, 0x83, 0xb9, 0xcf};
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  Contact contact = Contact(msg_odd);
  contact.update(msg_even);
  EXPECT_EQ(contact.icao, "4D2414");
  EXPECT_EQ(contact.altitude, 38025);
  EXPECT_EQ(contact.altitude_type, BAROMETRIC_ALT);
  EXPECT_NEAR(contact.lon, -119.3799, 1e-4);
  EXPECT_NEAR(contact.lat, -28.9675, 1e-4);
}

TEST_F(ContactTest, ContactTestGroundPositionUpdate1) {
  std::array<unsigned char, 14> message_even = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                                0x1f, 0x93, 0xa1, 0xc4, 0xcf,
                                                0xf5, 0x8f, 0xe8, 0x76};

  std::array<unsigned char, 14> message_odd = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                               0x1f, 0x95, 0x4e, 0x00, 0x83,
                                               0x12, 0x11, 0x0a, 0xa8};
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  Contact contact = Contact(msg_even, 51.99, 4.375);
  contact.update(msg_odd);
  EXPECT_EQ(contact.icao, "440DA5");
  EXPECT_NEAR(contact.heading, 340.3125, 1e-2);
  EXPECT_EQ(contact.heading_type, GROUND_HEADING);
  EXPECT_EQ(contact.speed, 0);
  EXPECT_EQ(contact.speed_type, GROUND_MOVEMENT);
  EXPECT_NEAR(contact.lon, 13.5154, 1e-4);
  EXPECT_NEAR(contact.lat, 52.3620, 1e-4);
}

TEST_F(ContactTest, ContactTestGroundPositionUpdate2) {
  std::array<unsigned char, 14> message_even = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                                0x1f, 0x92, 0xda, 0x28, 0xa4,
                                                0x85, 0xce, 0x0b, 0x56};

  std::array<unsigned char, 14> message_odd = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                               0x1f, 0x96, 0x05, 0xf3, 0xa7,
                                               0x55, 0x23, 0xad, 0x61};
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  Contact contact = Contact(msg_even, 51.99, 4.375);
  contact.update(msg_odd);
  EXPECT_EQ(contact.icao, "440DA5");
  EXPECT_NEAR(contact.heading, 340.3125, 1e-2);
  EXPECT_EQ(contact.heading_type, GROUND_HEADING);
  EXPECT_EQ(contact.speed, 0);
  EXPECT_EQ(contact.speed_type, GROUND_MOVEMENT);
  EXPECT_NEAR(contact.lon, 43.8984, 1e-4);
  EXPECT_NEAR(contact.lat, 19.0767, 1e-4);
}

TEST_F(ContactTest, ContactTestGroundPositionUpdate3) {
  std::array<unsigned char, 14> message_even = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                                0x1f, 0x91, 0x95, 0x22, 0x93,
                                                0xd3, 0xc5, 0x31, 0x0d};

  std::array<unsigned char, 14> message_odd = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                               0x1f, 0x96, 0x8b, 0xea, 0xe1,
                                               0x37, 0x9e, 0xf0, 0xfd};
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  Contact contact = Contact(msg_even, 39.1234, -34.6732);
  contact.update(msg_odd);
  EXPECT_EQ(contact.icao, "440DA5");
  EXPECT_NEAR(contact.heading, 340.3125, 1e-2);
  EXPECT_EQ(contact.heading_type, GROUND_HEADING);
  EXPECT_EQ(contact.speed, 0);
  EXPECT_EQ(contact.speed_type, GROUND_MOVEMENT);
  EXPECT_NEAR(contact.lon, -15.2576, 1e-4);
  EXPECT_NEAR(contact.lat, 68.0898, 1e-4);
}

TEST_F(ContactTest, ContactTestGroundPositionUpdate4) {
  std::array<unsigned char, 14> message_even = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                                0x1f, 0x92, 0xda, 0x28, 0xa4,
                                                0x85, 0xce, 0x0b, 0x56};

  std::array<unsigned char, 14> message_odd = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                               0x1f, 0x96, 0x05, 0xf3, 0xa7,
                                               0x55, 0x23, 0xad, 0x61};
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  Contact contact = Contact(msg_even, -19.9342, 16.2850);
  contact.update(msg_odd);
  EXPECT_EQ(contact.icao, "440DA5");
  EXPECT_NEAR(contact.heading, 340.3125, 1e-2);
  EXPECT_EQ(contact.heading_type, GROUND_HEADING);
  EXPECT_EQ(contact.speed, 0);
  EXPECT_EQ(contact.speed_type, GROUND_MOVEMENT);
  EXPECT_NEAR(contact.lon, 44.1341, 1e-4);
  EXPECT_NEAR(contact.lat, -70.9233, 1e-4);
}

TEST_F(ContactTest, ContactTestGroundPositionUpdate5) {
  std::array<unsigned char, 14> message_even = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                                0x1f, 0x91, 0x95, 0x22, 0x93,
                                                0xd3, 0xc5, 0x31, 0x0d};

  std::array<unsigned char, 14> message_odd = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                               0x1f, 0x96, 0x8b, 0xea, 0xe1,
                                               0x37, 0x9e, 0xf0, 0xfd};
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  Contact contact = Contact(msg_even, -10.4501, 13.2431);
  contact.update(msg_odd);
  EXPECT_EQ(contact.icao, "440DA5");
  EXPECT_NEAR(contact.heading, 340.3125, 1e-2);
  EXPECT_EQ(contact.heading_type, GROUND_HEADING);
  EXPECT_EQ(contact.speed, 0);
  EXPECT_EQ(contact.speed_type, GROUND_MOVEMENT);
  EXPECT_NEAR(contact.lon, -14.2669, 1e-4);
  EXPECT_NEAR(contact.lat, -21.9102, 1e-4);
}

TEST_F(ContactTest, ContactTestGroundPositionUpdateNoReference) {
  std::array<unsigned char, 14> message_even = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                                0x1f, 0x91, 0x95, 0x22, 0x93,
                                                0xd3, 0xc5, 0x31, 0x0d};

  std::array<unsigned char, 14> message_odd = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                               0x1f, 0x96, 0x8b, 0xea, 0xe1,
                                               0x37, 0x9e, 0xf0, 0xfd};
  EXPECT_EQ(true, check_crc(&message_even));
  ADSBMessage msg_even = ADSBMessage(message_even);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_EQ(true, check_crc(&message_odd));
  ADSBMessage msg_odd = ADSBMessage(message_odd);
  Contact contact = Contact(msg_even);
  double prev_lat = contact.lat;
  double prev_lon = contact.lon;
  contact.update(msg_odd);
  EXPECT_EQ(contact.icao, "440DA5");
  EXPECT_NEAR(contact.lon, prev_lon, 1e-4);
  EXPECT_NEAR(contact.lat, prev_lat, 1e-4);
}

TEST_F(ContactTest, ContactTestAirspeed1) {
  std::array<unsigned char, 14> message = {0x8d, 0x78, 0x1d, 0xa5, 0x99,
                                           0x15, 0x9c, 0x9e, 0x10, 0x04,
                                           0xb1, 0x8b, 0xb6, 0x4a};

  ADSBMessage msg = ADSBMessage(message);
  Contact contact = Contact(msg);
  EXPECT_EQ(contact.icao, "781DA5");
  EXPECT_EQ(contact.vertical_rate, 0);
  EXPECT_EQ(contact.vertical_rate_source, BAROMETER);
  EXPECT_EQ(contact.altitude_delta, -1200);
  EXPECT_NEAR(contact.speed, 475.44, 1e-2);
  EXPECT_EQ(contact.speed_type, GROUND_SPEED);
  EXPECT_NEAR(contact.heading, 239.82, 1e-2);
  EXPECT_EQ(contact.heading_type, TRACK_ANGLE);
}

TEST_F(ContactTest, ContactTestAirspeed2) {
  std::array<unsigned char, 14> message = {0x8d, 0x50, 0x3e, 0x9a, 0x9b,
                                           0x06, 0xa8, 0x18, 0x70, 0x44,
                                           0x00, 0x2d, 0x8d, 0xb8};

  ADSBMessage msg = ADSBMessage(message);
  Contact contact = Contact(msg);
  EXPECT_EQ(contact.icao, "503E9A");
  EXPECT_EQ(contact.vertical_rate, 1024);
  EXPECT_EQ(contact.vertical_rate_source, BAROMETER);
  EXPECT_EQ(contact.altitude_delta, 0);
  EXPECT_NEAR(contact.heading, 239.06, 1e-2);
  EXPECT_EQ(contact.heading_type, MAGNETIC);
  EXPECT_EQ(contact.speed, 194);
  EXPECT_EQ(contact.speed_type, INDICATED_AIRSPEED);
}

TEST_F(ContactTest, ContactListTestAirborneUpdate) {
  std::array<unsigned char, 14> message_1 = {0x8d, 0x3c, 0x65, 0x85, 0x23,
                                             0x10, 0xc2, 0x34, 0x04, 0x88,
                                             0x20, 0x5a, 0x8f, 0xaf};
  std::array<unsigned char, 14> message_2 = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                             0xc3, 0x93, 0xbc, 0x05, 0xfd,
                                             0x7f, 0xf0, 0x81, 0x1e};
  std::array<unsigned char, 14> message_3_even = {0x8d, 0x4b, 0xce, 0x15, 0x60,
                                                  0x09, 0x02, 0xea, 0x94, 0xb7,
                                                  0xdc, 0xb2, 0x52, 0x58};

  std::array<unsigned char, 14> message_3_odd = {0x8d, 0x4b, 0xce, 0x15, 0x60,
                                                 0x09, 0x06, 0x55, 0x9e, 0xa4,
                                                 0x8d, 0x88, 0xec, 0xe0};
  ADSBMessage msg = ADSBMessage(message_1);
  ContactList contacts = ContactList(10);
  contacts.update(message_1);
  Contact* contact_1 = contacts.get_contact("3C6585");
  Contact* contact_x = contacts.get_contact("NOICAO");
  EXPECT_EQ(contact_x, nullptr);
  EXPECT_EQ(contact_1->icao, "3C6585");
  EXPECT_EQ(contacts.contacts.size(), 1);
  EXPECT_EQ(contacts.get_contacts()->size(), 1);
  contacts.update(message_2);
  contact_1 = contacts.get_contact("3C6585");
  Contact* contact_2 = contacts.get_contact("4D2414");
  EXPECT_EQ(contact_1->icao, "3C6585");
  EXPECT_EQ(contact_2->icao, "4D2414");
  EXPECT_EQ(contacts.contacts.size(), 2);
  EXPECT_EQ(contacts.get_contacts()->size(), 2);
  contacts.update(message_3_odd);
  contact_1 = contacts.get_contact("3C6585");
  contact_2 = contacts.get_contact("4D2414");
  Contact* contact_3 = contacts.get_contact("4BCE15");
  EXPECT_EQ(contact_1->icao, "3C6585");
  EXPECT_EQ(contact_2->icao, "4D2414");
  EXPECT_EQ(contact_3->icao, "4BCE15");
  EXPECT_EQ(contact_3->lon, 0);
  EXPECT_EQ(contact_3->lat, 0);
  EXPECT_EQ(contacts.contacts.size(), 3);
  EXPECT_EQ(contacts.get_contacts()->size(), 3);
  contacts.update(message_3_even);
  EXPECT_NEAR(contact_3->lon, 13.5910, 1e-4);
  EXPECT_NEAR(contact_3->lat, 52.3745, 1e-4);
  EXPECT_EQ(contacts.get_contacts()->size(), 3);
}

TEST_F(ContactTest, ContactListTestGroundPositionUpdate) {
  std::array<unsigned char, 14> message_1 = {0x8d, 0x3c, 0x65, 0x85, 0x23,
                                             0x10, 0xc2, 0x34, 0x04, 0x88,
                                             0x20, 0x5a, 0x8f, 0xaf};
  std::array<unsigned char, 14> message_2 = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                             0xc3, 0x93, 0xbc, 0x05, 0xfd,
                                             0x7f, 0xf0, 0x81, 0x1e};
  std::array<unsigned char, 14> message_3_even = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                                  0x1f, 0x93, 0xa1, 0xc4, 0xcf,
                                                  0xf5, 0x8f, 0xe8, 0x76};

  std::array<unsigned char, 14> message_3_odd = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                                 0x1f, 0x95, 0x4e, 0x00, 0x83,
                                                 0x12, 0x11, 0x0a, 0xa8};
  ADSBMessage msg = ADSBMessage(message_1);
  ContactList contacts = ContactList(10, 51.99, 4.375);
  EXPECT_EQ(contacts.lat_ref, 51.99);
  EXPECT_EQ(contacts.lon_ref, 4.375);
  contacts.update(message_1);
  Contact* contact_1 = contacts.get_contact("3C6585");
  Contact* contact_x = contacts.get_contact("NOICAO");
  EXPECT_EQ(contact_x, nullptr);
  EXPECT_EQ(contact_1->icao, "3C6585");
  EXPECT_EQ(contacts.contacts.size(), 1);
  EXPECT_EQ(contacts.get_contacts()->size(), 1);
  contacts.update(message_2);
  contact_1 = contacts.get_contact("3C6585");
  Contact* contact_2 = contacts.get_contact("4D2414");
  EXPECT_EQ(contact_1->icao, "3C6585");
  EXPECT_EQ(contact_2->icao, "4D2414");
  EXPECT_EQ(contacts.contacts.size(), 2);
  EXPECT_EQ(contacts.get_contacts()->size(), 2);
  contacts.update(message_3_even);
  contact_1 = contacts.get_contact("3C6585");
  contact_2 = contacts.get_contact("4D2414");
  Contact* contact_3 = contacts.get_contact("440DA5");
  EXPECT_EQ(contact_1->icao, "3C6585");
  EXPECT_EQ(contact_2->icao, "4D2414");
  EXPECT_EQ(contact_3->icao, "440DA5");
  EXPECT_EQ(contact_3->lon, 0);
  EXPECT_EQ(contact_3->lat, 0);
  EXPECT_EQ(contacts.contacts.size(), 3);
  EXPECT_EQ(contacts.get_contacts()->size(), 3);
  contacts.update(message_3_odd);
  EXPECT_NEAR(contact_3->lon, 13.5154, 1e-4);
  EXPECT_NEAR(contact_3->lat, 52.3620, 1e-4);
  EXPECT_EQ(contacts.get_contacts()->size(), 3);
}

TEST_F(ContactTest, ContactListTestTimeout) {
  std::array<unsigned char, 14> message_1 = {0x8d, 0x3c, 0x65, 0x85, 0x23,
                                             0x10, 0xc2, 0x34, 0x04, 0x88,
                                             0x20, 0x5a, 0x8f, 0xaf};
  std::array<unsigned char, 14> message_2 = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                             0xc3, 0x93, 0xbc, 0x05, 0xfd,
                                             0x7f, 0xf0, 0x81, 0x1e};
  std::array<unsigned char, 14> message_3_even = {0x8d, 0x4b, 0xce, 0x15, 0x60,
                                                  0x09, 0x02, 0xea, 0x94, 0xb7,
                                                  0xdc, 0xb2, 0x52, 0x58};

  std::array<unsigned char, 14> message_3_odd = {0x8d, 0x4b, 0xce, 0x15, 0x60,
                                                 0x09, 0x06, 0x55, 0x9e, 0xa4,
                                                 0x8d, 0x88, 0xec, 0xe0};
  ADSBMessage msg_1 = ADSBMessage(message_1);
  ContactList contacts = ContactList(1);
  contacts.update(msg_1);
  Contact* contact_1 = contacts.get_contact("3C6585");
  Contact* contact_x = contacts.get_contact("NOICAO");
  EXPECT_EQ(contact_x, nullptr);
  EXPECT_EQ(contact_1->icao, "3C6585");
  EXPECT_EQ(contacts.contacts.size(), 1);
  EXPECT_EQ(contacts.get_contacts()->size(), 1);

  // Let first contact timeout
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  ADSBMessage msg_2 = ADSBMessage(message_2);
  contacts.update(msg_2);
  Contact* contact_2 = contacts.get_contact("4D2414");
  EXPECT_EQ(contact_2->icao, "4D2414");
  EXPECT_EQ(contacts.get_contacts()->size(), 1);
  ADSBMessage msg_3_odd = ADSBMessage(message_3_odd);
  contacts.update(message_3_odd);
  contact_2 = contacts.get_contact("4D2414");
  Contact* contact_3 = contacts.get_contact("4BCE15");
  EXPECT_EQ(contact_2->icao, "4D2414");
  EXPECT_EQ(contact_3->icao, "4BCE15");
  EXPECT_EQ(contact_3->lon, 0);
  EXPECT_EQ(contact_3->lat, 0);
  EXPECT_EQ(contacts.get_contacts()->size(), 2);
  ADSBMessage msg_3_even = ADSBMessage(message_3_even);
  contacts.update(message_3_even);
  EXPECT_NEAR(contact_3->lon, 13.5910, 1e-4);
  EXPECT_NEAR(contact_3->lat, 52.3745, 1e-4);
  EXPECT_EQ(contacts.get_contacts()->size(), 2);
}

TEST_F(ContactTest, ContactListTestToJson) {
  std::array<unsigned char, 14> message_1 = {0x8d, 0x3c, 0x65, 0x85, 0x23,
                                             0x10, 0xc2, 0x34, 0x04, 0x88,
                                             0x20, 0x5a, 0x8f, 0xaf};
  std::array<unsigned char, 14> message_2 = {0x8d, 0x4d, 0x24, 0x14, 0x58,
                                             0xc3, 0x93, 0xbc, 0x05, 0xfd,
                                             0x7f, 0xf0, 0x81, 0x1e};
  ADSBMessage msg = ADSBMessage(message_1);
  ContactList contacts = ContactList(10, 40.0, -35.0);
  contacts.update(message_1);
  contacts.update(message_2);
  EXPECT_EQ(
      contacts.to_json(),
      "{\"contacts\": [{\"icao\": \"4D2414\",\"callsign\": "
      "\"\",\"aircraft_category\": \"\",\"speed_type\": "
      "\"UNDETERMINED\",\"speed\": \"0\",\"heading_type\": "
      "\"UNDETERMINED\",\"heading\": 0,\"altitude_type\": "
      "\"BAROMETRIC\",\"altitude\": 38025,\"vertical_rate_source\": "
      "\"UNDETERMINED\",\"vertical_rate_status\": "
      "\"UNDETERMINED\",\"vertical_rate\": 0,\"altitude_delta\": "
      "0,\"altitude_delta_status\": \"UNDETERMINED\",\"position_status\": "
      "\"UNDETERMINED\",\"lat\": 0,\"lon\": 0,\"position_ref_status\": "
      "\"KNOWN\",\"lat_ref\": 40,\"lon_ref\": -35,\"autopilot\": "
      "\"NA\",\"lnav_mode\": \"NA\",\"vnav_mode\": \"NA\",\"approach_mode\": "
      "\"NA\",\"tcas_operational\": \"NA\",\"altitude_hold_mode\": "
      "\"NA\",\"intent_change_flag\": \"NA\",\"ifr_capability_flag\": "
      "\"NA\",\"nav_uncertainty_category_status\": "
      "\"UNDETERMINED\",\"nav_uncertainty_category\": "
      "-1,\"selected_altitude_source\": "
      "\"UNDETERMINED\",\"selected_altitude_status\": "
      "\"UNDETERMINED\",\"selected_altitude\": 0,\"selected_heading_status\": "
      "\"UNDETERMINED\",\"selected_heading\": "
      "0,\"baro_pressure_setting_status\": "
      "\"UNDETERMINED\",\"baro_pressure_setting\": -1,\"n_messages\": "
      "1,\"last_seen\": 0},{\"icao\": \"3C6585\",\"callsign\": \"DLH4AH  "
      "\",\"aircraft_category\": \"MED2\",\"speed_type\": "
      "\"UNDETERMINED\",\"speed\": \"0\",\"heading_type\": "
      "\"UNDETERMINED\",\"heading\": 0,\"altitude_type\": "
      "\"UNDETERMINED\",\"altitude\": 0,\"vertical_rate_source\": "
      "\"UNDETERMINED\",\"vertical_rate_status\": "
      "\"UNDETERMINED\",\"vertical_rate\": 0,\"altitude_delta\": "
      "0,\"altitude_delta_status\": \"UNDETERMINED\",\"position_status\": "
      "\"UNDETERMINED\",\"lat\": 0,\"lon\": 0,\"position_ref_status\": "
      "\"KNOWN\",\"lat_ref\": 40,\"lon_ref\": -35,\"autopilot\": "
      "\"NA\",\"lnav_mode\": \"NA\",\"vnav_mode\": \"NA\",\"approach_mode\": "
      "\"NA\",\"tcas_operational\": \"NA\",\"altitude_hold_mode\": "
      "\"NA\",\"intent_change_flag\": \"NA\",\"ifr_capability_flag\": "
      "\"NA\",\"nav_uncertainty_category_status\": "
      "\"UNDETERMINED\",\"nav_uncertainty_category\": "
      "-1,\"selected_altitude_source\": "
      "\"UNDETERMINED\",\"selected_altitude_status\": "
      "\"UNDETERMINED\",\"selected_altitude\": 0,\"selected_heading_status\": "
      "\"UNDETERMINED\",\"selected_heading\": "
      "0,\"baro_pressure_setting_status\": "
      "\"UNDETERMINED\",\"baro_pressure_setting\": -1,\"n_messages\": "
      "1,\"last_seen\": 0}]}");
}

TEST_F(ContactTest, ContactListTestToJsonEmpty) {
  ContactList contacts = ContactList(10, 40.0, -35.0);
  EXPECT_EQ(contacts.to_json(), "{\"contacts\": []}");
}