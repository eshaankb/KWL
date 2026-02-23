#pragma once
#include<map>
#include"values.hpp"
#include<string>
#include<memory>
#include<algorithm>
#include<unordered_map>

class Environment{
    public:
    Environment();
    Environment(Environment* parentEnv);
    Environment(const Environment&) = delete;
    Environment& operator=(const Environment&) = delete;
    Environment(Environment&&) = default;
    Environment& operator=(Environment&&) = default;
    void declareVal (std::string varname, std::unique_ptr<RuntimeVal> value, bool immut=false);
    void assignVal(std::string varname, std::unique_ptr<RuntimeVal> value);
    RuntimeVal* getVal(std::string varname);
    Environment* resolve(std::string varname);
    protected:
    Environment* parent;
    std::unordered_map<std::string,std::unique_ptr<RuntimeVal> > variables;
    std::unordered_map<std::string,bool> IsConst;
};