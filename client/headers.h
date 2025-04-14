#include <iostream>                 // For standard I/O operations
#include <string>                   // For string
#include <vector>                   // For vector
#include <map>                      // For map
#include <set>                      // For set
#include <queue>                    // For queue
#include <atomic>                   // For atomic
#include <condition_variable>       // For condition_variable
#include <thread>                   // For threads
#include <mutex>                    // For mutex
#include <functional>               // for function <void()>
#include <arpa/inet.h>              // For socket programming
#include <fcntl.h>                  // For open()
#include <unistd.h>                 // For close()
#include <sys/stat.h>               // For stat()
#include <errno.h>                  // For errno
#include <cstring>                  // For strerror
#include <openssl/hmac.h>           // For HMAC operations
#include <openssl/sha.h>            // For SHA hashing
#include <random>                   // For randomness at piece selection

#define POOL_SIZE 10
#define PIECE_SIZE 1024

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

using namespace std;

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

class ThreadPool {
    private:
        vector<thread> m_workers;
        queue<function<void()>> m_tasks;

        mutex m_queueMutex;
        condition_variable m_condition;
        condition_variable m_waitCondition;

        atomic<bool> m_stop;
        atomic<int> m_activeTasks;

        void workerThread();

    public:
        ThreadPool(size_t numThreads);
        ~ThreadPool();

        void enqueueTask(function<void()> task);
        void wait();


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

class Utils {
    friend class Leecher;
    
    private:
        Utils() = delete;
        static vector <string> findSHA(string filePath);
        static string findPieceSHA(string pieceData);
        static int giveFileSize(string filePath);

    public:
        static vector <string> processArgs(int argc, char *argv[]);
        static vector <string> tokenize(string buffer, char separator);
};

class Files {
    private:
        friend class Leecher;
        friend class Seeder;

        static mutex m_fileNameToFilePathMutex;
        static mutex m_filePathToAvailablePiecesMutex;

        static map <pair<string, string>, string> m_fileNameToFilePath; // fileName, groupName, filePath
        static map <string, vector<int>> m_filePathToAvailablePieces;

        static void addFilepath(string fileName, string groupName, string filePath);
        static void addPieceToFilepath(string filePath, int pieceNumber);
        static string giveFilePath(string fileName, string groupName);
        static string giveAvailablePieces(string filePath);
        static bool isPieceAvailable(string filePath, int pieceNumber);

    public:
        Files() = default;
        
};

class Leecher {
    private:
        mutex m_downloadFileMutex;

        string m_authToken{"NULL"};
        string m_seederIp;
        int m_seederPort;

        ClientSocket m_clientSocket;
        Logger m_logger;

        set <pair<string, string>> m_downloadingFiles; // groupId, fileName
        set <pair<string, string>> m_downloadedFiles; // groupId, fileName
        set <pair<string, string>> m_downloadFailFiles; // groupId, fileName

        void getCommand();
        void processUserRequests(string inputFromClient);
        string sendTracker(string messageForTracker);
        void checkForError(string response);
        void printResponse(vector <string> tokens, string response);
        void quit(vector <string> tokens, string response);
        void createUser(vector <string> tokens, string inputFromClient);
        void login(vector <string> tokens, string inputFromClient);
        void createGroup(vector <string> tokens, string inputFromClient);
        void joinGroup(vector <string> tokens, string inputFromClient);
        void leaveGroup(vector <string> tokens, string inputFromClient);
        void listRequests(vector <string> tokens, string inputFromClient);
        void acceptRequest(vector <string> tokens, string inputFromClient);
        void listGroups(vector <string> tokens, string inputFromClient);
        void listFiles(vector <string> tokens, string inputFromClient);
        void uploadFile(vector <string> tokens, string inputFromClient);
        void downloadFile(vector <string> tokens, string inputFromClient);
        void showDownloads(vector <string> tokens, string inputFromClient);
        void logout(vector <string> tokens, string inputFromClient);
        void stopShare(vector <string> tokens, string inputFromClient);
        void downloadFileThread(string fileName, string groupName, string destinationPath, int fileSize, vector <string> SHAs, unordered_map <int, vector <string> > pieceToSeeders);

        Leecher() = default;
        ~Leecher() = default;
        Leecher(const Leecher&) = delete;
        Leecher& operator=(const Leecher&) = delete;

        Leecher(string seederIp, int seederPort)
            : m_seederIp(seederIp)
            , m_seederPort(seederPort)
            , m_logger(Logger(seederIp, seederPort, "leecher"))
        {}

    public:
        void init();
        void connectTracker(string trackerIp, int trackerPort);
        void start();
        void stop();

        static Leecher& getInstance(string seederIp, int seederPort) {
            static Leecher m_instance(seederIp, seederPort);
            return m_instance;
        }
};

class Seeder{
    private:
        string m_seederIp;
        int m_seederPort;
        ServerSocket m_seederSocket;

        Logger m_logger;

        void acceptConnections();
        void handleLeecher(int leecherSocketFd);
        string executeCommand(string command, int leecherSocketFd);

        Seeder() = default;
        ~Seeder() = default;
        Seeder(const Seeder&) = delete;
        Seeder& operator=(const Seeder&) = delete;

        Seeder(string seederIp, int seederPort)
            : m_seederIp(seederIp)
            , m_seederPort(seederPort)
            , m_seederSocket(ServerSocket(seederIp, seederPort))
            , m_logger(Logger(seederIp, seederPort, "seeder"))
        {}

    public:
        void init();
        void start();
        void stop();

        static Seeder& getInstance(string seederIp, int seederPort) {
            static Seeder m_instance(seederIp, seederPort);
            return m_instance;
        }
};

extern Logger generalLogger;
