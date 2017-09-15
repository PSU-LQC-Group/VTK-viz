#include <SplittableString.h>
using namespace std;

SplittableString::SplittableString(){
    deliminator=".";
    text="";
    before="";
    after="";
    first="";
    last="";
    middle="";
}

SplittableString::~SplittableString(){
}



SplittableString::SplittableString(string textIn,string delimIn){
    text=textIn;
    deliminator=delimIn;
    first="";
    middle="";
    last="";
    before="";
    after="";
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

string SplittableString::getFirst(){
    return first;
}

string SplittableString::getLast(){
    return last;
}

string SplittableString::getMiddle(){
    return middle;
}

string SplittableString::getIndex(int index){
    if (index<0) {
        index=0;
    }
    if (index>tagList.size()) {
        index=tagList.size(); 
    }
    return tagList[index];
}

void SplittableString::split(){
    string textHold;
    std::size_t found = text.find(deliminator);
    if(found!=std::string::npos){
        before=text.substr(0,found);
        after=text.substr(found+1);
    }else{
        before=text;
        after="";
    }

    textHold=text;

    found = textHold.find(deliminator);
    while(found!=std::string::npos){
        tagList.push_back(text.substr(0,found));
        textHold=text.substr(found+1);
        std::size_t found = textHold.find(deliminator);
    }
    tagList.push_back(textHold);
    

    first=text.substr(0,text.find_first_of(deliminator));
    last=text.substr(text.find_last_of(deliminator)+1);
    middle=text.substr(text.find_first_of(deliminator)+1,text.find_last_of(deliminator));

}
