#pragma once
#include<map>
#include<values.hpp>
#include<string>

class Environment{
    public:
    RuntimeVal declareVal (std::string varname, RuntimeVal value);
    protected:
    Environment* parent;
    std::map<std::string,RuntimeVal> variables;
    Environment(Environment* parentEnv):  parent(parentEnv) {}   

};