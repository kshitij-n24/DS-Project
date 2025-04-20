#include "RegistryManager.h"
#include <algorithm>
#include <iostream>

using namespace std;

void RegistryManager::handleJoin(
    const string& id,
    const string& ip,
    int           port,
    const vector<pair<string,int>>& peers)
{
    lock_guard lk(mu_);

    // 1) Merge in any peers the tracker knows about
    for (auto& p : peers) {
        auto it = find_if(nodes_.begin(), nodes_.end(),
            [&](auto& n){ return n->ip==p.first && n->port==p.second; });
        if (it == nodes_.end()) {
            auto node = make_shared<NodeInfo>();
            node->id                = p.first + ":" + to_string(p.second);
            node->ip                = p.first;
            node->port              = p.second;
            node->lastPing          = Clock::now();
            node->registrationOrder = nextOrder_++;
            nodes_.push_back(node);
            cout << "[JOIN] Added peer " << node->id << "\n";
        }
    }

    // 2) Register (or refresh) the joining node itself
    auto it = find_if(nodes_.begin(), nodes_.end(),
        [&](auto& n){ return n->id == id; });
    if (it == nodes_.end()) {
        auto node = make_shared<NodeInfo>();
        node->id                = id;
        node->ip                = ip;
        node->port              = port;
        node->lastPing          = Clock::now();
        node->registrationOrder = nextOrder_++;
        nodes_.push_back(node);
        cout << "[JOIN] Node " << id << " at " << ip << ":" << port << "\n";
    } else {
        // just update its timestamp & endpoint
        (*it)->ip       = ip;
        (*it)->port     = port;
        (*it)->lastPing = Clock::now();
        cout << "[JOIN] Node " << id << " refreshed\n";
    }
}

void RegistryManager::handleHeartbeat(const string& id) {
    lock_guard lk(mu_);
    auto it = find_if(nodes_.begin(), nodes_.end(),
        [&](auto& n){ return n->id == id; });
    if (it != nodes_.end()) {
        (*it)->lastPing = Clock::now();
        cout << "[PING] " << id << "\n";
    } else {
        cerr << "[WARN] Heartbeat from unknown node " << id << "\n";
    }
}

list<shared_ptr<NodeInfo>> RegistryManager::getHealthyMembers() const {
    lock_guard lk(mu_);
    list<shared_ptr<NodeInfo>> healthy;
    auto now = Clock::now();
    for (auto& n : nodes_) {
        auto delta = chrono::duration_cast<chrono::seconds>(now - n->lastPing).count();
        if (delta < SDMConfig().heartbeatTimeoutSec) {
            healthy.push_back(n);
        }
    }
    return healthy;
}

void RegistryManager::removeNode(const string& id) {
    lock_guard lk(mu_);
    nodes_.remove_if([&](auto& n){ return n->id == id; });
    cout << "[SDM] Removed stale node " << id << "\n";
}
