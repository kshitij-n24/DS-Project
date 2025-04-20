#pragma once
#include "headers.h"
#include "RegistryManager.h"
#include "ElectionManager.h"
#include <thread>
#include <atomic>

class SDM {
public:
    explicit SDM(const SDMConfig& cfg = SDMConfig());
    ~SDM();

    // Starts listener & monitor threads
    void start();

    // Signals shutdown, joins threads
    void shutdown();

private:
    void runRegistrationServer();
    void handleClient(int clientSock);
    void monitorHeartbeats();
    void notifyRoleChange();

    SDMConfig           config_;
    RegistryManager     regMgr_;
    ElectionManager     electMgr_;
    std::thread         regThread_, hbThread_;
    std::atomic<bool>   stopFlag_{false};
};
