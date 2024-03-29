
#include "adsb_message.h"

#include <gtest/gtest.h>

#include "demodulator.h"

class ADSBMessageTest : public ::testing::Test {
 protected:
  ADSBMessageTest() {}
};

// identification message
TEST_F(ADSBMessageTest, ADSBMessageTestDecodeIdentificationMessage) {
  std::array<unsigned char, 14> message = {0x8d, 0x44, 0x0c, 0x36, 0x23,
                                           0x14, 0xa5, 0x76, 0xc8, 0xb2,
                                           0xa0, 0x73, 0x76, 0xfe};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 5);
  EXPECT_EQ(msg.icao, "440C36");
  EXPECT_EQ(msg.type_code, 4);
  EXPECT_EQ(msg.tc19_subtype, -1);
  EXPECT_EQ(msg.callsign, "EJU62KJ ");
  EXPECT_EQ(msg.aircraft_category, 3);
}

// airborne position message
TEST_F(ADSBMessageTest, ADSBMessageTestDecodeAirbornePositionMessage) {
  std::array<unsigned char, 14> message = {0x8d, 0x78, 0x1d, 0xa5, 0x58,
                                           0xc3, 0x86, 0xac, 0xda, 0x83,
                                           0xe5, 0xbe, 0x1d, 0x7a};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 5);
  EXPECT_EQ(msg.icao, "781DA5");
  EXPECT_EQ(msg.type_code, 11);
  EXPECT_NEAR(msg.lat_cpr, 0.668800, 1e-6);
  EXPECT_NEAR(msg.lon_cpr, 0.257607, 1e-6);
  EXPECT_EQ(msg.altitude, 38000);
  EXPECT_EQ(msg.altitude_type, BAROMETRIC_ALT);
  EXPECT_EQ(msg.cpr_format, 1);
}

// airborne position message
TEST_F(ADSBMessageTest, ADSBMessageTestDecodeAirbornePositionMessageOdd) {
  std::array<unsigned char, 14> message = {0x8d, 0x78, 0x1d, 0xa5, 0x58,
                                           0xc3, 0x83, 0x43, 0x02, 0x96,
                                           0x09, 0xbc, 0xfe, 0x06};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 5);
  EXPECT_EQ(msg.icao, "781DA5");
  EXPECT_EQ(msg.type_code, 11);
  EXPECT_NEAR(msg.lat_cpr, 0.815437, 1e-6);
  EXPECT_NEAR(msg.lon_cpr, 0.293037, 1e-6);
  EXPECT_EQ(msg.altitude, 38000);
  EXPECT_EQ(msg.altitude_type, BAROMETRIC_ALT);
  EXPECT_EQ(msg.cpr_format, 0);
}

// airspeed message subtype 1
TEST_F(ADSBMessageTest, ADSBMessageTestDecodeAirspeedMessageType3) {
  std::array<unsigned char, 14> message = {0x8d, 0x78, 0x1d, 0xa5, 0x99,
                                           0x15, 0x9c, 0x9e, 0x10, 0x04,
                                           0xb1, 0x8b, 0xb6, 0x4a};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 5);
  EXPECT_EQ(msg.icao, "781DA5");
  EXPECT_EQ(msg.type_code, 19);
  EXPECT_EQ(msg.tc19_subtype, 1);
  EXPECT_EQ(msg.speed_type, GROUND_SPEED);
  EXPECT_NEAR(msg.heading, 239.821634, 1e-6);
  EXPECT_EQ(msg.heading_type, TRACK_ANGLE);
  EXPECT_NEAR(msg.speed, 475.438744, 1e-6);
  EXPECT_EQ(msg.intent_change_flag, FALSE);
  EXPECT_EQ(msg.ifr_capability_flag, FALSE);
  EXPECT_EQ(msg.nav_uncertainty_category, 2);
  EXPECT_EQ(msg.vertical_rate_source, BAROMETER);
  EXPECT_EQ(msg.vertical_rate, 0);
  EXPECT_EQ(msg.altitude_delta, -1200);
}

// airspeed message subtype 3
TEST_F(ADSBMessageTest, ADSBMessageTestDecodeAirspeedMessageType1) {
  std::array<unsigned char, 14> message = {0x8d, 0x50, 0x3e, 0x9a, 0x9b,
                                           0x06, 0xa8, 0x18, 0x70, 0x44,
                                           0x00, 0x2d, 0x8d, 0xb8};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 5);
  EXPECT_EQ(msg.icao, "503E9A");
  EXPECT_EQ(msg.type_code, 19);
  EXPECT_EQ(msg.tc19_subtype, 3);
  EXPECT_NEAR(msg.speed, 194, 1e-2);
  EXPECT_EQ(msg.speed_type, INDICATED_AIRSPEED);
  EXPECT_NEAR(msg.heading, 239.0625, 1e-2);
  EXPECT_EQ(msg.heading_type, MAGNETIC);
  EXPECT_EQ(msg.intent_change_flag, FALSE);
  EXPECT_EQ(msg.ifr_capability_flag, FALSE);
  EXPECT_EQ(msg.nav_uncertainty_category, 0);
  EXPECT_EQ(msg.vertical_rate_source, BAROMETER);
  EXPECT_EQ(msg.vertical_rate, 1024);
  EXPECT_EQ(msg.altitude_delta, 0);
}

