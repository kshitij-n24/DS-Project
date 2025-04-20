#include "ElectionManager.h"
#include <algorithm>
#include <iostream>

using namespace std;

void ElectionManager::rebalance(
    const list<shared_ptr<NodeInfo>>& healthy)
{
    lock_guard lk(mu_);
    auto oldL = leader_, oldD = deputy_;
    leader_.reset();
    deputy_.reset();

    if (healthy.empty()) {
        cout << "[ELECT] No healthy nodes → no leader/deputy\n";
        return;
    }

    // Copy + sort by registrationOrder
    vector<shared_ptr<NodeInfo>> v(healthy.begin(), healthy.end());
    sort(v.begin(), v.end(),
         [](auto &a, auto &b){ return a->registrationOrder < b->registrationOrder; });

    // FCFS: first = leader, second = deputy (if present)
    leader_ = v[0];
    leader_->isLeader = true;
    if (v.size() >= 2) {
        deputy_ = v[1];
        deputy_->isDeputy = true;
    }

    // clear flags on any others
    for (size_t i=2; i<v.size(); ++i) {
        v[i]->isLeader = false;
        v[i]->isDeputy = false;
    }

    if (leader_!=oldL || deputy_!=oldD) {
        cout << "[ELECT] Leader=" 
             << (leader_? leader_->id : "none")
             << ", Deputy=" 
             << (deputy_? deputy_->id : "none") << "\n";
    }
}

shared_ptr<NodeInfo> ElectionManager::getLeader() const {
    lock_guard lk(mu_); return leader_;
}

shared_ptr<NodeInfo> ElectionManager::getDeputy() const {
    lock_guard lk(mu_); return deputy_;
}
