#include "SDM.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

SDM::SDM(const SDMConfig& cfg)
  : config_(cfg)
{}

SDM::~SDM() {
  shutdown();
}

void SDM::start() {
  stopFlag_ = false;
  regThread_ = thread(&SDM::runRegistrationServer, this);
  hbThread_  = thread(&SDM::monitorHeartbeats, this);
}

void SDM::shutdown() {
  if (!stopFlag_) {
    stopFlag_ = true;
    if (regThread_.joinable()) regThread_.join();
    if (hbThread_.joinable())  hbThread_.join();
    // Optionally notify LB of final snapshot here
  }
}

void SDM::runRegistrationServer() {
  int servSock = socket(AF_INET, SOCK_STREAM, 0);
  if (servSock < 0) { perror("socket"); return; }
  int opt = 1;
  setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(config_.registrationPort);

  if (bind(servSock, (sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind"); close(servSock); return;
  }
  if (listen(servSock, 16) < 0) {
    perror("listen"); close(servSock); return;
  }
  cout << "[SDM] Listening on port " << config_.registrationPort << "\n";

  while (!stopFlag_) {
    sockaddr_in cliAddr;
    socklen_t len = sizeof(cliAddr);
    int clientSock = accept(servSock, (sockaddr*)&cliAddr, &len);
    if (clientSock < 0) {
      if (stopFlag_) break;
      perror("accept");
      continue;
    }
    // Handle each connection in its own detached thread
    thread(&SDM::handleClient, this, clientSock).detach();
  }
  close(servSock);
}

void SDM::handleClient(int clientSock) {
  constexpr int BUF = 2048;
  char buffer[BUF];
  int  n = recv(clientSock, buffer, BUF-1, 0);
  if (n <= 0) { close(clientSock); return; }
  buffer[n] = '\0';

  istringstream iss(buffer);
  string cmd; iss >> cmd;

  if (cmd == "JOIN") {
    string        id, ip;
    int           port, peerCount;
    iss >> id >> ip >> port >> peerCount;
    vector<pair<string,int>> peers;
    for (int i = 0; i < peerCount; i++) {
      string pip; int pport;
      iss >> pip >> pport;
      peers.emplace_back(pip, pport);
    }
    regMgr_.handleJoin(id, ip, port, peers);
  }
  else if (cmd == "PING") {
    string id; iss >> id;
    regMgr_.handleHeartbeat(id);
  }
  else {
    cerr << "[SDM] Unknown command: " << cmd << "\n";
  }

  // Re‑elect and notify everyone
  auto healthy = regMgr_.getHealthyMembers();
  electMgr_.rebalance(healthy);
  notifyRoleChange();

  close(clientSock);
}

void SDM::monitorHeartbeats() {
  using namespace std::chrono_literals;
  while (!stopFlag_) {
    std::this_thread::sleep_for(1s);

    // Remove stale nodes
    auto healthy = regMgr_.getHealthyMembers();
    auto now = Clock::now();
    for (auto& node : healthy) {
      auto delta = chrono::duration_cast<chrono::seconds>(now - node->lastPing).count();
      if (delta >= config_.heartbeatTimeoutSec) {
        regMgr_.removeNode(node->id);
      }
    }

    // Re‑elect & notify
    healthy = regMgr_.getHealthyMembers();
    electMgr_.rebalance(healthy);
    notifyRoleChange();
  }
}

// Simple TCP notifier
static void sendNotification(const string& ip, int port, const string& msg) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return;
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
  addr.sin_port = htons(port);
  if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
    send(sock, msg.c_str(), msg.size(), MSG_NOSIGNAL);
  }
  close(sock);
}

void SDM::notifyRoleChange() {
  auto L = electMgr_.getLeader();
  auto D = electMgr_.getDeputy();

  ostringstream ss;
  ss << "ROLE_CHANGE ";
  ss << (L ? L->id : "none") << " ";
  ss << (D ? D->id : "none") << "\n";
  string msg = ss.str();

  for (auto& node : regMgr_.getHealthyMembers()) {
    sendNotification(node->ip, node->port, msg);
  }
}