TEST_F(ADSBMessageTest, ADSBMessageTestDecodeGroundMovement) {
  std::array<unsigned char, 14> message = {0x8c, 0x3c, 0x64, 0x45, 0x3a,
                                           0x5c, 0x53, 0xa5, 0x5e, 0xcd,
                                           0x24, 0x79, 0xd8, 0xee};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 4);
  EXPECT_EQ(msg.icao, "3C6445");
  EXPECT_EQ(msg.type_code, 7);
  EXPECT_NEAR(msg.speed, 14, 1e-6);
  EXPECT_EQ(msg.speed_type, GROUND_MOVEMENT);
  EXPECT_NEAR(msg.heading, 194.0625, 1e-6);
  EXPECT_EQ(msg.heading_type, GROUND_HEADING);
}

TEST_F(ADSBMessageTest, ADSBMessageTestDecodeGroundMovement2) {
  std::array<unsigned char, 14> message = {0x8c, 0x3c, 0x64, 0x45, 0x3a,
                                           0x9b, 0x85, 0x52, 0xbe, 0x80,
                                           0x39, 0xe4, 0xb9, 0x50};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 4);
  EXPECT_EQ(msg.icao, "3C6445");
  EXPECT_EQ(msg.type_code, 7);
  EXPECT_NEAR(msg.speed, 17, 1e-6);
  EXPECT_EQ(msg.speed_type, GROUND_MOVEMENT);
  EXPECT_NEAR(msg.heading, 157.5, 1e-2);
  EXPECT_EQ(msg.heading_type, GROUND_HEADING);
}

TEST_F(ADSBMessageTest, ADSBMessageTestDecodeGroundMovement3) {
  std::array<unsigned char, 14> message = {0x8c, 0x3f, 0x59, 0xf3, 0x3a,
                                           0x99, 0x93, 0xad, 0xa2, 0xcf,
                                           0xa0, 0xe5, 0xa0, 0xc5};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 4);
  EXPECT_EQ(msg.icao, "3F59F3");
  EXPECT_EQ(msg.type_code, 7);
  EXPECT_NEAR(msg.speed, 17, 1e-6);
  EXPECT_EQ(msg.speed_type, GROUND_MOVEMENT);
  EXPECT_NEAR(msg.heading, 70.3125, 1e-6);
  EXPECT_EQ(msg.heading_type, GROUND_HEADING);
}

TEST_F(ADSBMessageTest, ADSBMessageTestDecodeGroundMovement4) {
  std::array<unsigned char, 14> message = {0x8c, 0x3f, 0x59, 0xf3, 0x3a,
                                           0x99, 0x95, 0x59, 0xae, 0x82,
                                           0xc3, 0x1f, 0xff, 0xd5};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 4);
  EXPECT_EQ(msg.icao, "3F59F3");
  EXPECT_EQ(msg.type_code, 7);
  EXPECT_NEAR(msg.speed, 17, 1e-6);
  EXPECT_EQ(msg.speed_type, GROUND_MOVEMENT);
  EXPECT_NEAR(msg.heading, 70.3125, 1e-6);
  EXPECT_EQ(msg.heading_type, GROUND_HEADING);
}

TEST_F(ADSBMessageTest, ADSBMessageTestHexString) {
  std::array<unsigned char, 14> message = {0x8c, 0x44, 0x0d, 0xa5, 0x38,
                                           0x1f, 0x93, 0xa1, 0xc4, 0xcf,
                                           0xf5, 0x8f, 0xe8, 0x76};
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.HexString(), "8c440da5381f93a1c4cff58fe876");
  for (int n = 0; n < 14; n++) {
    EXPECT_EQ(msg.message[n], message[n]);
  }
}

