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
#include <openssl/hmac.h>       // For HMAC operations
#include <openssl/sha.h>        // For SHA hashing


#define TOKEN_EXPIRY_DURATION 36000         /// Token expiry duration in seconds (10 hour)
#define SECRET_KEY "chin_tapak_dum_dum"     /// Secret key for HMAC operations
#define PIECE_SIZE 1024                     /// Piece size for file chunking (in bytes)

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

        static string generateToken(string payload);
        static string validateToken(string token);

    public:
        static pair<string, int> processArgs(int argc, char* argv[]);
        static vector<string> tokenize(string buffer, char separator);
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

class File {
    friend class Group;
    friend class Users;
    friend class Groups;

    private:
        File(string fileName, vector<string> SHA, int size, unordered_set<string> userName)
            : m_fileName(fileName)
            , m_SHA(SHA)
            , m_size(size)
            , m_userNames(userName)
        {}

        string m_fileName;
        vector<string> m_SHA;
        int m_size;
        unordered_set<string> m_userNames;
    
    public:
        File() = default;  
};

class User {
    friend class Users;
    friend class Groups;

    private:
        User(string userName, string password)
            : m_userName(userName)
            , m_password(password)
        {}

        string m_userName;
        string m_password;
        unordered_set<string> m_groups;
    
    public:
        User() = default;  
};

class Group {
    friend class Users;
    friend class Groups;

    private:
        Group(string groupName, vector<string> participants)
            : m_groupName(groupName)
            , m_participants(participants)
        {}

        string m_groupName;
        vector<string> m_participants;
        unordered_set<string> m_pendingJoins;
        unordered_map<string, File> m_files;
    
    public:
        Group() = default; 
};

class Users {
    friend class Groups;

    private:
        mutex m_usersMutex;
        static mutex m_userToIpMutex;

        unordered_map <string, User> m_users;
        static unordered_map< string, string> m_userToIp; // userName, IP, authToken

        Users() = default;
        ~Users() = default;
        Users(const Users&) = delete;
        Users& operator=(const Users&) = delete;    

    public:
        string addUser(string userName, string password);
        string loginUser(string userName, string password, string seederIpPort);
        string logoutUser(string authToken);

        static Users& getInstance() {
            static Users m_instance;
            return m_instance;
        }
};

class Groups {
    private:
        mutex m_groupsMutex;
        unordered_map<string, Group> m_groups;

        Groups() = default;
        ~Groups() = default;
        Groups(const Groups&) = delete;
        Groups& operator=(const Groups&) = delete;

    public:
        string addGroup(string groupName, string authToken);
        string joinGroup(string groupName, string authToken);
        string listRequests(string groupName, string authToken);
        string listGroups(string authToken);
        string acceptRequest(string groupName, string pendingUserName, string authToken);
        string listFiles(string groupName, string authToken);
        string uploadFile(string fileName, string groupName, string fileSize, string SHAs, string authToken);
        string downloadFile(string fileName, string groupName, string authToken);
        string stopShare(string groupName, string fileName, string authToken);
        string leaveGroup(string groupName, string authToken);
        
        static Groups& getInstance() {
            static Groups m_instance;
            return m_instance;
        }
};

class Tracker {
    private:
        string m_trackerIp;
        int m_trackerPort;
        ServerSocket m_trackerSocket;
        Users& m_users;
        Groups& m_groups;
        Logger m_logger;

        void acceptConnections();
        void handleLeecher(int leecherSocketFd);
        string executeCommand(string command);

        Tracker() = default;
        ~Tracker() = default;
        Tracker(const Tracker&) = delete;
        Tracker& operator=(const Tracker&) = delete;
        
        Tracker(string trackerIp, int trackerPort)
            : m_trackerIp(trackerIp)
            , m_trackerPort(trackerPort)
            , m_trackerSocket(ServerSocket(trackerIp, trackerPort))
            , m_users(Users::getInstance())  
            , m_groups(Groups::getInstance())
            , m_logger(Logger(trackerIp, trackerPort, "tracker"))
        {}

    public:
        void init();
        void start();
        void stop();

        static Tracker& getInstance(string trackerIp, int trackerPort) {
            static Tracker m_instance(trackerIp, trackerPort);
            return m_instance;
        }
};

extern Logger generalLogger;