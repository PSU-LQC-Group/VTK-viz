#include <FreeFormatParser.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
FreeFormatParser::FreeFormatParser(){
}

FreeFormatParser::FreeFormatParser(string name){
    filename=name;
}

FreeFormatParser::~FreeFormatParser(){
}

void FreeFormatParser::setFilename(string name){
    filename=name;
}

vector<string> FreeFormatParser::getFirstLevel(string firstLevelName){
    vector<string> value;
    transform(firstLevelName.begin(),firstLevelName.end(),firstLevelName.begin(),::toupper);
    for (int i = 0; i < firstLevelLength; i++) {
        /* cout << "compare fisrt " << firstLevel[i][0] << firstLevelName <<endl; */
        if(firstLevel[i][0]==firstLevelName){
            /* cout << "Inside the correct part" << endl; */
            value.push_back(firstLevel[i][1]);
            /* break; */
        }
    }
    return value;
}

bool FreeFormatParser::firstKeyExist(string firstLevelName){
    bool value=false;
    for (int i = 0; i < firstLevelLength; i++) {
        /* cout << "compare fisrt " << firstLevel[i][0] << firstLevelName <<endl; */
        if(firstLevel[i][0]==firstLevelName){
            value=true;
            break;
        }
    }

    return value;
}

vector<string> FreeFormatParser::getSecondLevel(string firstLevelName,string secondLevelName){
    vector<string> value;
    transform(secondLevelName.begin(),secondLevelName.end(),secondLevelName.begin(),::toupper);
    for (int i = 0; i < secondLevelLength; i++) {
        /* cout << "compare second" << secondLevel[i][0] << secondLevel[i][1] << firstLevelName << secondLevelName <<endl; */
        if(secondLevel[i][0]==firstLevelName && secondLevel[i][1]==secondLevelName){
            /* cout << "Inside the correct part" << secondLevel[i][2] << endl; */
            value.push_back(secondLevel[i][2]);
            /* break; */
        }
    }
    return value;

}

bool FreeFormatParser::secondKeyExist(string firstLevelName,string secondLevelName){
    bool value=false,first=false,second=false;

    for (int i = 0; i < firstLevelLength; i++) {
        /* cout << "compare second" << i << " "<<secondLevel[i][0] << secondLevel[i][1] << firstLevelName << secondLevelName <<endl; */
        if(secondLevel[i][0]==firstLevelName){
            first=true;
            break;
        }
    }


    for (int i = 0; i < secondLevelLength; i++) {
        if(secondLevel[i][0]==firstLevelName && secondLevel[i][1]==secondLevelName){
            second=true;
            break;
        }
    }
    value = first && second;
    return value;

}

bool FreeFormatParser::parse(){
    string line;
    string firstId,secondId,value,comment;
    ifstream freeFile(filename,std::ifstream::in);
    if(freeFile){
        firstLevelLength=0;
        secondLevelLength=0;
        while(getline(freeFile,line)){
            FreeFormatOneLine lineHold(line);
            lineHold.split();
            firstId=lineHold.getFirstLevelIdentifier();
            secondId=lineHold.getSecondLevelIdentifier();
            value=lineHold.getValues();
            comment=lineHold.getComment();
            /* cout << "List"<<firstId << secondId << value << comment<< endl; */
            if(secondId=="" && firstId!=""){

            /* std::cout << "FirstLevel" << line <<std::endl; */
                transform(firstId.begin(),firstId.end(),firstId.begin(),::toupper);
                vector<string> first;
                first.resize(3);
                first[0]=firstId;
                first[1]=value;
                first[2]=comment;
                firstLevel.push_back(first);
                firstLevelLength++;
            }else if(firstId!="" && secondId!=""){
            /* std::cout << "SecondLevel" << line <<std::endl; */
                transform(secondId.begin(),secondId.end(),secondId.begin(),::toupper);
                vector<string> second;
                second.resize(4);
                second[0]=firstId;
                second[1]=secondId;
                second[2]=value;
                second[3]=comment;
                secondLevel.push_back(second);
                secondLevelLength++;
            }
        }
        freeFile.close();
        return true;
    }else{
        return false;
        /* std::cout << filename << " does not exist" << std::endl; */
    }
}
