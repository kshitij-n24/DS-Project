#include "../headers.h"

//: Declaration
// class ClientSocket {
//     private:
//         string m_serverIp;
//         int m_serverPort{-1};
//         int m_socketFd{-1};

//     public:
//         ClientSocket() = default;

//         void createSocket();
//         void setOptions();
//         void connectSocket(string serverIp, int serverPort);
//         void sendSocket(string response);
//         string recvSocket();
//         void closeSocket();
// };

void ClientSocket::createSocket(){
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

void ClientSocket::connectSocket(string serverIp, int serverPort){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    //: An Internet Address (ClientSocket Address) is represented by sockaddr_in structure
    //: Here we are prepareing server's Internet Address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);  
    //: Here we have IP address of server, If we have domain-name then first we need to convert it to IP
    //: Using getAddrInfo(string domainName, string Port/Protocol, struct addrinfo options, struct addrinfo *result)
    //: then we can use *result to get IP from struct addrinfo
    //: Here we have IP in string format so we can not use htonl
    //: Instead we use inet_pton(), converting it into uint_32t
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
        throw string("Converting IP address " + serverIp + "!!\nError:" + string(strerror(errno)));
        closeSocket();
    }
    //: Connecting to server
    if (connect(m_socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw string("Connecting to " + serverIp + ":" + to_string(serverPort) + "!!\nError: " + string(strerror(errno)));
        closeSocket();
    }
    m_serverIp = serverIp;
    m_serverPort = serverPort;
}


void ClientSocket::setOptions(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    int opt = 1;
    if (setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        closeSocket();
        throw string("At setOptions!!\nError: " + string(strerror(errno))) ;
    }

    // struct timeval timeout;      
    // timeout.tv_sec = 15;  // 15 seconds
    // timeout.tv_usec = 0; // 0 microseconds
    // setsockopt(m_socketFd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}

void ClientSocket::sendSocket(string message){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    if(m_serverIp == "" || m_serverPort == -1){
        throw string("Socket is not connected with server!! Connect it first using connectSocket(string serverIp, int serverPort)!!");
    }
    string messageLength = to_string(message.size());
    message = messageLength + " " + message;

    if(send(m_socketFd, message.c_str(), message.size(), 0) < 0){
        close(m_socketFd);
        throw string("Sending message\nError: " + string(strerror(errno)));
    }
}

string ClientSocket::recvSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    if(m_serverIp == "" || m_serverPort == -1){
        throw string("Socket is not connected with server!! Connect it first using connectSocket(string serverIp, int serverPort)!!");
    }
    int totalDataLength = -1;
    string receivedData = "";

    while(true){
        char buffer[524288];
        int bytesRead = recv(m_socketFd, buffer, sizeof(buffer), 0);
        if(bytesRead == 0) {
            throw string("Error: Server closed the connection!!");
        }
        if (bytesRead < 0) {
            close(m_socketFd);
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                throw string("Receive timeout occurred!! No data received within 15 seconds!!");
            }
            throw string("Receiving data from server!!");
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

void ClientSocket::closeSocket(){
    if(m_socketFd == -1) {
        throw string("Socket not exist!! Create socket first using createSocket()!!");
    }
    close(m_socketFd);
    m_socketFd = -1;
    m_serverIp = "";
    m_serverPort = -1;
}
