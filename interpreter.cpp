#include<string>
#include <charconv>
#include <cctype>
#include<memory>
#include"values.hpp"
#include"ast.hpp"
#include"interpreter.hpp"
#include<iostream>
#include "environment.hpp"
/*
========================
HELPER FUNCTIONS
========================
*/
string valueTypeName(ValueType t) {
    switch (t) {
        case ValueType::Integer: return "Integer";
        case ValueType::Float: return "Float";
        case ValueType::String: return "String";
        case ValueType::Function: return "Function";
        case ValueType::Null: return "Null";
        case ValueType::Bool: return "Boolean";
    }
    return "Unknown";
}

LiteralType classifyLiteral(const std::string& s) {
    if (s.empty()) return LiteralType::Invalid;

    // Boolean check
    if (s == "true" || s == "false") {
        return LiteralType::Boolean;
    }

    // Check for quoted string
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
        return LiteralType::String;
    }

    // Try integer
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
/*
========================
EVALUATION FUNCTIONS
========================
*/

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

BoolVal EvalBoolBExpr(BoolVal left, BoolVal right, std::string op){
    bool result=false;
    bool works=false;
    if(op=="and"){
        result = left.value&&right.value;
        works=true;
    }else if(op=="or"){
        result = left.value||right.value;
        works=true;
    }
    if(works){
        return(BoolVal(result));
    }else{
       return(BoolVal(false));
    }
}

BoolVal EvalCompBExpr(RuntimeVal& left, RuntimeVal& right, std::string op){
    bool result=false;
    bool works=false;
    if(left.type==ValueType::Integer&&right.type==ValueType::Integer){
        int lval=static_cast<IntVal&>(left).value;
        int rval=static_cast<IntVal&>(right).value;
        // std::cout<<"Comparing "<<lval<<" and "<<rval<<" with =="<<std::endl;
        if(op=="=="){
            result = (lval==rval);
            works=true;
        }else if(op=="!="){
            result = lval!=rval;
            works=true;
        }else if(op=="<"){
            result = lval<rval;
            works=true;
        }else if(op==">"){
            result = lval>rval;
            works=true;
        }else if(op=="<="){
            result = lval<=rval;
            works=true;
        }else if(op==">="){
            result = lval>=rval;
            works=true;
        }
    }else if(left.type==ValueType::Float&&right.type==ValueType::Float){
        float lval=static_cast<FloatVal&>(left).value;
        float rval=static_cast<FloatVal&>(right).value;
            // std::cout<<"Comparing "<<lval<<" and "<<rval<<" with =="<<std::endl;
        if(op=="=="){
            result = (lval==rval);
            works=true;
        }else if(op=="!="){
            result = lval!=rval;
            works=true;
        }
        else if(op=="<"){
            result = lval<rval;
            works=true;
        }else if(op==">"){
            result = lval>rval;
            works=true;
        }else if(op=="<="){
            result = lval<=rval;
            works=true;       }
            else if(op==">="){
            result = lval>=rval;
            works=true;
        }
    }else if(left.type==ValueType::String&&right.type==ValueType::String){
        std::string lval=static_cast<StringVal&>(left).value;
        std::string rval=static_cast<StringVal&>(right).value;
            // std::cout<<"Comparing "<<lval<<" and "<<rval<<" with =="<<std::endl;
        if(op=="=="){
            result = (lval==rval);
            works=true;
        }else if(op=="!="){
            result = lval!=rval;
            works=true;
        }
    }else if(left.type==ValueType::Bool&&right.type==ValueType::Bool){
        bool lval=static_cast<BoolVal&>(left).value;
        bool rval=static_cast<BoolVal&>(right).value;
            // std::cout<<"Comparing "<<lval<<" and "<<rval<<" with =="<<std::endl;
        if(op=="=="){
            result = (lval==rval);
            works=true;
        }else if(op=="!="){
            result = lval!=rval;
            works=true;
        }
    }
    if(works){
        return(BoolVal(result));
    }else{
       return(BoolVal(false));
    }
}