TEST_F(ADSBMessageTest, ADSBMessageTestDecodeStatus1) {
  std::array<unsigned char, 14> message = {0x8d, 0x3d, 0x2c, 0xc2, 0xea,
                                           0x03, 0x68, 0x57, 0xb7, 0x53,
                                           0x40, 0xa4, 0xb3, 0xc4};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 5);
  EXPECT_EQ(msg.icao, "3D2CC2");
  EXPECT_EQ(msg.type_code, 29);
  EXPECT_EQ(msg.autopilot, TRUE);
  EXPECT_EQ(msg.lnav_mode, FALSE);
  EXPECT_EQ(msg.vnav_mode, FALSE);
  EXPECT_EQ(msg.approach_mode, FALSE);
  EXPECT_EQ(msg.altitude_hold_mode, TRUE);
  EXPECT_EQ(msg.tcas_operational, FALSE);
  EXPECT_EQ(msg.selected_altitude, 1696);
  EXPECT_EQ(msg.selected_altitude_source, MCPFCU);
  EXPECT_EQ(msg.selected_heading_status, KNOWN);
  EXPECT_NEAR(msg.selected_heading, 333.984375, 1e-4);
  EXPECT_NEAR(msg.baro_pressure_setting, 1012.0, 1e-4);
}

TEST_F(ADSBMessageTest, ADSBMessageTestDecodeStatus2) {
  std::array<unsigned char, 14> message = {0x8d, 0x44, 0x01, 0x75, 0xea,
                                           0x0d, 0xc8, 0x66, 0x1d, 0x3c,
                                           0x08, 0xf2, 0x1b, 0xdc};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 5);
  EXPECT_EQ(msg.icao, "440175");
  EXPECT_EQ(msg.type_code, 29);
  EXPECT_EQ(msg.autopilot, UNDETERMINED_BOOL);
  EXPECT_EQ(msg.lnav_mode, UNDETERMINED_BOOL);
  EXPECT_EQ(msg.vnav_mode, UNDETERMINED_BOOL);
  EXPECT_EQ(msg.approach_mode, UNDETERMINED_BOOL);
  EXPECT_EQ(msg.altitude_hold_mode, UNDETERMINED_BOOL);
  EXPECT_EQ(msg.tcas_operational, TRUE);
  EXPECT_EQ(msg.selected_altitude, 7008);
  EXPECT_EQ(msg.selected_altitude_source, MCPFCU);
  EXPECT_EQ(msg.selected_heading_status, KNOWN);
  EXPECT_NEAR(msg.selected_heading, 189.84375, 1e-4);
  EXPECT_NEAR(msg.baro_pressure_setting, 1013.6, 1e-4);
}

TEST_F(ADSBMessageTest, ADSBMessageTestDecodeStatus3) {
  std::array<unsigned char, 14> message = {0x8d, 0x30, 0x07, 0xf2, 0xea,
                                           0x46, 0x68, 0x60, 0x01, 0x7f,
                                           0x48, 0x66, 0x63, 0x6b};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 5);
  EXPECT_EQ(msg.icao, "3007F2");
  EXPECT_EQ(msg.type_code, 29);
  EXPECT_EQ(msg.autopilot, TRUE);
  EXPECT_EQ(msg.lnav_mode, FALSE);
  EXPECT_EQ(msg.vnav_mode, FALSE);
  EXPECT_EQ(msg.approach_mode, FALSE);
  EXPECT_EQ(msg.altitude_hold_mode, TRUE);
  EXPECT_EQ(msg.tcas_operational, TRUE);
  EXPECT_EQ(msg.selected_altitude, 36000);
  EXPECT_EQ(msg.selected_altitude_source, MCPFCU);
  EXPECT_EQ(msg.selected_heading_status, UNDETERMINED);
  EXPECT_NEAR(msg.selected_heading, 0, 1e-4);
  EXPECT_NEAR(msg.baro_pressure_setting, 1013.6, 1e-4);
}

TEST_F(ADSBMessageTest, ADSBMessageTestDecodeStatus4) {
  std::array<unsigned char, 14> message = {0x8d, 0x46, 0x1e, 0x21, 0xea,
                                           0x46, 0x68, 0x60, 0x01, 0x7f,
                                           0x88, 0x43, 0xb3, 0x77};
  EXPECT_EQ(true, check_crc(&message));
  ADSBMessage msg = ADSBMessage(message);
  EXPECT_EQ(msg.downlink_format, 17);
  EXPECT_EQ(msg.downlink_capability, 5);
  EXPECT_EQ(msg.icao, "461E21");
  EXPECT_EQ(msg.type_code, 29);
  EXPECT_EQ(msg.autopilot, TRUE);
  EXPECT_EQ(msg.lnav_mode, FALSE);
  EXPECT_EQ(msg.vnav_mode, TRUE);
  EXPECT_EQ(msg.approach_mode, FALSE);
  EXPECT_EQ(msg.altitude_hold_mode, FALSE);
  EXPECT_EQ(msg.tcas_operational, TRUE);
  EXPECT_EQ(msg.selected_altitude, 36000);
  EXPECT_EQ(msg.selected_altitude_source, MCPFCU);
  EXPECT_EQ(msg.selected_heading_status, UNDETERMINED);
  EXPECT_NEAR(msg.selected_heading, 0, 1e-4);
  EXPECT_NEAR(msg.baro_pressure_setting, 1013.6, 1e-4);
}
