#include "../headers.h"

void Leecher::init(){
    m_clientSocket.createSocket();
}

void Leecher::connectTracker(string trackerIp, int trackerPort){
    m_clientSocket.connectSocket(trackerIp, trackerPort);
    m_clientSocket.setOptions();
    m_logger.log("SUCCESS", "Leecher connected to tracker at " + trackerIp + ":" + to_string(trackerPort) + ".");
}

void Leecher::start(){
    thread t(&Leecher::getCommand, this);
    t.detach();
}

void Leecher::stop(){
    m_clientSocket.closeSocket();
}

void Leecher::getCommand(){
    m_logger.log("INFO", "Started getting commands!!");
    while(true){
        try{
            string inputFromClient;
            cout << ">> " << flush;
            getline(cin, inputFromClient);      
            processUserRequests(inputFromClient);
        }
        catch(const string& e){
            cout << string(RED) + "Error: " + e + "\n" + string(RESET) << flush;
        }
    }
}

void Leecher::processUserRequests(string inputFromClient){
    vector <string> tokens = Utils::tokenize(inputFromClient, ' ');
    
    if(tokens.size() == 0) return;
    else if(tokens[0] == "quit" || tokens[0] == "exit") quit(tokens, inputFromClient);
    else if(tokens[0] == "create_user") createUser(tokens, inputFromClient);
    else if(tokens[0] == "login") login(tokens, inputFromClient);
    else if(tokens[0] == "create_group") createGroup(tokens, inputFromClient);
    else if(tokens[0] == "join_group") joinGroup(tokens, inputFromClient);
    else if(tokens[0] == "leave_group") leaveGroup(tokens, inputFromClient);
    else if(tokens[0] == "list_requests") listRequests(tokens, inputFromClient);
    else if(tokens[0] == "accept_request") acceptRequest(tokens, inputFromClient);
    else if(tokens[0] == "list_groups") listGroups(tokens, inputFromClient);
    else if(tokens[0] == "list_files") listFiles(tokens, inputFromClient);
    else if(tokens[0] == "upload_file") uploadFile(tokens, inputFromClient);
    else if(tokens[0] == "download_file") downloadFile(tokens, inputFromClient);
    else if(tokens[0] == "show_downloads") showDownloads(tokens, inputFromClient);
    else if(tokens[0] == "logout") logout(tokens, inputFromClient);
    else if(tokens[0] == "stop_share") stopShare(tokens, inputFromClient);
    else throw string("Invalid command!!");
}

void Leecher::printResponse(vector <string> tokens, string response){
    if(tokens[0] == "login"){
        vector <string> responseTokens = Utils::tokenize(response, ' ');
        response = "";
        for(int i = 0; i< (int)responseTokens.size(); i++){
            if(i != 1) response.append(responseTokens[i] + " ");
        }
    }

    if(tokens[0] == "list_groups"){
        vector <string> responseTokens = Utils::tokenize(response, ' ');
        if(responseTokens.size() == 1) {
            cout << string(YELLOW) + "There is no group in the system!!\n" + string(RESET) << flush;
            return;
        }
        response = "List of groups is as follows : " + responseTokens[1];
        cout << response + "\n" << flush;
        return;
    }

    if(tokens[0] == "list_requests"){
        vector <string> responseTokens = Utils::tokenize(response, ' ');
        if(responseTokens.size() == 1) {
            cout << string(YELLOW) + "There is no pending joinee in the group!!\n" + string(RESET) << flush;
            return;
        }
        response = "List of pending requests in the group is as follows : " + responseTokens[1];
        cout << response + "\n" << flush;
        return;
    }

    if(tokens[0] == "list_files"){
        vector <string> responseTokens = Utils::tokenize(response, ' ');
        if(responseTokens.size() == 1) {
            cout << string(YELLOW) + "There is no files in the group!!\n" + string(RESET) << flush;
            return;
        }
        response = "List of files in the group is as follows : " + responseTokens[1];
        cout << response + "\n" << flush;
        return;
    }

    cout << string(GREEN) + response + "\n" + string(RESET) << flush;
}

