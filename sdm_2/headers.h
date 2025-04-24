#include <iostream>             // For standard I/O operations
#include <thread>               // For threads
#include <algorithm>            // For count
#include <string>               // For string
#include <vector>               // For vector
#include <unordered_map>        // For unordered_map
#include <unordered_set>        // For unordered_set
#include <mutex>                // For mutex
#include <arpa/inet.h>          // For socket programming
#include <fcntl.h>              // For open()
#include <unistd.h>             // For read(), write(), close()
#include <sys/stat.h>           // For stat()
#include <errno.h>              // For errno error checking
#include <cstring>              // For strerror

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

using namespace std;

class Utils {
    friend class Users;
    friend class Groups;

    private:
        Utils() = delete;

    public:
        static pair<string, int> processArgs(int argc, char* argv[]);
        static vector<string> tokenize(string buffer, char separator);
};

enum class ConnAction { CLOSE, KEEP_OPEN };

class ExecResult {
    private:
        std::string m_response;
        ConnAction m_action;

    public:
        ExecResult(const std::string& resp, ConnAction act)
          : m_response(resp), m_action(act)
        {}

        const std::string& response() const { return m_response; }
        ConnAction action() const { return m_action; }
};

class Logger{
    private: 
        mutex m_logMutex;

        string m_seederIp;
        string m_seederPort;
        string m_logDirPath;
        string m_logFilePath;

    public:
        Logger() = default;
        
        Logger(string seederIp, int seederPort, string name);

        //: Move constructor (can't move mutex, so leave it default in the moved-from object)
        Logger(Logger&& other) noexcept {
            m_seederIp = move(other.m_seederIp);
            m_seederPort = move(other.m_seederPort);
            m_logDirPath = move(other.m_logDirPath);
            m_logFilePath = move(other.m_logFilePath);
        }

        //: Move assignment operator (same as move constructor)
        Logger& operator=(Logger&& other) noexcept {
            if (this != &other) {
                m_seederIp = move(other.m_seederIp);
                m_seederPort = move(other.m_seederPort);
                m_logDirPath = move(other.m_logDirPath);
                m_logFilePath = move(other.m_logFilePath);
            }
            return *this;
        }

        void log(string type, string content);
};

class ClientSocket {
    private:
        string m_serverIp;
        int m_serverPort{-1};
        int m_socketFd{-1};

    public:
        ClientSocket() = default;

        void createSocket();
        void setOptions();
        void connectSocket(string serverIp, int serverPort);
        void sendSocket(string response);
        string recvSocket();
        void closeSocket();
};

class ServerSocket {
    private:
        string m_serverIp;
        int m_serverPort;
        int m_socketFd{-1};

    public:
        ServerSocket() = default;
        ServerSocket(string serverIp, int serverPort) 
            : m_serverIp(serverIp)
            , m_serverPort(serverPort) 
        {}

        void createSocket();
        void setOptions();
        void bindSocket();
        void listenSocket();
        int acceptSocket();
        void sendSocket(int clientSocketFd, string response);
        string recvSocket(int clientSocketFd);
        void closeSocket();
};

class SDMService {
    private:
        string m_sdmIp;
        int m_sdmPort;
        ServerSocket m_sdmSocket;
        Logger m_logger;
        vector<pair<string,int>> m_trackerList;
        unordered_map<string, steady_clock::time_point> m_lastHeartbeat;
        mutex m_trackerMutex;
        int m_lbSocketFd{-1};
        mutex m_lbMutex;
        pair<string,int> m_replicaTracker;
        pair<string,int> m_leaderTracker;

        void acceptConnections();
        ExecResult executeCommand(string command, int clientFd);
        void monitorHeartbeats();
        void electTrackers();
        string registerTracker(string trackerIp, string trackerPort);
        string removeTracker(string trackerIp, string trackerPort);
        void cleanupStaleTrackers();
        void updateLoadBalancer();


        SDMService() = default;
        ~SDMService() = default;
        SDMService(const SDMService&) = delete;
        SDMService& operator=(const SDMService&) = delete;
        
        SDMService(string sdmIp, int trackerPort)
            : m_sdmIp(sdmIp)
            , m_sdmPort(sdmPort)
            , m_sdmSocket(ServerSocket(sdmIp, sdmPort))
            , m_logger(Logger(sdmIp, sdmPort, "sdm"))
            , m_trackerList(0)
        {}

    public:
        void init();
        void start();
        void stop();

        static SDMService& getInstance(string sdmIp, int sdmPort) {
            static SDMService m_instance(sdmIp, sdmPort);
            return m_instance;
        }
};

extern Logger generalLogger;