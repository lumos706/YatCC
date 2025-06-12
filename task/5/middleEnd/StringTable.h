#pragma once

#include<unordered_map>
#include "Variable.h"

using std::unordered_map;

struct StringTableNode
{
    // 将变量名与变量实体联系的map
    unordered_map<string, shared_ptr<Variable>> variableTable;
    // 父节点
    shared_ptr<StringTableNode> father;
    StringTableNode(shared_ptr<StringTableNode> father=nullptr): father{father}{};

    // 插入变量
    void insert(shared_ptr<Variable> variable);

    // 查找
    shared_ptr<Variable> lookUp(string name);
    static int tableNum;
    void print();
};
typedef shared_ptr<StringTableNode> StringTableNodePtr;
