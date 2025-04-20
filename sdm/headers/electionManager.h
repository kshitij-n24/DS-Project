#pragma once
#include "headers.h"
#include <vector>

class ElectionManager {
public:
    ElectionManager() = default;

    // Recompute leader & deputy (FCFS on registrationOrder)
    void rebalance(const std::list<std::shared_ptr<NodeInfo>>& healthy);

    std::shared_ptr<NodeInfo> getLeader() const;
    std::shared_ptr<NodeInfo> getDeputy() const;

private:
    mutable std::mutex                      mu_;
    std::shared_ptr<NodeInfo>               leader_, deputy_;
};
