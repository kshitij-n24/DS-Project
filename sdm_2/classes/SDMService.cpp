#include "../headers.h"

void SDMService::init(string lbIp, int lbPort){
    m_sdmTrackerSocket.createSocket();
    m_sdmTrackerSocket.setOptions();
    m_sdmTrackerSocket.bindSocket();
    m_sdmTrackerSocket.listenSocket();

    m_ClientSocket.createSocket();
    m_ClientSocket.setOptions();
    m_ClientSocket.connectSocket(lbIp, lbPort);

    cout << "Tracker started listening!!\n" << flush;
    m_logger.log("Success", "Tracker started listening!!");
}

void SDMService::start(){
    thread t1(&SDMService::acceptConnections, this);
    t1.detach();
    
    thread t2(&SDMService::cleanupStaleTrackers, this)
    t2.detach();
}

void SDMService::stop(){
    m_sdmTrackerSocket.closeSocket();
}

void SDMService::acceptConnections(){
    while(true){
        try{
            int clientFd = m_sdmTrackerSocket.acceptSocket();
            cout << "Connection established with FD of " + to_string(clientFd) + "\n" << flush;
            m_logger.log("INFO", "Connection established with FD of " + to_string(clientFd));
            
            thread t1(&SDMService::handleConnection, this, clientFd);
            t1.detach();
        }
        catch(const string& e){
            m_logger.log("ERROR", e);
        }
    }
}

void SDMService::handleConnection(int clientFd){
    while (true) {
        try{
            string receivedData = m_sdmTrackerSocket.recvSocket(clientFd);

            if(receivedData == "") {
                m_logger.log("INFO", "FD = " + to_string(clientFd) + " | lb/tracker closed the connection!!");
                break;
            }
            m_logger.log("COMMAND", "FD = " + to_string(clientFd) + " | Recieved from lb/tracker : " + receivedData);
            
            ExecResult result("", ConnAction::CLOSE);
            try{
                result = executeCommand(req, clientFd);
            }
            catch(const ExecResult& e){
                m_logger.log("ERROR", "cmd error: " + e.response);
                result = e;
            }
            
            m_sdmTrackerSocket.sendSocket(clientFd, result.response);

            if (result.action() == ConnAction::CLOSE) {
                close(clientFd);
                break;
            }  
        }
        catch(const ExecResult& e){
            m_logger.log("ERROR", "FD = " + to_string(clientFd) + " | While handling connection!! Error: " + e.response);
        }
    }
}


ExecResult executeCommand(string command, int clientFd){
    if(command == "") throw string("Invalid command!!");
    vector <string> tokens = Utils::tokenize(command, ' ');
    
    if(tokens.size() < 1) throw string("Invalid command!!");

    if(tokens[0] == "register_tracker"){
        if(tokens.size() != 3) throw string("Invalid arguments to register_tracker command!!");
        
        string trackerIp = tokens[1];
        string trackerPort = tokens[2];

        string resp = registerTracker(trackerIp, trackerPort);
        return ExecResult(resp, ConnAction::CLOSE);
    }

    if (tokens[0] == "heartbeat") {
        if(tokens.size() != 2) throw string("Invalid arguments to heartbeat command!!");

        string trackerIpPort = tokens[1];

        monitorHeartbeats(trackerIpPort);
        return ExecResult("ACK", ConnAction::KEEP_OPEN);
    }

    
    throw ExecResult("ACK", ConnAction::CLOSE);
}

void SDMService::electTrackers() {
        lock_guard<mutex> lock(m_trackerMutex);

    bool replicaStillThere = false;
    for (auto &t : m_trackerList) {
        if (t == m_replicaTracker) {
            replicaStillThere = true;
            break;
        }
    }

    if (!replicaStillThere) {
        if (!m_trackerList.empty()) {
            m_replicaTracker = m_trackerList[0];
        } else {
            m_replicaTracker = {"", 0};
        }
    }

    bool leaderStillValid = false;
    for (auto &t : m_trackerList) {
        if (t == m_leaderTracker && t != m_replicaTracker) {
            leaderStillValid = true;
            break;
        }
    }
    if (!leaderStillValid) {
        m_leaderTracker = {"", 0};
        for (auto &t : m_trackerList) {
            if (t != m_replicaTracker) {
                m_leaderTracker = t;
                break;
            }
        }
    }
}


