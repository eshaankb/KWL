#include<string>
#include <charconv>
#include <cctype>
#include"values.hpp"
#include"ast.hpp"
#include"interpreter.hpp"
RuntimeVal EvalProgram(Program* prog) {
    RuntimeVal result = Nullval();
    for (auto stmt : prog->body) {
        result = Eval(stmt);
    }
    return result;
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

 RuntimeVal EvalBinaryExpr(BinaryExpr binop){
    RuntimeVal LHS = Eval(binop.left);
    RuntimeVal  RHS = Eval(binop.right);
    if(RHS.type==ValueType::Integer||LHS.type==ValueType::Integer){
        //return 
    }else if(RHS.type==ValueType::Float&&RHS.type==ValueType::Float){
        
    }
    return(Nullval());

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

RuntimeVal Eval(Stmt* astNode){
    switch(astNode->kind){
        case(NodeType::Literal):
        if(classifyLiteral(dynamic_cast<Literal*>(astNode)->value) == LiteralType::Integer){
            Literal* lit = dynamic_cast<Literal*>(astNode);
            return IntVal(std::stoi(lit->value));
        } else if(classifyLiteral(dynamic_cast<Literal*>(astNode)->value) == LiteralType::Float){
            Literal* lit = dynamic_cast<Literal*>(astNode);
            return Floatval(std::stof(lit->value));}
        case(NodeType::Program):
            return EvalProgram(dynamic_cast<Program*>(astNode));
    }

}