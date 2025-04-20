#include "SDM.h"
#include <signal.h>
#include <iostream>
#include <unistd.h>

static SDM* g_sdm = nullptr;

void onSignal(int) {
  if (g_sdm) {
    std::cout << "\n[SDM] Signal received → shutting down...\n";
    g_sdm->shutdown();
  }
  _exit(0);
}

int main(int argc, char** argv) {
  SDMConfig cfg;
  if (argc > 1) cfg.registrationPort    = std::stoi(argv[1]);
  if (argc > 2) cfg.heartbeatTimeoutSec = std::stoi(argv[2]);

  SDM sdm(cfg);
  g_sdm = &sdm;

  signal(SIGINT,  onSignal);
  signal(SIGTERM, onSignal);

  sdm.start();
  // Block forever; shutdown on signal
  while (true) pause();

  return 0;
}
