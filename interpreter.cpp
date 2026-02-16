#include<string>
#include <charconv>
#include <cctype>
#include<memory>
#include"values.hpp"
#include"ast.hpp"
#include"interpreter.hpp"

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

std::unique_ptr<RuntimeVal> EvalProgram(Program* prog) {
    std::unique_ptr<RuntimeVal> result = std::make_unique<Nullval>();
    for (auto stmt : prog->body) {
        result = Eval(stmt);
    }
    return std::make_unique<RuntimeVal>(result);
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
        return(IntVal(NAN));
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
        return(FloatVal(NAN));
    }
}

std::unique_ptr<RuntimeVal> EvalBinaryExpr(BinaryExpr binop){
    std::unique_ptr<RuntimeVal> LHS = Eval(binop.left);
    std::unique_ptr<RuntimeVal> RHS = Eval(binop.right);
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




std::unique_ptr<RuntimeVal> Eval(Stmt* astNode){
    switch(astNode->kind){
        case(NodeType::Literal):
        if(classifyLiteral(dynamic_cast<Literal*>(astNode)->value) == LiteralType::Integer){
            Literal* lit = dynamic_cast<Literal*>(astNode);
            return std::make_unique<IntVal>(std::stoi(lit->value));
        } else if(classifyLiteral(dynamic_cast<Literal*>(astNode)->value) == LiteralType::Float){
            Literal* lit = dynamic_cast<Literal*>(astNode);
            return std::make_unique<FloatVal>(std::stof(lit->value));}
        case(NodeType::Program):
            return EvalProgram(dynamic_cast<Program*>(astNode));
    }

}