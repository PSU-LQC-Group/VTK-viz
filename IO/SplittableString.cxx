#include <SplittableString.h>
using namespace std;

SplittableString::SplittableString(){
}

SplittableString::~SplittableString(){
}



SplittableString::SplittableString(string textIn,string delimIn){
    text=textIn;
    deliminator=delimIn;
    split();
}

void SplittableString::setDeliminator(std::string newDeliminator){
    deliminator = newDeliminator;
    /* split(text,deliminator); */
}

void SplittableString::setText(std::string newText){
    text=newText;
    /* split(text,deliminator); */
}


string SplittableString::getBefore(){
    return before;
}

string SplittableString::getAfter(){
    return after;
}

string SplittableString::getText(){
    return text;
}

void SplittableString::split(){
    std::size_t found = text.find(deliminator);
    if(found!=std::string::npos){
        before=text.substr(0,found);
        after=text.substr(found+1);
    }else{
        before=text;
        after="";
    }
}
