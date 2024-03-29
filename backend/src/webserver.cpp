#include "webserver.h"

#include <time.h>

#include <iostream>
#include <thread>

uWS::App *globalApp;

void broadcast_callback(us_timer_t *timer) {
  SharedContactList **shared_contacts =
      reinterpret_cast<SharedContactList **>(us_timer_ext(timer));
  {
    std::unique_lock<std::mutex> lock{(*shared_contacts)->mutex};
    globalApp->publish(
        "broadcast",
        std::string_view((*shared_contacts)->contact_list.to_json()),
        uWS::OpCode::TEXT, false);
    (*shared_contacts)->contacts_updated = false;
  }
}

void run_webserver(SharedContactList *contacts, int port) {
  std::cout << "Starting webserver..." << std::endl;

  struct PerSocketData {};
  uWS::App app =
      uWS::App()
          .ws<PerSocketData>(
              "/*",
              {
                  .compression = uWS::SHARED_COMPRESSOR,
                  .maxPayloadLength = 16 * 1024 * 1024,
                  .idleTimeout = 16,
                  .maxBackpressure = 1 * 1024 * 1024,
                  .closeOnBackpressureLimit = false,
                  .resetIdleTimeoutOnSend = false,
                  .sendPingsAutomatically = true,
                  .open = [](auto *ws) { ws->subscribe("broadcast"); },
              })
          .listen(port, [port](auto *listen_socket) {
            if (listen_socket) {
              std::cout << "Listening on port " << port << std::endl;
            } else {
              std::cout << "Failed to listen on port " << port << std::endl;
            }
          });

  struct us_loop_t *loop = (struct us_loop_t *)uWS::Loop::get();
  struct us_timer_t *data_poll_timer =
      us_create_timer(loop, sizeof(std::mutex *), 0);

  memcpy(us_timer_ext(data_poll_timer), &contacts, sizeof(SharedContactList *));

  SharedContactList **ptr =
      reinterpret_cast<SharedContactList **>(us_timer_ext(data_poll_timer));

  us_timer_set(data_poll_timer, broadcast_callback, 100, 100);

  globalApp = &app;

  app.run();
}