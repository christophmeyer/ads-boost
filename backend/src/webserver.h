#ifndef ADSBOOST_WEBSERVER_H_
#define ADSBOOST_WEBSERVER_H_

#include <condition_variable>
#include <mutex>

#include "App.h"
#include "contact.h"

void broadcastWhenReady(uWS::SSLApp *globalApp);
void run_webserver(SharedContactList *contacts, int port);

#endif  // ADSBOOST_WEBSERVER_H_