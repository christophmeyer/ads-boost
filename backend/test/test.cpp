#include <unistd.h>

#include "gtest/gtest.h"

std::string g_testdata_path = "";

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}