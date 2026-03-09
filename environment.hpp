#pragma once
#include<map>
#include"values.hpp"
#include<string>
#include<algorithm>
#include<unordered_map>

// Forward declaration
class StructDecl;
#include "ast.hpp"

class Environment{
    public:
    Environment();
    Environment(Environment* parentEnv);
    Environment(Environment&&) = default;
    std::unordered_map<std::string,RuntimeVal* > variables;
    Environment& operator=(Environment&&) = default;
    void declareVal (std::string varname, RuntimeVal* value, bool immut=false);
    void assignVal(std::string varname, RuntimeVal* value);
    RuntimeVal* getVal(std::string varname);
    Environment* resolve(std::string varname);
    void declareClass(std::string className, StructDecl* classDecl);
    StructDecl* getClass(std::string className);
    protected:
    Environment* parent;
    std::unordered_map<std::string,bool> IsConst;
    std::unordered_map<std::string, StructDecl*> classes;
};