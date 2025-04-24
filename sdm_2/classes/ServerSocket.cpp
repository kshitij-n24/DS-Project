#include "../headers.h"

//: Declaration
// class ServerSocket {
//     private:
//         string m_serverIp;
//         int m_serverPort;
//         int m_socketFd{-1};

//     public:
//         ServerSocket() = default;
//         ServerSocket(string serverIp, int serverPort) 
//             : m_serverIp(serverIp)
//             , m_serverPort(serverPort) 
//         {}

//         void createSocket();
//         void setOptions();
//         void bindSocket();
//         void listenSocket();
//         int acceptSocket();
//         void sendSocket(int clientSocketFd, string response);
//         string recvSocket(int clientSocketFd);
//         void closeSocket();
// };

void ServerSocket::createSocket(){
    //: Much like everything in UNIX sockets are treated as file
    //: To create a socket, we need <sys/socket.h> header file
    int domainAddressFormat = AF_INET; //: Address Family (IPv4)
    int type = SOCK_STREAM; //: TCP
    int protocol = 0; //: TCP/IP
    
    //: socket() return file descriptor for that created socket
    m_socketFd = socket(domainAddressFormat, type, protocol);
    if(m_socketFd < 0){
        throw string("Creating a socket!!\nError: " + string(strerror(errno))) ;
    }

}

void ServerSocket::setOptions(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    int opt = 1;
    if (setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        closeSocket();
        throw string("At setOptions!!\nError: " + string(strerror(errno))) ;
    }
}

void ServerSocket::bindSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    //: Network Protocol uses Big-Endian Format for communication
    //: But different systems can have different fotmat specifications
    //: To get Big-Endian Format at the end, not depending on which particular Format you system is using
    //: We have some functions in <arpa/inet.h> library
    //: uint_32t htonl(uint32_t hostint32) // Translate 4 Byte int to network format (host to network long)
    //: uint_16t htons(uint16_t hostint16) // Translate 2 Byte int to network format (host to network short)
    //: uint_32t ntohl(uint32_t hostint32) // Translate 4 Byte int to host format (host to network long)
    //: uint_16t ntohs(uint16_t hostint16) // Translate 2 Byte int to host format (host to network short)

    //: An Internet Address (ServerSocket Address) is represented by following structure
    //: struct sockaddr_in {
    //:     sa_family_t sin_family;
    //:     in_port_t sin_port;
    //:     struct in_addr sin_addr;
    //: }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; //: Address Family (IPv4)
    serverAddr.sin_port = htons(m_serverPort); //: Port
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //: IPv4 Address of current computer 
    //: In above line we can set IP address also using inet_pton() function

    //: Binding socket with specific m_port
    //: If not binded it can associate the socket to any m_port
    if (bind(m_socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        closeSocket();
        throw string("Binding socket!!\nError: " + string(strerror(errno)));
    }

}

void ServerSocket::listenSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    //: listen returns fd
    //: We listen on a socket that has been bound with bind
    //: At a time we are allowing 100 connections waiting to be accepted in queue
    //: If queue is full the server will reject additional requests
    if (listen(m_socketFd, 5000) < 0) {
        closeSocket();
        throw string("Listening socket!!\nError: " + string(strerror(errno)));
    }
}

int ServerSocket::acceptSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
        
    //: It accepts incoming connection requests from queue
    int clientSocket = accept(m_socketFd, (struct sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket < 0) {
        close(clientSocket);
        throw string("Accepting connection!!\nError:" + string(strerror(errno)));
    }
    return clientSocket;   
}

void ServerSocket::closeSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    close(m_socketFd);
    m_socketFd = -1;
    m_serverIp = "";
    m_serverPort = -1;
}

void ServerSocket::sendSocket(int clientSocketFd, string response){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    string messageLength = to_string(response.size());
    response = messageLength + " " + response;

    if(send(clientSocketFd, response.c_str(), response.size(), 0) < 0){
        close(clientSocketFd);
        throw string("Sending message to client at fd " + to_string(clientSocketFd) + "!!\nError: " + string(strerror(errno)));
    }
}

string ServerSocket::recvSocket(int clientSocketFd){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    int totalDataLength = -1;
    string receivedData = "";

    while(true){
        char buffer[524288];
        int bytesRead = recv(clientSocketFd, buffer, sizeof(buffer), 0);

        if(bytesRead == 0) return "";

        if (bytesRead < 0) {
            close(clientSocketFd);
            throw string("Error recieving message from client-socket at fd " + to_string(clientSocketFd) + "!! Connection closed forcefully!!");
        }

        receivedData += string(buffer, bytesRead);

        
        if(totalDataLength == -1) {
            vector<string> temp = Utils::tokenize(receivedData, ' ');
            totalDataLength = stoi(temp[0]) - (bytesRead - (temp[0].size() + 1));
            receivedData = receivedData.substr(temp[0].size() + 1);
        }
        else {
            totalDataLength -= bytesRead;
        }

        if(totalDataLength == 0) {
            break;
        }
    }

    return receivedData;
}

