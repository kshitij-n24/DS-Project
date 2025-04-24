#include "headers.h"

Logger generalLogger;

void handleQuitOfService(SDMService &sdm_service);

int main(int argc, char* argv[]){
    try{
        vector<string> IpPortVec = Utils::processArgs(argc, argv);
        
        string sdmIp = IpPortVec[0];
        int sdmPort = stoi(IpPortVec[1]);
        string lbIp = IpPortVec[2];
        int lbPort = stoi(IpPortVec[3]);

        generalLogger = Logger(sdmIp, sdmPort, "general");

        generalLogger.log("INFO", "Creating SDM!!");
        
        // SDM sdm_service(sdmIp, sdmPort);
        SDM& sdm_service = SDM::getInstance(sdmIp, sdmPort);
        generalLogger.log("INFO", "SDM created successfully!!");   

        sdm_service.init(lbIp, lbPort);
        sdm_service.start();
        generalLogger.log("INFO", "SDM started accepting connections!!");

        thread t(handleQuitOfService, ref(sdm_service));
        t.detach();

        while(1);
    }
    catch(const string& e){
        generalLogger.log("ERROR", "Creating tracker!! Error: " + e);
        cout << "Error: " + e + "\n" << flush;
        exit(1);
    }

    return 0;
}

void handleQuitOfService(SDM &sdm_service){
    try{
        while(1){
            string s;
            cin >> s;
            if(s == "quit" || s == "exit"){
                sdm_service.stop();
                generalLogger.log("INFO" , "SDM quit.");
                exit(0);
            }
        }
    }
    catch(const string& e){
        cout << string(RED) + "Error: " + e + "\n" + string(RESET) << flush;
        exit(1);
    }
}
