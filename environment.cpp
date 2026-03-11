#include "environment.hpp"
#include"ast.hpp"
std::string valuetypeintarray[] = {"Int", "String", "Boolean", "Array", "Object", "Null"}; 
Environment::Environment()
    : parent(nullptr) {}

Environment::Environment(Environment* parentEnv)
    : parent(parentEnv) {}

void Environment::declareVal (std::string varname, RuntimeVal* value, bool immut){
    if(immut){
        IsConst[varname]=true;
    }
    variables[varname] = value;
};

void Environment::assignVal (std::string varname, RuntimeVal* value){
    Environment* env = resolve(varname);
    if(value->type != env->getVal(varname)->type){
        throw std::runtime_error("Type error: Cannot assign value of type " + valuetypeintarray[static_cast<int>(value->type)] + " to variable of type " + valuetypeintarray[static_cast<int>(env->getVal(varname)->type)]);
    }if(env->IsConst[varname]){
        throw std::runtime_error("Assignment error: Cannot assign to constant variable '" + varname + "'");
    }
    env->variables[varname] = value;

};

Environment* Environment::resolve(std::string varname){
    if(variables.find(varname) != variables.end()){
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
    return env->variables[varname];
}

void Environment::declareClass(std::string className, StructDecl* classDecl) {
    classes[className] = classDecl;
}

StructDecl* Environment::getClass(std::string className) {
    if (classes.find(className) != classes.end()) {
        return classes[className];
    } else if (parent != nullptr) {
        return parent->getClass(className);
    } else {
        throw std::runtime_error("Class not found: " + className);
        return nullptr;
    }
}