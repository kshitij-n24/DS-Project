#include "../headers.h"

void Seeder::init(){
    m_seederSocket.createSocket();
    m_seederSocket.setOptions();
    m_seederSocket.bindSocket();
    m_seederSocket.listenSocket();
    cout << "Seeder started listening!!\n" << flush;
    m_logger.log("Success", "Seeder started listening!!");
}

void Seeder::start(){
    thread t(&Seeder::acceptConnections, this);
    t.detach();
}

void Seeder::stop(){
    m_seederSocket.closeSocket();
}

void Seeder::acceptConnections(){
    while(true){
        try{
            int leecherSocketFd = m_seederSocket.acceptSocket();
            m_logger.log("INFO", "Connection established with FD of " + to_string(leecherSocketFd));
            thread t1(&Seeder::handleLeecher, this, leecherSocketFd);
            t1.detach();
        }
        catch(const string& e){
            m_logger.log("ERROR", e);
        }
    }
}

void Seeder::handleLeecher(int leecherSocketFd){
    while (true) {
        try{
            string receivedData = m_seederSocket.recvSocket(leecherSocketFd);

            if(receivedData == "") {
                m_logger.log("INFO", "LeecherSocket = " + to_string(leecherSocketFd) + " | Leecher closed the connection!!");
                break;
            }
            m_logger.log("COMMAND", "LeecherSocket = " + to_string(leecherSocketFd) + " | Recieved from leecher : " + receivedData);
            
            string response = "";

            try{
                response = "Success: " + executeCommand(receivedData, leecherSocketFd);
            }
            catch(const string& e){
                response = "Error: " + e;
            }
            
            m_seederSocket.sendSocket(leecherSocketFd, response);
        }
        catch(const string& e){
            m_logger.log("ERROR", "LeecherSocket = " + to_string(leecherSocketFd) + " | While handling leecher!! Error: " + e);
        }
    }
}

string Seeder::executeCommand(string command, int leecherSocketFd){
    if(command == "") throw string("Invalid command!!");
    vector <string> tokens = Utils::tokenize(command, ' ');
    
    if(tokens.size() < 1) throw string("Invalid command!!");

    if(tokens[0] == "give_piece_info"){
        if(tokens.size() != 3) throw string("Invalid arguments to give_piece_info command!!");
        
        string fileName = tokens[1];
        string groupName = tokens[2];

        lock_guard <mutex> guard_1(Files::m_fileNameToFilePathMutex);
        lock_guard <mutex> guard_2(Files::m_filePathToAvailablePiecesMutex);

        //: If {fileName, groupName} not exist, return response with empty string
        if(Files::m_fileNameToFilePath.find({fileName, groupName}) == Files::m_fileNameToFilePath.end()){
            return " ";
        }

        string filePath = Files::m_fileNameToFilePath[{fileName, groupName}];

        //: If filePath not exist remove entry from first map and return response with empty string 
        if(Files::m_filePathToAvailablePieces.find(filePath) == Files::m_filePathToAvailablePieces.end()){
            Files::m_fileNameToFilePath.erase({fileName, groupName});
            return " ";
        }

        //: Building a space separated response with available pieces
        string temp = "";
        for(auto it: Files::m_filePathToAvailablePieces[filePath]){
            temp.append(" " + to_string(it));
        }

        m_logger.log("INFO", "leecherSocket = " + to_string(leecherSocketFd) + 
            " | Sending response to leecher. Response = " + temp);
        return temp;
    }

    if(tokens[0] == "give_piece"){
        if(tokens.size() != 4) throw string("Invalid arguments to login command!!");
        
        string fileName = tokens[1];
        string groupName = tokens[2];
        int pieceNumber = stoi(tokens[3]);

        lock_guard <mutex> guard_1(Files::m_fileNameToFilePathMutex);
        lock_guard <mutex> guard_2(Files::m_filePathToAvailablePiecesMutex);

        if(Files::m_fileNameToFilePath.find({fileName, groupName}) == Files::m_fileNameToFilePath.end()){
            throw string("File not Exist!!");
        }

        string filePath = Files::m_fileNameToFilePath[{fileName, groupName}];
        if(Files::m_filePathToAvailablePieces.find(filePath) == Files::m_filePathToAvailablePieces.end()){
            throw string("Filepieces map not Exist!!");
        }

        vector <int> tempVec = Files::m_filePathToAvailablePieces[filePath];
        if(find(tempVec.begin(), tempVec.end(), pieceNumber) == tempVec.end()){
            throw string("Piece not Found!!");
        }

        int fd = open(filePath.c_str(), O_RDONLY, S_IRUSR);
        if (fd == -1) {
            throw string("Failed to open file at Seeder!!");
        }

        int pieceOffset = PIECE_SIZE * pieceNumber;
        if(lseek(fd, pieceOffset, SEEK_SET) == -1){
            throw string("Failed to Seek at seeder!!");
        }

        char buffer[PIECE_SIZE];
        int bytesRead = read(fd, buffer, PIECE_SIZE);
        if(bytesRead == -1){
            throw string("Failed to Read a piece at seeder!!");
        }

        close(fd);
        
        string pieceData(buffer, bytesRead);

        m_logger.log("INFO", "leecherSocket = " + to_string(leecherSocketFd) + 
            " | Sending pieceData to leecher");

        return pieceData;

    }

    throw string("Invalid command!!");
}


