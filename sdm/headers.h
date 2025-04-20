#pragma once

#include <string>
#include <list>
#include <memory>
#include <mutex>
#include <chrono>
#include <atomic>

using Clock = std::chrono::steady_clock;

// ———— Configuration for SDM ————
struct SDMConfig {
    int      registrationPort   = 9000;             // TCP port to listen on
    int      heartbeatTimeoutSec= 5;                // seconds before a node is “dead”
    std::string lbConfigPath    = "loadbalancer.conf";
};

// ———— Per‑node state ————
struct NodeInfo {
    std::string               id;             // unique tracker ID
    std::string               ip;
    int                       port;
    Clock::time_point         lastPing;
    uint64_t                  registrationOrder = 0;
    bool                      isLeader  = false;
    bool                      isDeputy  = false;
};
