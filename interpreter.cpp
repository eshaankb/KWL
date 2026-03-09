#include<string>
#include <charconv>
#include <cctype>
#include <fstream>
#include"types.hpp"
#include"values.hpp"
#include"ast.hpp"
#include"parser.hpp"
#include"interpreter.hpp"
#include<iostream>
#include "environment.hpp"
#include <filesystem>
namespace fs = std::filesystem;
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
RuntimeVal* createDefaultValue(ValueType type) {
    switch(type) {
        case ValueType::Integer: return new IntVal(0);
        case ValueType::Float:   return new FloatVal(0.0);
        case ValueType::String:  return new StringVal("");
        case ValueType::Bool:    return new BoolVal(false);
        default:                 return new Nullval();
    }
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

RuntimeVal* EvalBlockStmt(BlockStmt* block, Environment& env) {
    RuntimeVal* result = new Nullval();
    Environment localEnv(&env); // create a new environment for the block
    for (auto stmt : block->body) {
        result = Eval(stmt, localEnv);
    }
    return result;
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
    if (!val) {
        throw std::runtime_error("Runtime Error: Undefined identifier '" + identifier.name + "'");
    }
    // structures and other complex values can be cloned directly
    if(val->type == ValueType::Structure){
        return val->clone();
    }
    switch(val->type){
        case ValueType::Integer:
            return new IntVal(static_cast<IntVal&>(*val));
        case ValueType::Float:
            return new FloatVal(static_cast<FloatVal&>(*val));
        case ValueType::String:
            return new StringVal(static_cast<StringVal&>(*val));
        case ValueType::Bool:
            return new BoolVal(static_cast<BoolVal&>(*val));
        default:
            return new Nullval();
    }
}

RuntimeVal* EvalStruct(StructDecl* decl, Environment& env){
    // Register the class definition in the environment
    env.declareClass(decl->name, decl);
    return new Nullval();
}

RuntimeVal* EvalConstructorCall(ConstructorCallExpr call, Environment& env){
    
    StructDecl* classDecl = env.getClass(call.className);
    if (!classDecl) {
        throw std::runtime_error("Runtime Error: Class '" + call.className + "' not found");
    }
    
    
    StructureVal* instance = new StructureVal({}, classDecl->name);
    
    
    for (const auto& field : classDecl->vars) {
        RuntimeVal* initVal;
            switch(field.second){
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
                        break;
                }
                instance->fields[field.first] = initVal;
    }
        
        if (classDecl->constructor && !call.arguments.empty()) {
            if (call.arguments.size() != classDecl->constructor->params.size()) {
                throw std::runtime_error("Runtime Error: Constructor expects " + std::to_string(classDecl->constructor->params.size()) + 
                                         " arguments but got " + std::to_string(call.arguments.size()));
            }
            for (size_t i = 0; i < classDecl->constructor->params.size(); i++) {
                RuntimeVal* argVal = Eval(call.arguments[i], env);
                if (argVal->type != classDecl->constructor->params[i].second) {
                    throw std::runtime_error("Type Error: Constructor argument type mismatch for parameter '" + 
                                           classDecl->constructor->params[i].first + "'");
                }
                instance->fields[classDecl->constructor->params[i].first] = argVal;
            }
        }
        return instance;}

RuntimeVal* EvalCallStruct(CallStructExpr* call, Environment& env){
    RuntimeVal* obj = Eval(call->object, env);
    
    if (auto ident = dynamic_cast<Identifier*>(call->object)) {
        RuntimeVal* potentialModule = nullptr;
        try {
            potentialModule = env.getVal(ident->name);
        } catch (...) {
        }
        
        if (potentialModule && (uintptr_t)potentialModule > 1000) { 
            Environment* modEnv = reinterpret_cast<Environment*>(potentialModule);
            try {
                StructDecl* classDecl = modEnv->getClass(call->field);
                if (classDecl) {
                    StructureVal* instance = new StructureVal({}, classDecl->name);
                    for (const auto& field : classDecl->vars) {
                        RuntimeVal* initVal;
                        switch(field.second){
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
                                break;
                        }
                        instance->fields[field.first] = initVal;
                    }
                    return instance;
                }
            } catch (...) {
            }
        }
    }
    
    if(obj->type!=ValueType::Structure){
        throw std::runtime_error("Type Error: Attempting to access field of non-structure value");
    }
    StructureVal& structVal = static_cast<StructureVal&>(*obj);
    auto it = structVal.fields.find(call->field);
    if(it==structVal.fields.end()){
        throw std::runtime_error("Runtime Error: Field '"+call->field+"' not found in structure");
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
        case NodeType::NullLiteral: return "NullLiteral";
        case NodeType::StructureDeclaration: return "StructureDeclaration";
        case NodeType::ConstructorCall: return "ConstructorCall";
        default: return "UnknownNodeType";
    }
}
RuntimeVal* EvalFunctionDecl(FunctionDecl* decl, Environment& env){
    FunctionVal* func = new FunctionVal({{}}, nullptr);
    func->paramNames.reserve(decl->parameters.size());
    for (int i = 0; i < decl->parameters.size(); i++) {
        func->paramNames.emplace_back(decl->parameters[i]->name, decl->parameters[i]->type);
    }
    func->body = decl->body;
    env.declareVal(decl->name, func, true);
    return new Nullval();
}

RuntimeVal* EvalFunctionCall(FunctionCall* call, Environment& env){
    FunctionVal* funcVal = dynamic_cast<FunctionVal*>(env.getVal(call->functionName));
    if (!funcVal) {
        throw std::runtime_error("Runtime Error: '" + call->functionName + "' is not a function");
    }
    if (call->arguments.size() != funcVal->paramNames.size()) {
        throw std::runtime_error("Runtime Error: Function '" + call->functionName + "' expects " + 
                                 std::to_string(funcVal->paramNames.size()) + " arguments but got " + 
                                 std::to_string(call->arguments.size()));
    }
    Environment* localEnv =  new Environment(&env); // create a new environment for the function call
    for(size_t i = 0; i < call->arguments.size(); i++) {
        localEnv->declareVal(funcVal->paramNames[i].first, Eval(call->arguments[i], env));
    }
    try {
        return Eval(funcVal->body, *localEnv);
    } catch (const RetVal& ret) {
        return ret.value;
    }

}


RuntimeVal* Eval(Stmt* astNode, Environment& env){
    if (!astNode) {
        std::cerr << "[EVAL] called with null astNode\n";
        return new Nullval();
    }
    std::cerr << "[EVAL] node kind=" << printNodeType(astNode->kind) << "\n";
    switch(astNode->kind){
        case NodeType::ImportStatement: {
            auto* import = dynamic_cast<ImportStmt*>(astNode);
            std::string modPath = import->modulePath;

            if (!modPath.empty() && ((modPath.front() == '"' && modPath.back() == '"') ||
                                     (modPath.front() == '\'' && modPath.back() == '\''))) {
                modPath = modPath.substr(1, modPath.size() - 2);
            }

            if (modPath.size() >= 2 && modPath[0] == '.' && modPath[1] == '/') {
                modPath = modPath.substr(2);
            }

            if (modPath.find(".kwl") == std::string::npos) {
                modPath += ".kwl";
            }

            std::vector<fs::path> searchPaths = {
                fs::current_path(),
                fs::current_path() / "modules",
                "/usr/local/lib/kwl/"
            };

            fs::path targetFilePath;
            bool found = false;

            for (const auto& dir : searchPaths) {
                fs::path fullPath = dir / modPath;
                if (fs::exists(fullPath)) {
                    targetFilePath = fullPath;
                    found = true;
                    break;
                }
            }

            if (!found) {
                throw std::runtime_error("Module '" + modPath + "' not found in search paths.");
            }

            std::ifstream modfile(targetFilePath);
            if (!modfile.is_open()) {
                throw std::runtime_error("Failed to open module file: " + targetFilePath.string());
            }
            std::string modsrc((std::istreambuf_iterator<char>(modfile)), std::istreambuf_iterator<char>());
            modfile.close();
            Parser modParser;
            Program modAST = modParser.produceAST(modsrc);
            Environment* modEnv = new Environment();
            for (auto stmt : modAST.body) {
                if (stmt->kind == NodeType::ExportDeclaration) {
                    auto* exportDecl = dynamic_cast<ExportDecl*>(stmt);
                    Eval(exportDecl->decl, *modEnv);
                }
            }
            if (import->importAll) {
                for (const auto& [name, val] : modEnv->variables) {
                    env.declareVal(name, val->clone());
                }
            } else {
                if (!import->alias.empty()) {
                    env.variables[import->alias] = reinterpret_cast<RuntimeVal*>(modEnv);
                }
            }
            return new Nullval();
        }
        case NodeType::ExportDeclaration: {
            return new Nullval();
        }
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
            
            if (auto callExpr = dynamic_cast<CallExpr*>(astNode)) {
                RuntimeVal* calleeVal = Eval(callExpr->callee, env);
                if (calleeVal && calleeVal->type == ValueType::Structure) {
                    StructureVal& structVal = static_cast<StructureVal&>(*calleeVal);
                    try {
                        StructDecl* classDecl = env.getClass(structVal.className);
                        if (classDecl && classDecl->constructor && !callExpr->arguments.empty()) {
                            if (callExpr->arguments.size() != classDecl->constructor->params.size()) {
                                throw std::runtime_error("Runtime Error: Constructor expects " + std::to_string(classDecl->constructor->params.size()) + 
                                                       " arguments but got " + std::to_string(callExpr->arguments.size()));
                            }
                            for (size_t i = 0; i < callExpr->arguments.size(); i++) {
                                RuntimeVal* argVal = Eval(callExpr->arguments[i], env);
                                if (argVal->type != classDecl->constructor->params[i].second) {
                                    throw std::runtime_error("Type Error: Constructor argument type mismatch for parameter '" + 
                                                           classDecl->constructor->params[i].first + "'");
                                }
                                structVal.fields[classDecl->constructor->params[i].first] = argVal;
                            }
                        }
                    } catch (const std::runtime_error& e) {

                    }
                    return calleeVal;
                }
                return new Nullval();
            }
            
            if (auto callStructExpr = dynamic_cast<CallStructExpr*>(astNode)) {
                return EvalCallStruct(callStructExpr, env);
            }
            return new Nullval();
        }
        case NodeType::ConstructorCall: {
            auto* constrCall = dynamic_cast<ConstructorCallExpr*>(astNode);
            return EvalConstructorCall(*constrCall, env);
        }
        case NodeType::Identifier: {
            auto* id = dynamic_cast<Identifier*>(astNode);
            return EvalIdentifier(*id, env);
        }
        case NodeType::BinaryExpression: {
            auto* binop = dynamic_cast<BinaryExpr*>(astNode);
            return EvalBinaryExpr(*binop, env);
        }case NodeType::ReturnStatement: {
            auto* stmt = static_cast<ReturnStmt*>(astNode);
            RuntimeVal* val = new Nullval();
            
            if (stmt->value) {
                val = Eval(stmt->value, env);
            }

            throw RetVal(val);
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
        } case NodeType::BlockStatement: {
            return EvalBlockStmt(dynamic_cast<BlockStmt*>(astNode), env);
        } case NodeType::FunctionDeclaration: {
            return EvalFunctionDecl(dynamic_cast<FunctionDecl*>(astNode), env);
        } case NodeType::FunctionCall: {
            return EvalFunctionCall(dynamic_cast<FunctionCall*>(astNode), env);
        }default:
            std::cerr<<"Unimplemented AST node type in Eval: "<<printNodeType(astNode->kind)<<std::endl;
            return new Nullval();
    }
}
