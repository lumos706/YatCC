#include"Label.h"

Label::Label(string name): name(name)
{
}

void Label::print(){
    cout<<name<<":";//<<endl;
}

string Label::getStr()
{
    return "label %"+name;
}
