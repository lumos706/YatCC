#include "StringTable.h"

void StringTableNode::insert(shared_ptr<Variable> Variable)
{
    variableTable[Variable->name] = Variable;
}

shared_ptr<Variable> StringTableNode::lookUp(string name)
{
    if (variableTable.find(name) != variableTable.end())
    {
        return variableTable[name];
    }
    else
    {
        if (father)
        {
            return father->lookUp(name);
        }
        else
        {
            return nullptr;
        }
    }
}

int StringTableNode::tableNum=0;

void StringTableNode::print()
{
    std::cerr<<tableNum++<<endl;
    for(auto it=variableTable.begin();it!=variableTable.end();it++)
    {
        std::cerr<<"  "<<it->first<<": "<<it->second->getTypeStr()<<endl;
    }
    std::cerr<<endl<<endl;
}
