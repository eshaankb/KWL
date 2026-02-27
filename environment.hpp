#pragma once
#include<map>
#include"values.hpp"
#include<string>
#include<algorithm>
#include<unordered_map>

class Environment{
    public:
    Environment();
    Environment(Environment* parentEnv);
    Environment(Environment&&) = default;
    Environment& operator=(Environment&&) = default;
    void declareVal (std::string varname, RuntimeVal* value, bool immut=false);
    void assignVal(std::string varname, RuntimeVal* value);
    RuntimeVal* getVal(std::string varname);
    Environment* resolve(std::string varname);
    protected:
    Environment* parent;
    std::unordered_map<std::string,RuntimeVal* > variables;
    std::unordered_map<std::string,bool> IsConst;
};