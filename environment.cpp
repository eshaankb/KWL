#include "environment.hpp"

std::string valuetypeintarray[] = {"Int", "String", "Boolean", "Array", "Object", "Null"}; 
Environment::Environment()
    : parent(nullptr) {}

Environment::Environment(Environment* parentEnv)
    : parent(parentEnv) {}

void Environment::declareVal (std::string varname, std::unique_ptr<RuntimeVal> value, bool immut){
    if(immut){
        IsConst[varname]=true;
    }
    variables[varname] = std::move(value);
};

void Environment::assignVal (std::string varname, std::unique_ptr<RuntimeVal> value){
    Environment* env = resolve(varname);
    if(value->type != env->getVal(varname)->type){
        throw std::runtime_error("Type error: Cannot assign value of type " + valuetypeintarray[static_cast<int>(value->type)] + " to variable of type " + valuetypeintarray[static_cast<int>(env->getVal(varname)->type)]);
    }if(env->IsConst[varname]){
        throw std::runtime_error("Assignment error: Cannot assign to constant variable '" + varname + "'");
    }
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