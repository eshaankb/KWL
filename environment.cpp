/**
 * @file environment.cpp
 * @brief Environment implementation for the KWL interpreter.
 *
 * Implements the Environment class methods for variable scope management,
 * including declaration, assignment, lookup, and class/structure definitions.
 *
 * @author KWL Interpreter
 * @date 2026
 */

#include "values.hpp"
#include <stdexcept>
#include "environment.hpp"
#include"ast.hpp"
std::string valuetypeintarray[] = {"Int", "Float", "String", "Function", "Bool", "Structure", "Null", "Array", "Range", "TrueClass", "Module", "Integer64", "Float64"}; 
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
    ValueType targetType = env->getVal(varname)->type;
    ValueType sourceType = value->type;

    // Coerce value to target type when a safe numeric promotion/demotion applies,
    // matching the same rules as EvalVarDecl.
    if (sourceType != targetType) {
        if (targetType == ValueType::Integer64 && sourceType == ValueType::Integer)
            value = new Int64Val(static_cast<IntVal*>(value)->value);
        else if (targetType == ValueType::Integer && sourceType == ValueType::Integer64)
            value = new IntVal(static_cast<int>(static_cast<Int64Val*>(value)->value));
        else if (targetType == ValueType::Float && sourceType == ValueType::Integer)
            value = new FloatVal(static_cast<float>(static_cast<IntVal*>(value)->value));
        else if (targetType == ValueType::Float64 && sourceType == ValueType::Integer)
            value = new Float64Val(static_cast<double>(static_cast<IntVal*>(value)->value));
        else if (targetType == ValueType::Float64 && sourceType == ValueType::Integer64)
            value = new Float64Val(static_cast<double>(static_cast<Int64Val*>(value)->value));
        else if (targetType == ValueType::Float64 && sourceType == ValueType::Float)
            value = new Float64Val(static_cast<double>(static_cast<FloatVal*>(value)->value));
        else if (targetType == ValueType::Float && sourceType == ValueType::Float64)
            value = new FloatVal(static_cast<float>(static_cast<Float64Val*>(value)->value));
        else {
            throw std::runtime_error("Type error: Cannot assign value of type " +
                std::string(valuetypeintarray[static_cast<int>(sourceType)]) +
                " to variable of type " +
                std::string(valuetypeintarray[static_cast<int>(targetType)]));
        }
    }
    if(env->IsConst[varname]){
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