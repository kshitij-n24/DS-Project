#include "headers.h"

// This is a declaration only
// Overwriting it after getting a seederIp and seederPort
Logger generalLogger;

void startSeeder(string seederIp, int seederPort);

int main(int argc, char* argv[]){
    try{
        vector <string> ipAndPorts = Utils::processArgs(argc, argv);

        if(ipAndPorts.size() != 4) {
            cout << string(RED) + "Args processing failed!!\n" + string(RESET) << flush;
        }

        string seederIp = ipAndPorts[0];
        int seederPort = stoi(ipAndPorts[1]);
        string trackerIp = ipAndPorts[2];
        int trackerPort = stoi(ipAndPorts[3]);

        generalLogger = Logger(seederIp, seederPort, "general");

        thread t(startSeeder, seederIp, seederPort);
        t.detach();

        generalLogger.log("INFO", "Creating leecher!!");
        
        Leecher& leecher = Leecher::getInstance(seederIp, seederPort);
        generalLogger.log("INFO", "Leecher created successfully!!");   

        leecher.init();
        leecher.connectTracker(trackerIp, trackerPort);
        generalLogger.log("INFO", "Leecher connected to tracker successfully!!");
        
        leecher.start();
        generalLogger.log("INFO", "Leecher is ready for commands!!");

        while(1);
    }
    catch(const string& e){
        generalLogger.log("ERROR", "Creating leecher!! Error: " + e);
        cout << string(RED) + "Error: " + e + "\n" + string(RESET) << flush;
        exit(1);
    }
}

void startSeeder(string seederIp, int seederPort){
    try{
        generalLogger.log("INFO", "Creating seeder!!");
        
        Seeder& seeder = Seeder::getInstance(seederIp, seederPort);

        generalLogger.log("INFO", "Seeder created successfully!!");
        
        seeder.init();
        seeder.start();
        
        generalLogger.log("INFO", "Seeder started accepting connections!!");
        
        while(1);
    }
    catch(const string& e){
        generalLogger.log("ERROR", "Creating seeder!! Error: " + e);
        cout << string(RED) + "Error: " + e + "\n" + string(RESET) << flush;
        exit(1);
    }
}