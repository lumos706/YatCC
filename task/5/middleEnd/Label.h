#pragma once
#include <string>
#include <iostream>
#include "utils.h"

using std::string;
using std::cout;
using std::shared_ptr;

struct Label
{
    // label名
    string name;
    Label(string name="entry");
    void print();
    // 返回label名
    string getStr();
};
typedef shared_ptr<Label> LabelPtr;