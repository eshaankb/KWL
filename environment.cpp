#include "environment.hpp"

Environment::Environment()
    : parent(nullptr) {}

Environment::Environment(Environment* parentEnv)
    : parent(parentEnv) {}

void Environment::declareVal (std::string varname, std::unique_ptr<RuntimeVal> value){
    variables[varname] = std::move(value);
};

void Environment::assignVal (std::string varname, std::unique_ptr<RuntimeVal> value){
    Environment* env = resolve(varname);
    env->variables[varname] = std::move(value);

};

Environment* Environment::resolve(std::string varname){
    if(variables.contains(varname)){
        return this;
    }else if(parent!=nullptr){
        return parent->resolve(varname);
    }else{
        throw std::runtime_error("Variable not found: " + varname);
        return nullptr;
    }
} 

RuntimeVal* Environment::getVal(std::string varname) {
    Environment* env = resolve(varname);
    return env->variables[varname].get();
}