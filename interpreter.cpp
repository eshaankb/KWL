#include<string>
#include <charconv>
#include <cctype>
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
        case ValueType::Structure: return "Class";
    }
    return "Unknown";
}

LiteralType classifyLiteral(const std::string& s) {
    if (s.empty()) return LiteralType::Invalid;

    // Boolean check
    if (s == "true" || s == "false") {
        return LiteralType::Boolean;
    }

    // Check for quoted string (double or single quotes)
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') ||
                           (s.front() == '\'' && s.back() == '\''))) {
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

RuntimeVal* EvalProgram(Program* prog,Environment& env) {
    RuntimeVal* result = new Nullval();
    for (auto stmt : prog->body) {
        result = Eval(stmt, env);
    }
    return (result);
}

RuntimeVal* EvalIfStmt(IfStmt* ifstmt, Environment& env) {
    auto condVal = Eval(ifstmt->condition, env);
    if (condVal->type != ValueType::Bool) {
        throw std::runtime_error("Type Error: Condition in if statement must evaluate to a boolean");
    }
    if (static_cast<BoolVal&>(*condVal).value) {
        Environment localEnv(&env); // create a new environment for the if block
        return Eval(ifstmt->body, localEnv);
    } else if (ifstmt->elseBranch != nullptr) {
        Environment localEnv(&env); // create a new environment for the if block
        return Eval(ifstmt->elseBranch, localEnv);
    } else {
        return new Nullval();
    }
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


RuntimeVal* EvalBinaryExpr(BinaryExpr binop, Environment& env){
    RuntimeVal* LHS = Eval(binop.left, env);
    // std::cerr<<"Evaluating binary expression with operator "<<binop.op<<" and left operand of type "<<valueTypeName(LHS->type)<<std::endl;
    if(binop.op=="and"||binop.op=="or"){
        if(!(LHS->type==ValueType::Bool)){
            throw std::runtime_error("Type Error: Logical operators require boolean operands");
        }else if(binop.op=="and"&&static_cast<BoolVal&>(*LHS).value==false){
            return(new BoolVal(false));
        }else if(binop.op=="or"&&static_cast<BoolVal&>(*LHS).value==true){
            return(new BoolVal(true));
    }}

    RuntimeVal* RHS = Eval(binop.right, env);
    if(binop.op=="and"||binop.op=="or"){
    if(RHS->type!=ValueType::Bool){
        throw std::runtime_error("Type Error: Logical operators require boolean operands");
    }else if(binop.op=="and"){
        return(new BoolVal(static_cast<BoolVal&>(*LHS).value&&static_cast<BoolVal&>(*RHS).value));
    }else if(binop.op=="or"){
        return(new BoolVal(static_cast<BoolVal&>(*LHS).value||static_cast<BoolVal&>(*RHS).value));
    }
}
        
    //== n= < > =< >=
    if(binop.op=="=="||binop.op=="n="||binop.op=="<"||binop.op==">"||binop.op=="<="||binop.op==">="){
        return(new BoolVal(EvalCompBExpr(*LHS,*RHS,binop.op)));
    }

    if(RHS->type==ValueType::Null||LHS->type==ValueType::Null){
        return(new Nullval());
    }
    else if(RHS->type==ValueType::Integer||LHS->type==ValueType::Integer){
        //return 
        if(RHS->type==LHS->type){
        return(new IntVal(EvalIntBExpr(static_cast<IntVal&>(*LHS),static_cast<IntVal&>(*RHS),binop.op)));}
    }else if(RHS->type==ValueType::Float&&LHS->type==ValueType::Float){
        return(new FloatVal(EvalFloatBExpr(static_cast<FloatVal&>(*LHS),static_cast<FloatVal&>(*RHS),binop.op)));
    }else if((RHS->type==ValueType::Integer&&LHS->type==ValueType::Float)||(RHS->type==ValueType::Float&&LHS->type==ValueType::Integer)){
        FloatVal lval=(RHS->type==ValueType::Integer)? FloatVal(static_cast<IntVal&>(*LHS).value):static_cast<FloatVal&>(*LHS);
        FloatVal rval=(RHS->type==ValueType::Integer)? FloatVal(static_cast<IntVal&>(*RHS).value):static_cast<FloatVal&>(*RHS);
        return(new FloatVal(EvalFloatBExpr(lval,rval,binop.op)));
    }else if(RHS->type==ValueType::Bool&&LHS->type==ValueType::Bool){
        return(new BoolVal(EvalBoolBExpr(static_cast<BoolVal&>(*LHS),static_cast<BoolVal&>(*RHS),binop.op)));
    }
    return(new Nullval());

}

RuntimeVal* EvalIdentifier(Identifier identifier, Environment& env){
    auto val = env.getVal(identifier.name);
    switch(val->type){
        case ValueType::Integer:
            return(new IntVal(static_cast<IntVal&>(*val)));
        case ValueType::Float:
            return(new FloatVal(static_cast<FloatVal&>(*val)));
        case ValueType::String:
            return(new StringVal(static_cast<StringVal&>(*val)));
        case ValueType::Bool:
            return(new BoolVal(static_cast<BoolVal&>(*val)));
        default:
            return(new Nullval());
    }
}

RuntimeVal* EvalStruct(StructDecl* decl, Environment& env){
    // Register the class definition in the environment
    env.declareClass(decl->name, decl);
    return new Nullval();
}

RuntimeVal* EvalConstructorCall(CallExpr call, Environment& env){
    // callee should be identifier naming class
    if (auto id = dynamic_cast<Identifier*>(call.callee)) {
        StructDecl* classDecl = env.getClass(id->name);
        if (!classDecl) {
            throw std::runtime_error("Runtime Error: Class '" + id->name + "' not found");
        }
        
        // Create a new instance with default field values
        StructureVal* instance = new StructureVal({}, classDecl->name);
        
        // initialize defaults
        for (Stmt* stmt : classDecl->vars->body) {
            if (stmt->kind == NodeType::VariableDeclaration) {
                VarDecl* var = dynamic_cast<VarDecl*>(stmt);
                RuntimeVal* initVal;
                switch(var->type){
                    case ValueType::Integer:
                        initVal = new IntVal(0);
                        break;
                    case ValueType::Float:
                        initVal = new FloatVal(0.0);
                        break;
                    case ValueType::String:
                        initVal = new StringVal("");
                        break;
                    case ValueType::Bool:
                        initVal = new BoolVal(false);
                        break;
                    default:
                        initVal = new Nullval();
                }
                instance->fields[var->name] = initVal;
            }
        }
        
        // apply constructor parameters if provided
        if (classDecl->constructor && !call.arguments.empty()) {
            auto& params = classDecl->constructor->params;
            if (call.arguments.size() != params.size()) {
                throw std::runtime_error("Runtime Error: Constructor expects " + std::to_string(params.size()) + 
                                         " arguments but got " + std::to_string(call.arguments.size()));
            }
            for (size_t i = 0; i < params.size(); i++) {
                RuntimeVal* argVal = Eval(call.arguments[i], env);
                if (argVal->type != params[i].second) {
                    throw std::runtime_error("Type Error: Constructor argument type mismatch for parameter '" + 
                                           params[i].first + "'");
                }
                instance->fields[params[i].first] = argVal;
            }
        }
        return instance;
    }
    throw std::runtime_error("Runtime Error: Constructor call on non-identifier");
}

RuntimeVal* EvalCallStruct(CallStructExpr call, Environment& env){
    RuntimeVal* obj = Eval(call.object, env);
    if(obj->type!=ValueType::Structure){
        throw std::runtime_error("Type Error: Attempting to access field of non-structure value");
    }
    StructureVal& structVal = static_cast<StructureVal&>(*obj);
    auto it = structVal.fields.find(call.field);
    if(it==structVal.fields.end()){
        throw std::runtime_error("Runtime Error: Field '"+call.field+"' not found in structure");
    }
    return it->second;
}

RuntimeVal* EvalVarDecl(VarDecl decl, Environment& env){
    std::cerr << "[EVAL] EvalVarDecl name="<< decl.name << " type="<< valueTypeName(decl.type) <<" struct="<< decl.structTypeName <<"\n";
    RuntimeVal* initVal;
    if(decl.value!=nullptr){
        initVal = Eval(decl.value, env);
        std::cerr << "[EVAL] initializer returned type="<< valueTypeName(initVal->type) <<"\n";
        if (decl.type == ValueType::Structure) {
            if (initVal->type != ValueType::Structure) {
                throw std::runtime_error("Type Error: Expected structure initializer for variable '" + decl.name + "'");
            }
            // optionally check class name match
            if (!decl.structTypeName.empty()) {
                StructureVal& sv = static_cast<StructureVal&>(*initVal);
                if (sv.className != decl.structTypeName) {
                    throw std::runtime_error("Type Error: Expected instance of class '" + decl.structTypeName + "'");
                }
            }
        } else {
            if (initVal->type != decl.type) {
                throw std::runtime_error("Type Error: Variable declaration type does not match initializer type");
            }
        }
    }else{
        switch(decl.type){
            case ValueType::Integer:
                initVal = new IntVal(0);
                break;
            case ValueType::Float:
                initVal = new FloatVal(0.0);
                break;
            case ValueType::String:
                initVal = new StringVal("");
                break;
            case ValueType::Bool:
                initVal = new BoolVal(false);
                break;
            case ValueType::Structure:
                // create empty structure with class name
                {
                    StructureVal* inst = new StructureVal();
                    inst->className = decl.structTypeName;
                    initVal = inst;
                }
                break;
            default:
                initVal = new Nullval();
        }
    }
    env.declareVal(decl.name, initVal, decl.immutable);
    return new Nullval();
}


/*
========================
INTERPRETER
========================
*/
std::string printNodeType(NodeType t){
    switch(t){
        case NodeType::Program: return "Program";
        case NodeType::BlockStatement: return "BlockStatement";
        case NodeType::VariableDeclaration: return "VariableDeclaration";
        case NodeType::Assignment: return "Assignment";
        case NodeType::BinaryExpression: return "BinaryExpression";
        case NodeType::UnaryExpression: return "UnaryExpression";
        case NodeType::Literal: return "Literal";
        case NodeType::Identifier: return "Identifier";
        case NodeType::CallExpression: return "CallExpression";
        case NodeType::IndexExpression: return "IndexExpression";
        case NodeType::IfStatement: return "IfStatement";
        case NodeType::ElseStatement: return "ElseStatement";
        case NodeType::WhileStatement: return "WhileStatement";
        case NodeType::ReturnStatement: return "ReturnStatement";
        case NodeType::BuiltinCall: return "BuiltinCall";
        default: return "UnknownNodeType";
    }
}

RuntimeVal* Eval(Stmt* astNode, Environment& env){
    if (!astNode) {
        std::cerr << "[EVAL] called with null astNode\n";
        return new Nullval();
    }
    std::cerr << "[EVAL] node kind=" << printNodeType(astNode->kind) << "\n";
    switch(astNode->kind){
        case NodeType::IfStatement: {
            return EvalIfStmt(dynamic_cast<IfStmt*>(astNode), env);
        }
        case NodeType::Literal: {
            auto* lit = dynamic_cast<Literal*>(astNode);
            auto type = classifyLiteral(lit->value);
            if(type == LiteralType::Integer)
                return new IntVal(std::stoi(lit->value));
            else if(type == LiteralType::Float)
                return new FloatVal(std::stof(lit->value));
            else if(type == LiteralType::String) {
                string str = lit->value;
                if (str.size() >= 2 && ((str.front() == '"' && str.back() == '"') ||
                                        (str.front() == '\'' && str.back() == '\''))) {
                    str = str.substr(1, str.size() - 2);
                }
                return new StringVal(str);
            }
            else if(type == LiteralType::Boolean)
                return new BoolVal(lit->value == "true");
            else
                {return new Nullval(); }
            break;
        }
        case NodeType::VariableDeclaration: {
            auto* decl = dynamic_cast<VarDecl*>(astNode);
            return EvalVarDecl(*decl, env);

        }case NodeType::StructureDeclaration: {
            auto structure = dynamic_cast<StructDecl*>(astNode);
            return EvalStruct(structure, env);

        }case NodeType::CallExpression: {
            if (auto callStructExpr = dynamic_cast<CallStructExpr*>(astNode)) {
                return EvalCallStruct(*callStructExpr, env);
            } else if (auto callExpr = dynamic_cast<CallExpr*>(astNode)) {
                // check for constructor call
                try {
                    return EvalConstructorCall(*callExpr, env);
                } catch (const std::runtime_error& r) {
                    // if class not found, maybe it's a normal function
                    // for now we rethrow
                    throw;
                }
            }
            return new Nullval();
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
            env.assignVal(ident->name, val); 
            return env.getVal(ident->name);
        }default:
            std::cerr<<"Unimplemented AST node type in Eval: "<<printNodeType(astNode->kind)<<std::endl;
            return new Nullval();
    }
}
