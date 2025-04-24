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
    if(argc != 4){
        throw string("Invalid arguments!!");
    }
    
    vector <string> temp;

    char *seederIpAndPort = argv[1];
    char *trackerInfoFileName = argv[2];
    int trackerNumber = atoi(argv[3]);

    vector <string> seederIpPortVec = tokenize(seederIpAndPort, ':');
    if((int)seederIpPortVec.size() != 2){
        throw string("Invalid format of ip:port of seeder!!");
    }
    temp.push_back(seederIpPortVec[0]);
    temp.push_back(seederIpPortVec[1]);

    if(trackerNumber <= 0) {
        throw string("Tracker number is invalid!!");
    }

    int fd = open(trackerInfoFileName, O_RDONLY);
    if(fd < 0) {
        string s = trackerInfoFileName;
        throw string("Opening " + s + " file!!");
    }
    
    char buffer[524288];
    int bytesRead = read(fd, buffer, sizeof(buffer));
    if(bytesRead <= 0){
        string s = trackerInfoFileName;
        throw string("Reading " + s + " file!!");
    }

    vector <string> ipAndPorts = tokenize(buffer, '\n');
    if((int)ipAndPorts.size() < trackerNumber) {
        throw string("IP and port of tracker number " + to_string(trackerNumber) + " is not defined in file!!");
    }

    string ipAndPort = ipAndPorts[trackerNumber-1];

    vector <string> trackerIpPortVec = tokenize(ipAndPort, ':');
    if((int)trackerIpPortVec.size() != 2){
        throw string("Invalid format of ip:port of tracker number " + to_string(trackerNumber) + "!!");
    }

    temp.push_back(trackerIpPortVec[0]);
    temp.push_back(trackerIpPortVec[1]);
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