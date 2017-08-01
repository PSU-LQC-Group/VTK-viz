#include <string>

#ifndef SplittableString_H
#define SplittableString_H
using namespace std;
class SplittableString{
    public:
        SplittableString();
        SplittableString(std::string,std::string);
        ~SplittableString();

        void setDeliminator(std::string);
        void setText(std::string);
        std::string getBefore();
        std::string getAfter();
        std::string getText();
        void split();


    private:
        std::string deliminator=".";
        std::string text="";
        std::string before="";
        std::string after="";

};

#endif
