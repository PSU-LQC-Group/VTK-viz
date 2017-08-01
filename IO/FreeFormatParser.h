#include <string>
#include <FreeFormatOneLine.h>
#include <vector>


#ifndef FreeFormatParser_H
#define FreeFormatParser_H
using namespace std;
class FreeFormatParser{
    public:
        FreeFormatParser();
        FreeFormatParser(string);
        ~FreeFormatParser();
        void setFilename(string);
        void parse();
        string getFirstLevel(string firstLevelName);
        string getSecondLevel(string firstLevelName,string secondLevelName);

    private:
        string filename="";
        vector<vector<string> > firstLevel;
        vector<vector<string> > secondLevel;
        int firstLevelLength=0;
        int secondLevelLength=0;

};
#endif
