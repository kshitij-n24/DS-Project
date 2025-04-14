#include "../headers.h"

mutex Files::m_fileNameToFilePathMutex;
mutex Files::m_filePathToAvailablePiecesMutex;
map<pair<string, string>, string> Files::m_fileNameToFilePath;
map<string, vector<int>> Files::m_filePathToAvailablePieces;

void Files::addFilepath(string fileName, string groupName, string filePath){
    lock_guard <mutex> guard(Files::m_fileNameToFilePathMutex);
    m_fileNameToFilePath[{fileName, groupName}] = filePath;
}

void Files::addPieceToFilepath(string filePath, int pieceNumber){
    lock_guard <mutex> guard(Files::m_filePathToAvailablePiecesMutex);
    m_filePathToAvailablePieces[filePath].push_back(pieceNumber);
}

string Files::giveFilePath(string fileName, string groupName){
    lock_guard <mutex> guard(Files::m_fileNameToFilePathMutex);
    if(m_fileNameToFilePath.find({fileName, groupName}) == m_fileNameToFilePath.end()){
        return "";
    }
    return m_fileNameToFilePath[{fileName, groupName}];
}

string Files::giveAvailablePieces(string filePath){
    lock_guard <mutex> guard(Files::m_filePathToAvailablePiecesMutex);
    if(m_filePathToAvailablePieces.find(filePath) == m_filePathToAvailablePieces.end()){
        return "";
    }

    string temp = "";
    for(auto it: m_filePathToAvailablePieces[filePath]){
        temp.append(" " + to_string(it));
    }
    return temp;
}

bool Files::isPieceAvailable(string filePath, int pieceNumber){
    vector <int> tempVec = m_filePathToAvailablePieces[filePath];
    if(find(tempVec.begin(), tempVec.end(), pieceNumber) == tempVec.end()){
        return false;
    }
    return true;
}