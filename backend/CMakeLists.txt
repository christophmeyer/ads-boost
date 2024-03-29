cmake_minimum_required(VERSION 3.10)
project(ads-boost VERSION 0.1 DESCRIPTION "Decoder for ADS-B")
set (CMAKE_CXX_STANDARD 20)


include(ExternalProject)
ExternalProject_Add(
    uWebSockets
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/uWebSockets
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -f GNUmakefile default
    BUILD_IN_SOURCE TRUE
    INSTALL_COMMAND ""
)

add_library(ads_boost STATIC
src/adsb_message.cpp
src/contact.cpp
src/webserver.cpp
src/sdr_handler.cpp
src/demodulator.cpp)

add_dependencies(ads_boost uWebSockets)
target_include_directories(ads_boost PUBLIC ./include ./src /usr/include /usr/local/include ./uWebSockets/src ./uWebSockets/uSockets/src)
target_link_directories(ads_boost PUBLIC ./src /usr/local/lib)

set(USOCKETS_OBJECT_FILES 
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/bsd.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/context.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/epoll_kqueue.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/gcd.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/io_context.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/io_loop.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/io_socket.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/libuv.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/loop.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/openssl.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/quic.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/socket.o"
"${CMAKE_CURRENT_SOURCE_DIR}/uWebSockets/uSockets/udp.o")

add_executable(ads-boost ./src/ads-boost.cpp)
add_dependencies(ads-boost uWebSockets)
target_include_directories(ads-boost PUBLIC ./src ./uWebSockets/uSockets)
target_compile_options(ads-boost PUBLIC -O3 -Wall -pedantic)
target_link_libraries(ads-boost PUBLIC ads_boost z rtlsdr m pthread stdc++ ${USOCKETS_OBJECT_FILES}
)

add_executable(test_runner ./test/test.cpp 
./src/adsb_message_test.cpp
./src/demodulator_test.cpp
./src/contact_test.cpp)
target_include_directories(test_runner PUBLIC ./src ./)
add_dependencies(test_runner uWebSockets)
target_link_libraries(test_runner ${USOCKETS_OBJECT_FILES} z gtest pthread ads_boost m stdc++)
INSTALL(TARGETS test_runner DESTINATION test/bin COMPONENT tests)   
