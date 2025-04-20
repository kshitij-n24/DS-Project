#pragma once
#include "headers.h"
#include <vector>
#include <utility>

class RegistryManager {
public:
    RegistryManager() = default;

    // Called when a tracker sends:
    //   JOIN <id> <ip> <port> <peerCount> [<peerIp> <peerPort>]ₙ
    void handleJoin(const std::string& id,
                    const std::string& ip,
                    int                 port,
                    const std::vector<std::pair<std::string,int>>& peers);

    // Called when a tracker sends:
    //   PING <id>
    void handleHeartbeat(const std::string& id);

    // Returns only those nodes whose lastPing < timeout
    std::list<std::shared_ptr<NodeInfo>> getHealthyMembers() const;

    // Force‑remove a node (e.g. detected stale in monitor thread)
    void removeNode(const std::string& id);

private:
    mutable std::mutex                         mu_;
    std::list<std::shared_ptr<NodeInfo>>       nodes_;
    std::atomic<uint64_t>                      nextOrder_{1};
};