string SDMService::registerTracker(const string& trackerIp, const string& trackerPort){
    lock_guard<mutex> lock(m_trackerMutex);
    int port = stoi(trackerPort);
    for (auto& t : m_trackerList) {
        if (t.first == trackerIp && t.second == port)
            return "ERROR: Tracker already registered";
    }
    m_trackerList.emplace_back(trackerIp, port);
    m_lastHeartbeat[ip+":"+portStr] = chrono::steady_clock::now();
    m_logger.log("INFO", "Registered tracker " + trackerIp + ":" + trackerPort);

    electTrackers();

    bool isLeader  = (ip==m_leaderTracker.first && port==m_leaderTracker.second);
    bool isReplica = (ip==m_replicaTracker.first && port==m_replicaTracker.second);
    string leaderAddr  = m_leaderTracker.first.empty() ? "none" : m_leaderTracker.first+":"+to_string(m_leaderTracker.second);
    string replicaAddr = m_replicaTracker.first.empty() ? "none" : m_replicaTracker.first+":"+to_string(m_replicaTracker.second);

    // “1” or “0” for the two bools
    string resp = (amLeader ? "1" : "0") + " " + leaderAddr + " " + (amReplica ? "1" : "0") + " " + replicaAddr;

    updateLoadBalancer();  // send updated list to LB
    return resp;
}

void SDMService::monitorHeartbeats(string trackerIpPort){
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(m_lastHeartbeatMutex);
    m_lastHeartbeat[trackerIpPort] = now;

    m_logger.log("INFO", "Heartbeat received from " + trackerIpPort);
}

string SDMService::removeTracker(const string& trackerIp, const string& trackerPort){
    lock_guard<mutex> lock(m_trackerMutex);
    int port = stoi(trackerPort);
    auto it = find_if(m_trackerList.begin(), m_trackerList.end(),
                      [&](auto& p){ return p.first == trackerIp && p.second == port; });
    if (it == m_trackerList.end())
    {
        m_logger.log("INFO", "Could not find tracker " + trackerIp + ":" + trackerPort);
        return "ERROR: Tracker not found";
    }

    m_trackerList.erase(it);
    m_logger.log("INFO", "Removed tracker " + trackerIp + ":" + trackerPort);
    updateLoadBalancer();  // send updated list to LB
    return "SUCCESS";
}

void SDMService::cleanupStaleTrackers(){
     constexpr auto TIMEOUT = chrono::seconds(30);
     while (true) {
         this_thread::sleep_for(chrono::seconds(10));
         vector<pair<string,string>> stale;

         {
             lock_guard<mutex> lock(m_trackerMutex);
             auto now = chrono::steady_clock::now();
             for (auto& t : m_trackerList) {
                 string key = t.first + ":" + to_string(t.second);
                 auto it = m_lastHeartbeat.find(key);
                 if (it == m_lastHeartbeat.end() ||
                     now - it->second > TIMEOUT)
                 {
                     stale.emplace_back(t.first, to_string(t.second));
                 }
             }
         }

         // remove stale outside the lock
         for (auto& p : stale) {
             removeTracker(p.first, p.second);
             m_logger.log("WARN", "Auto-removed stale tracker " + p.first + ":" + p.second);
         }
     }
 }

void SDMService::updateLoadBalancer(){
    string payload;
    
    payload = "update_trackers ";

    lock_guard<mutex> lock(m_trackerMutex);
    for (auto& t : m_trackerList) {
        if (t != m_replicaTracker) {
            payload += t.first + ":" + to_string(t.second) + " ";
        }
    }

    if (!payload.empty()) payload.pop_back();

    if (m_lbSocketFd != -1) {
        try {
            m_sdmLBClientSocket.sendSocket(payload);
            m_logger.log("INFO", "Pushed to tracker list to LB: [" + payload + "]");
        }
        catch (const string& e) {
            m_logger.log("WARN", "Failed to push to LB FD=" + to_string(m_lbSocketFd) + ": " + e);
        }
    }
}
