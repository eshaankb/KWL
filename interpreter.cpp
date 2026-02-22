#include<string>
#include <charconv>
#include <cctype>
#include<memory>
#include"values.hpp"
#include"ast.hpp"
#include"interpreter.hpp"
#include<iostream>
#include "environment.hpp"
string valueTypeName(ValueType t) {
    switch (t) {
        case ValueType::Integer: return "Integer";
        case ValueType::Float: return "Float";
        case ValueType::String: return "String";
        case ValueType::Function: return "Function";
        case ValueType::Null: return "Null";
    }
    return "Unknown";
}

LiteralType classifyLiteral(const std::string& s) {
    if (s.empty()) return LiteralType::Invalid;

    // Check for quoted string
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return LiteralType::String;
    }
    {
        int value;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
        if (ec == std::errc() && ptr == s.data() + s.size()) {
            return LiteralType::Integer;
        }
    }

    // Try float (fallback to stod)
    try {
        size_t idx;
        std::stod(s, &idx);
        if (idx == s.size()) {
            return LiteralType::Float;
        }
    } catch (...) {}

    return LiteralType::Invalid;
}

std::unique_ptr<RuntimeVal> EvalProgram(Program* prog,Environment& env) {
    std::unique_ptr<RuntimeVal> result = std::make_unique<Nullval>();
    for (auto stmt : prog->body) {
        result = Eval(stmt, env);
    }
    return (result);
}


IntVal EvalIntBExpr(IntVal left, IntVal right, std::string op){
    int result=0;
    bool works=false;
    if(op=="+"){
        result = left.value+right.value;
        works=true;
    }else if(op=="-"){
        result = left.value-right.value;
        works=true;
    }else if(op=="*"){
        result = left.value*right.value;
        works=true;
    }else if(op=="/"){
        if(right.value!=0){
        result = left.value/right.value;
        works=true;}
    }
    if(works){
        return(IntVal(result));
    }else{
       return(IntVal(65535));
    }
}

FloatVal EvalFloatBExpr(FloatVal left, FloatVal right, std::string op){
    int result=0;
    bool works=false;
    if(op=="+"){
        result = left.value+right.value;
        works=true;
    }else if(op=="-"){
        result = left.value-right.value;
        works=true;
    }else if(op=="*"){
        result = left.value*right.value;
        works=true;
    }else if(op=="/"){
        if(right.value!=0){
        result = left.value/right.value;
        works=true;}
    }
    if(works){
        return(FloatVal(result));
    }else{
        return(FloatVal(65535.0));
    }
}

std::unique_ptr<RuntimeVal> EvalBinaryExpr(BinaryExpr binop, Environment& env){
    std::unique_ptr<RuntimeVal> LHS = Eval(binop.left, env);
    std::unique_ptr<RuntimeVal> RHS = Eval(binop.right, env);
    if(RHS->type==ValueType::Null||LHS->type==ValueType::Null){
        return(std::make_unique<Nullval>());
    }
    else if(RHS->type==ValueType::Integer||LHS->type==ValueType::Integer){
        //return 
        if(RHS->type==LHS->type){
        return(std::make_unique<IntVal>(EvalIntBExpr(static_cast<IntVal&>(*LHS),static_cast<IntVal&>(*RHS),binop.op)));}
    }else if(RHS->type==ValueType::Float&&LHS->type==ValueType::Float){
        return(std::make_unique<IntVal>(EvalIntBExpr(static_cast<IntVal&>(*LHS),static_cast<IntVal&>(*RHS),binop.op)));
    }
    return(std::make_unique<Nullval>());

}

std::unique_ptr<RuntimeVal> EvalIdentifier(Identifier identifier, Environment& env){
    switch(env.getVal(identifier.name)->type){
        case ValueType::Integer:
            return std::make_unique<IntVal>(static_cast<IntVal&>(*env.getVal(identifier.name)));
        case ValueType::Float:
            return std::make_unique<FloatVal>(static_cast<FloatVal&>(*env.getVal(identifier.name)));
        case ValueType::String:
            return std::make_unique<StringVal>(static_cast<StringVal&>(*env.getVal(identifier.name)));
        default:
            return std::make_unique<Nullval>();
    }
}




std::unique_ptr<RuntimeVal> Eval(Stmt* astNode, Environment& env){
    switch(astNode->kind){
        case NodeType::Literal: {
            auto* lit = dynamic_cast<Literal*>(astNode);
            auto type = classifyLiteral(lit->value);
            if(type == LiteralType::Integer)
                return std::make_unique<IntVal>(std::stoi(lit->value));
            else if(type == LiteralType::Float)
                return std::make_unique<FloatVal>(std::stof(lit->value));
            else
                return std::make_unique<StringVal>(lit->value);
        }
        case NodeType::Identifier: {

        }
        case NodeType::BinaryExpression: {
            auto* binop = dynamic_cast<BinaryExpr*>(astNode);
            return EvalBinaryExpr(*binop, env);
        }

        case NodeType::Program: {
            return EvalProgram(dynamic_cast<Program*>(astNode),env);
        }

        default:
            std::cerr<<"Unimplemented AST node type in Eval: "<<static_cast<int>(astNode->kind)<<std::endl;
            return std::make_unique<Nullval>();
    }
}
