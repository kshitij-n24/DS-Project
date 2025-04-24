#include "../headers.h"

// class Utils {

//     private:
//         /// Deleted default constructor to prevent instantiation of Utils class
//         Utils() = delete;
//     public:
//         static vector <string> processArgs(int argc, char *argv[]);
//         static vector <string> tokenize(string buffer, char separator);
// };

vector <string> Utils::processArgs(int argc, char *argv[]){
    if(argc != 3){
        throw string("Invalid arguments!!");
    }
    
    vector <string> temp;

    char *sdmInfoFileName = argv[1];
    char *lbInfoFileName = argv[2];    
    
    char buffer[524288];
    int bytesRead = read(fd, buffer, sizeof(buffer));
    if(bytesRead <= 0){
        string s = sdmInfoFileName;
        throw string("Reading " + s + " file!!");
    }

    vector <string> sdmIpAndPort = tokenize(buffer, '\n');
    if((int)sdmIpAndPort.size() != 1){
        throw string("Invalid format of ip:port in SDM file !!");
    }

    vector <string> sdmIpPortVec = tokenize(sdmIpAndPort, ':');
    if((int)sdmIpPortVec.size() != 2){
        throw string("Invalid format of ip:port of SDM !!");
    }

    temp.push_back(sdmIpPortVec[0]);
    temp.push_back(sdmIpPortVec[1]);


    // For Load Balancer

    buffer[bytesRead] = '\0';
    memset(buffer, 0, sizeof(buffer));

    bytesRead = read(fd, buffer, sizeof(buffer));
    if(bytesRead <= 0){
        string s = lbInfoFileName;
        throw string("Reading " + s + " file!!");
    }

    vector <string> lbIpAndPort = tokenize(buffer, '\n');
    if((int)lbIpAndPort.size() != 1){
        throw string("Invalid format of ip:port in LB file !!");
    }

    vector <string> lbIpPortVec = tokenize(lbIpAndPort, ':');
    if((int)lbIpPortVec.size() != 2){
        throw string("Invalid format of ip:port of Load Balancer !!");
    }

    temp.push_back(lbIpPortVec[0]);
    temp.push_back(lbIpPortVec[1]);

    return temp;
}

vector <string> Utils::tokenize(string buffer, char separator){
    vector <string> ans;
    string temp;
    for(auto it: buffer){
        if(it == separator) {
            if(temp.size()) ans.push_back(temp);
            temp.clear();
        }
        else temp.push_back(it);
    }
    if(temp.size()) ans.push_back(temp);
    return ans;
}