void Leecher::checkForError(string response){
    if(response.substr(0, 5) == "Error"){
        throw response.substr(7);
    }
}

string Leecher::sendTracker(string messageForTracker){
    m_logger.log("COMMAND", "Sending to tracker : " + messageForTracker);
    
    m_clientSocket.sendSocket(messageForTracker);
    
    string response = m_clientSocket.recvSocket();
    m_logger.log("COMMAND", "Recieved from tracker : " + response);
    
    checkForError(response);
    
    return response;
}

void Leecher::quit(vector <string> tokens, string inputFromClient){
    if(m_authToken != "NULL") {
        try{
            m_logger.log("INFO", "authToken found!! Sending logout request to tracker");
            logout(tokens, "logout");
        }
        catch(const string& e){
            m_logger.log("ERROR", "Error from tracker during 'quit logout'!! Error : " + e);    
        }
    }
    else{
        m_logger.log("INFO", "authToken not found!! No need to send logout request to tracker");
    }

    stop();
    m_logger.log("SUCCESS", "Leecher Quit.");
    exit(0);
}

void Leecher::createUser(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

void Leecher::login(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient + " " + m_seederIp + ":" + to_string(m_seederPort);
    string response = sendTracker(messageForTracker);
    vector <string> responseTokens = Utils::tokenize(response, ' ');
    m_authToken = responseTokens[1];
    printResponse(tokens, response);
}

void Leecher::createGroup(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

void Leecher::joinGroup(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

void Leecher::leaveGroup(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

void Leecher::listRequests(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

void Leecher::acceptRequest(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

void Leecher::listGroups(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

void Leecher::listFiles(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

void Leecher::uploadFile(vector <string> tokens, string inputFromClient){
    if(tokens.size() != 3) {
        throw string("Invalid arguments to upload_file command!!");
    }
    
    string filePath = tokens[1];
    string groupName = tokens[2];
    
    //: Validating path of a file
    struct stat info;
    if (stat(filePath.c_str(), &info) != 0 || !(info.st_mode & S_IFREG)) {
        throw string("Filepath is invalid!!");
    }
    
    //: Extracting fileName form filePath
    vector<string> temp = Utils::tokenize(filePath, '/');
    string fileName = temp[temp.size() - 1];
    
    //: Store whole File SHA, and piecewise SHAs in one vector
    vector<string> fileSHAs = Utils::findSHA(filePath.c_str());
    
    //: Store size of a file
    int fileSize = Utils::giveFileSize(filePath.c_str());
    
    //: Builiding Command for tracker
    //: "upload_file fileName groupName fileSize (whole_file_SHA:piece_1_SHA:...:piece_N_SHA) m_authToken"
    string messageForTracker = "upload_file " + fileName + " " + groupName + " " + to_string(fileSize) + " ";
    
    for(auto it : fileSHAs) {
        messageForTracker.append(it + ":");
    }
    messageForTracker.append(" " + m_authToken);
    
    string response = sendTracker(messageForTracker);
    
    Files::addFilepath(fileName, groupName, filePath);
        
    int numOfPieces = fileSize / PIECE_SIZE;
    if(fileSize % PIECE_SIZE) numOfPieces++;

    for(int i = 0; i < numOfPieces; i++) {
        Files::addPieceToFilepath(filePath, i);
    }

    printResponse(tokens, response);
}

void Leecher::downloadFile(vector <string> tokens, string inputFromClient){
    if(tokens.size() != 4) {
        throw string("Invalid arguments to download_file command!!");
    }

    string groupName = tokens[1];
    string fileName = tokens[2];
    string destinationPath = tokens[3];

    struct stat info;
    if (stat(destinationPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        throw string("Destination path is invalid!!\nError: " + string(strerror(errno)));
    }

    if(destinationPath[destinationPath.size() - 1] != '/') {
        destinationPath += '/';
    }

    string filePath = destinationPath + fileName;

    if (stat(filePath.c_str(), &info) == 0 && (info.st_mode & S_IFREG)) {
        cout << string(YELLOW) + "Warning: File " + fileName + " already exists at destination!!\n" + string(RESET) << flush;
        while(1){
            cout << string(YELLOW) + "Do you want to overwrite it? [Y/N]: " + string(RESET) << flush;
            
            string temp;
            getline(cin, temp);
            
            if(temp == "N" || temp == "n") return;
            if(temp == "Y" || temp == "y") break;
            cout << string(RED) + "Error: Not a valid choice!!\n" + string(RESET) << flush;
        }
    }

    string messageForTracker = "download_file " + groupName + " " + fileName + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    
    vector <string> responseTokens = Utils::tokenize(response, ' ');

    int fileSize = stoi(responseTokens[1]);
    string SHAsStr = responseTokens[2];
    string ipAndPortsStr = responseTokens[3];

    vector <string> SHAs = Utils::tokenize(SHAsStr, ':');
    vector <string> ipAndPorts = Utils::tokenize(ipAndPortsStr, ',');
    int totalPieces = SHAs.size()-1;

    unordered_map <int, vector <string> > pieceToSeeders; // piece no., ip:port

    for(auto it: ipAndPorts){
        vector <string> temp = Utils::tokenize(it, ':');
        string seederIp = temp[0];
        int seederPort = stoi(temp[1]);

        m_logger.log("INFO" , "Creating Socket for connecting " + seederIp + ":" + to_string(seederPort) + " to get piece info!!");
        ClientSocket leecherSocket;
        leecherSocket.createSocket();
        leecherSocket.setOptions();
        leecherSocket.connectSocket(seederIp, seederPort);
        
        m_logger.log("INFO" , "Cennected Socket for connecting " + seederIp + ":" + to_string(seederPort) + " to get piece info!!");
        
        string messageForSeeder = "give_piece_info " + fileName + " " + groupName;
        m_logger.log("COMMAND" , "Sending Message to seeder " + seederIp + ":" + to_string(seederPort) + " Message = " + messageForSeeder);
        leecherSocket.sendSocket(messageForSeeder);

        string response = leecherSocket.recvSocket();
        m_logger.log("COMMAND" , "Recieved Message from seeder " + seederIp + ":" + to_string(seederPort) + " Message = " + response);
        
        try{
            checkForError(response);
        }
        catch(const string& e){
            m_logger.log("ERROR" , "Error from seeder " + seederIp + ":" + to_string(seederPort) + " Error : " + e);
        }

        leecherSocket.closeSocket();
        
        vector<string> responseTokens = Utils::tokenize(response, ' ');
        for(int i = 1; i < (int)responseTokens.size(); i++){
            pieceToSeeders[stoi(responseTokens[i])].push_back(it);            
        }
    }

    if((int)pieceToSeeders.size() != totalPieces) {
        m_logger.log("ERROR", "Can not download the file!! All pieces of file is not available in the group.");
        throw string("Can not download the file!! All pieces of file is not available in the group!!");
    }
    {
        lock_guard <mutex> guard(m_downloadFileMutex);
        m_logger.log("INFO", "FileName = " + fileName + " | Changing status as 'downloading'!!");
        m_downloadingFiles.insert({groupName, fileName});
    }

    thread t2(&Leecher::downloadFileThread, this, fileName, groupName, filePath, fileSize, SHAs, pieceToSeeders);
    t2.detach();

    m_logger.log("SUCCESS", "File downloading started.");

    response = "Success: file downloading started!!\n";
    printResponse(tokens, response);
}

void Leecher::showDownloads(vector <string> tokens, string inputFromClient){
    lock_guard <mutex> guard(m_downloadFileMutex);
    for(auto it: m_downloadingFiles){
        cout << "[D][" + it.first + "] " + it.second + "\n" << flush; 
    }
    for(auto it: m_downloadedFiles){
        cout << "[C][" + it.first + "] " + it.second + "\n" << flush; 
    }
    for(auto it: m_downloadFailFiles){
        cout << "[F][" + it.first + "] " + it.second + "\n" << flush; 
    }
    cout << string(YELLOW) + "D (Downloading), C (Complete), F (Failed)\n, [G] where G = Group Name\n" + string(RESET) << flush;
}

void Leecher::logout(vector <string> tokens, string inputFromClient){
    if(m_authToken == "NULL"){
        throw string("You are not logged in!!");
    }
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    m_authToken = "NULL";
    printResponse(tokens, response);
}

void Leecher::stopShare(vector <string> tokens, string inputFromClient){
    string messageForTracker = inputFromClient + " " + m_authToken;
    string response = sendTracker(messageForTracker);
    printResponse(tokens, response);
}

void Leecher::downloadFileThread(string fileName, string groupName, string filePath, int fileSize, vector <string> SHAs, unordered_map <int, vector <string> > pieceToSeeders){
    vector <pair <int, vector <string>>> pieceToSeedersVector;

    for(auto it: pieceToSeeders){
        pieceToSeedersVector.push_back({it.first, it.second});
    }

    sort(pieceToSeedersVector.begin(), pieceToSeedersVector.end(), [](const auto &a, const auto &b) {
        return a.second.size() < b.second.size();
    });

    ThreadPool pool(POOL_SIZE); 

    m_logger.log("INFO" , "FileName = " + fileName + " | Pieces sorted and thread pool is created.");
    mutex isFirstPieceMutex;
    bool isFirstPiece = true;

    for (int i = 0; i < (int)pieceToSeedersVector.size(); i++) {
        pool.enqueueTask(
            [this, i, fileName, groupName, filePath, fileSize, SHAs, pieceToSeedersVector, &isFirstPieceMutex, &isFirstPiece]
            {
                int pieceNumber = pieceToSeedersVector[i].first;
                int retry = 5; //: If any error in below code, we will retry 

                while(retry--){
                    try{
                        //: Randomly selecting one Seeder from many
                        int n = pieceToSeedersVector[i].second.size();

                        // Create a random device and a generator
                        random_device rd; 
                        mt19937 gen(rd()); // Mersenne Twister generator
                        
                        // Create a distribution in the range [0, n-1]
                        uniform_int_distribution<> dist(0, n - 1);
                        int random_number = dist(gen);
                        
                        string seederIpPort = pieceToSeedersVector[i].second[random_number];
                        vector <string> temp = Utils::tokenize(seederIpPort, ':');
                        string seederIp = temp[0];
                        int seederPort = stoi(temp[1]);

                        ClientSocket leecherSocket;
                        leecherSocket.createSocket();
                        leecherSocket.setOptions();
                        leecherSocket.connectSocket(seederIp, seederPort);

                        m_logger.log("INFO", "FileName = " + fileName + " | Piece No. = " + to_string(pieceNumber) + " | Retry = " + to_string(retry) + " | i = " + to_string(i) +  
                                " | Connected to leecher " + seederIpPort);

                        string messageForSeeder = "give_piece " + fileName + " " + groupName + " " + to_string(pieceNumber); 

                        m_logger.log("INFO", "FileName = " + fileName + " | Piece No. = " + to_string(pieceNumber) + " | Retry = " + to_string(retry) + " | i = " + to_string(i) +  
                                " | Sending to leecher : " + messageForSeeder);
                                
                        leecherSocket.sendSocket(messageForSeeder);
                        string response = leecherSocket.recvSocket();
                        
                        m_logger.log("INFO", "FileName = " + fileName + " | Piece No. = " + to_string(pieceNumber) + " | Retry = " + to_string(retry) + " | i = " + to_string(i) +  
                                " | recieved from leecher!!");
                        
                        checkForError(response);

                        leecherSocket.closeSocket();

                        vector <string> receivedDataTokens = Utils::tokenize(response, ' ');
                        if(receivedDataTokens[0] == "Error:"){
                            throw string("Error from seeder!! ERROR: " + response);
                        }
                        
                        string pieceData = response.substr(strlen("Success: "));
                        string pieceSHA = Utils::findPieceSHA(pieceData);

                        if(pieceSHA != SHAs[pieceNumber + 1]){
                            throw string("PieceSHA mismatched!!" + pieceSHA + " " + SHAs[pieceToSeedersVector[pieceNumber].first + 1]);
                        }

                        int fd = open(filePath.c_str(), O_WRONLY | O_CREAT , S_IRUSR | S_IWUSR);
                        if (fd == -1) {
                            throw string(" Opening destination file!!");
                        }

                        int pieceOffset = PIECE_SIZE * pieceToSeedersVector[i].first;
                        if(lseek(fd, pieceOffset, SEEK_SET) == -1){
                            throw string("Seeking destination file!!");
                        }

                        int written = write(fd, pieceData.c_str(), pieceData.size());

                        if (written == -1) {
                            throw string("Writing destination file!!");
                        }
                        if (written != (int)pieceData.size()) {
                            throw string("Writing destination file is incomplete!!");
                        }
                        close(fd);
                        //: ON FIRST PIECE
                        {
                            lock_guard <mutex> guard(isFirstPieceMutex);
                            if(isFirstPiece) {
                                string messageForTracker = "upload_file " + fileName + " " + groupName + " " + to_string(fileSize) + " ";
                                for(auto it : SHAs) {
                                    messageForTracker.append(it + ":");
                                }
                                messageForTracker += " " + m_authToken;
                                string response = sendTracker(messageForTracker);
                                m_logger.log("INFO", "FileName = " + fileName + " | Piece No. = " + to_string(pieceNumber) + " | Retry = " + to_string(retry) + " | i = " + to_string(i) +  
                                    " | Upload on first piece " + response);
                                Files::addFilepath(fileName, groupName, filePath);
                                isFirstPiece = false;
                            }
                        }
                            
                        pieceNumber = pieceToSeedersVector[i].first;
                        Files::addPieceToFilepath(filePath, pieceNumber);
                    }
                    catch(const string& e){
                        m_logger.log("ERROR", "FileName = " + fileName +  " | Piece No. = " + to_string(pieceNumber) + " | Retry = " + to_string(retry) +  " | i = " + to_string(i) +
                                " | " + e);
                        continue;
                    }
                    m_logger.log("INFO", "FileName = " + fileName + " | Piece No. = " + to_string(pieceNumber) + " | Retry = " + to_string(retry) +  " | i = " + to_string(i) +
                                " | Successfully downloaded!!");
                    break;    
                }
                if(retry <= 0) {
                    // TODO : stop download of entire file here
                    {
                        lock_guard <mutex> guard(m_downloadFileMutex);
                        m_logger.log("INFO", "FileName = " + fileName + " | Changing status as 'download failed'!!");
                        m_downloadingFiles.erase({groupName, fileName});
                        m_downloadFailFiles.insert({groupName, fileName});
                    }
                    m_logger.log("ERROR", "FileName = " + fileName + " | i = " + to_string(i) +  " | Piece No. = " + to_string(pieceNumber) + " | Retry = " + to_string(retry) + 
                                " | Retry exhausted at piece " + to_string(pieceNumber) + "!! File download failed!!");
                }
            }
        );
    }

    pool.wait();

    m_logger.log("INFO", "FileName = " + fileName + " | File downloaded!! checking the SHA!!");
    vector <string> tempSHAs = Utils::findSHA(filePath);
    if(tempSHAs[0] != SHAs[0]){
        m_logger.log("ERROR", "FileName = " + fileName + " | File SHA mismatched!!");

        if (unlink(filePath.c_str()) == -1) {
            m_logger.log("ERROR", "FileName = " + fileName + " | Deleting file!!");
        }
        else{
            m_logger.log("INFO", "FileName = " + fileName + " | File deleted!!");
        }
        {
            lock_guard <mutex> guard(m_downloadFileMutex);
            m_logger.log("INFO", "FileName = " + fileName + " | Changing status as 'download failed'!!");
            m_downloadingFiles.erase({groupName, fileName});
            m_downloadFailFiles.insert({groupName, fileName});
        }
        return;
    }
    {
        lock_guard <mutex> guard(m_downloadFileMutex);
        m_logger.log("INFO", "FileName = " + fileName + " | Changing status as 'downloaded'!!");
        m_downloadingFiles.erase({groupName, fileName});
        m_downloadedFiles.insert({groupName, fileName});
    }
    m_logger.log("SUCCESS", "FileName = " + fileName + " | SHA Matched!! File download successfull!!");
}