FloatVal EvalFloatBExpr(FloatVal left, FloatVal right, std::string op){
    float result=0;
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
    // std::cerr<<"Evaluating binary expression with operator "<<binop.op<<" and left operand of type "<<valueTypeName(LHS->type)<<std::endl;
    if(binop.op=="and"||binop.op=="or"){
        if(!(LHS->type==ValueType::Bool)){
            throw std::runtime_error("Type Error: Logical operators require boolean operands");
        }else if(binop.op=="and"&&static_cast<BoolVal&>(*LHS).value==false){
            return(std::make_unique<BoolVal>(false));
        }else if(binop.op=="or"&&static_cast<BoolVal&>(*LHS).value==true){
            return(std::make_unique<BoolVal>(true));
    }}

    std::unique_ptr<RuntimeVal> RHS = Eval(binop.right, env);
    if(binop.op=="and"||binop.op=="or"){
    if(RHS->type!=ValueType::Bool){
        throw std::runtime_error("Type Error: Logical operators require boolean operands");
    }else if(binop.op=="and"){
        return(std::make_unique<BoolVal>(static_cast<BoolVal&>(*LHS).value&&static_cast<BoolVal&>(*RHS).value));
    }else if(binop.op=="or"){
        return(std::make_unique<BoolVal>(static_cast<BoolVal&>(*LHS).value||static_cast<BoolVal&>(*RHS).value));
    }
}
        
    //== n= < > =< >=
    if(binop.op=="=="||binop.op=="n="||binop.op=="<"||binop.op==">"||binop.op=="<="||binop.op==">="){
        return(std::make_unique<BoolVal>(EvalCompBExpr(*LHS,*RHS,binop.op)));
    }

    if(RHS->type==ValueType::Null||LHS->type==ValueType::Null){
        return(std::make_unique<Nullval>());
    }
    else if(RHS->type==ValueType::Integer||LHS->type==ValueType::Integer){
        //return 
        if(RHS->type==LHS->type){
        return(std::make_unique<IntVal>(EvalIntBExpr(static_cast<IntVal&>(*LHS),static_cast<IntVal&>(*RHS),binop.op)));}
    }else if(RHS->type==ValueType::Float&&LHS->type==ValueType::Float){
        return(std::make_unique<FloatVal>(EvalFloatBExpr(static_cast<FloatVal&>(*LHS),static_cast<FloatVal&>(*RHS),binop.op)));
    }else if((RHS->type==ValueType::Integer&&LHS->type==ValueType::Float)||(RHS->type==ValueType::Float&&LHS->type==ValueType::Integer)){
        FloatVal lval=(RHS->type==ValueType::Integer)? FloatVal(static_cast<IntVal&>(*LHS).value):static_cast<FloatVal&>(*LHS);
        FloatVal rval=(RHS->type==ValueType::Integer)? FloatVal(static_cast<IntVal&>(*RHS).value):static_cast<FloatVal&>(*RHS);
        return(std::make_unique<FloatVal>(EvalFloatBExpr(lval,rval,binop.op)));
    }else if(RHS->type==ValueType::Bool&&LHS->type==ValueType::Bool){
        return(std::make_unique<BoolVal>(EvalBoolBExpr(static_cast<BoolVal&>(*LHS),static_cast<BoolVal&>(*RHS),binop.op)));
    }
    return(std::make_unique<Nullval>());

}

std::unique_ptr<RuntimeVal> EvalIdentifier(Identifier identifier, Environment& env){
    auto val = env.getVal(identifier.name);
    switch(val->type){
        case ValueType::Integer:
            return std::make_unique<IntVal>(static_cast<IntVal&>(*val));
        case ValueType::Float:
            return std::make_unique<FloatVal>(static_cast<FloatVal&>(*val));
        case ValueType::String:
            return std::make_unique<StringVal>(static_cast<StringVal&>(*val));
        case ValueType::Bool:
            return std::make_unique<BoolVal>(static_cast<BoolVal&>(*val));
        default:
            return std::make_unique<Nullval>();
    }
}

std::unique_ptr<RuntimeVal> EvalVarDecl(VarDecl decl, Environment& env){
    std::unique_ptr<RuntimeVal> initVal;
    if(decl.value!=nullptr){
        initVal = Eval(decl.value, env);
    }else{
        switch(decl.type){
            case ValueType::Integer:
                initVal = std::make_unique<IntVal>(0);
                break;
            case ValueType::Float:
                initVal = std::make_unique<FloatVal>(0.0);
                break;
            case ValueType::String:
                initVal = std::make_unique<StringVal>("");
                break;
            case ValueType::Bool:
                initVal = std::make_unique<BoolVal>(false);
                break;
            default:
                initVal = std::make_unique<Nullval>();
        }
        // initVal = std::make_unique<Nullval>();
    }
    env.declareVal(decl.name, std::move(initVal), decl.immutable);
    return std::make_unique<Nullval>();
}


/*
========================
INTERPRETER
========================
*/

std::unique_ptr<RuntimeVal> Eval(Stmt* astNode, Environment& env){
    switch(astNode->kind){
        case NodeType::Literal: {
            auto* lit = dynamic_cast<Literal*>(astNode);
            auto type = classifyLiteral(lit->value);
            if(type == LiteralType::Integer)
                return std::make_unique<IntVal>(std::stoi(lit->value));
            else if(type == LiteralType::Float)
                return std::make_unique<FloatVal>(std::stof(lit->value));
            else if(type == LiteralType::String)
                return std::make_unique<StringVal>(lit->value);
            else if(type == LiteralType::Boolean)
                return std::make_unique<BoolVal>(lit->value == "true");
            else
                {return std::make_unique<Nullval>(); }
            break;
        }
        case NodeType::VariableDeclaration: {
            auto* decl = dynamic_cast<VarDecl*>(astNode);
            return EvalVarDecl(*decl, env);

        }
        case NodeType::Identifier: {
            auto* id = dynamic_cast<Identifier*>(astNode);
            return EvalIdentifier(*id, env);
        }
        case NodeType::BinaryExpression: {
            auto* binop = dynamic_cast<BinaryExpr*>(astNode);
            return EvalBinaryExpr(*binop, env);
        }

        case NodeType::Program: {
            return EvalProgram(dynamic_cast<Program*>(astNode),env);
        }case NodeType::Assignment: {
            auto* asgn = dynamic_cast<Assignment*>(astNode);
            auto* ident = dynamic_cast<Identifier*>(asgn->target);
            if (!ident) {
                throw std::runtime_error("Invalid assignment target (L-Value error)");
            }
            auto val = Eval(asgn->value, env);
            env.assignVal(ident->name, std::move(val)); 
            return env.getVal(ident->name)->clone();
    }

        default:
            std::cerr<<"Unimplemented AST node type in Eval: "<<static_cast<int>(astNode->kind)<<std::endl;
            return std::make_unique<Nullval>();
    }
}
