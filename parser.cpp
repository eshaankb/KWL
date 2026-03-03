#include "parser.hpp"
#include"values.hpp"
#include<string>
#include <stdexcept>

using namespace std;

Program Parser::produceAST(string sourcecode) {
    tokens = tokenize(sourcecode);

    Program program;

    int stmtCount = 0;
    while (notEOF()) {
        std::cerr << "[PARSER] about to parse statement, next token=" << tokens[0].value << "\n";
        Stmt* s = ParseStmt();
        if (s == nullptr) {
            std::cerr << "[PARSER] ParseStmt returned nullptr\n";
        }
        program.body.push_back(s);
        stmtCount++;
        if (stmtCount > 1000) {
            std::cerr << "[PARSER] too many statements, aborting\n";
            break;
        }
    }

    std::cerr << "[PARSER] finished produceAST with "<<program.body.size()<<" statements\n";
    return program;
}

bool Parser::notEOF() {
    return tokens.size() && tokens[0].type != TokenType::EndOfFile;
}

Token& Parser::peek(int offset) {
    return tokens[offset];
}


Token Parser::eat() {
    Token prev = tokens[0];
    tokens.erase(tokens.begin());
    return prev;
}

/* =======================
   STATEMENTS
   ======================= */

Stmt* Parser::ParseStmt() {
    std::cerr << "[PARSER] ParseStmt start, next token=" << peek().value << " type=" << (int)peek().type << "\n";
    // skip any comment tokens (lexer now removes them, but be safe)
    while (peek().type == TokenType::CommentLine || peek().type == TokenType::CommentBlock) {
        eat();
    }
    // variable declaration: int x = ...
    Stmt* stmt = nullptr;
    if(peek().type == TokenType::Keyword&&peek().value=="crclass"){ 
        stmt = ParseClassDecl();
    }
    else if(peek().type == TokenType::BlockKeyword && peek().value=="if"){
        stmt = parseIf();}
    else if (peek().type == TokenType::TypeIdent || 
       (peek().type == TokenType::Identifier && structNames.contains(peek().value)) ||
       (peek().type == TokenType::Keyword && peek().value == "mkimmutable")) {
        stmt= ParseVarDecl();
    }else{

    // fallback: expression statement
    stmt = ParseExpr();}
    if (peek().type == TokenType::Semicolon)
        eat();
    else{
        throw std::runtime_error("Syntax Error: Expected ';' at the end of statement, but found '" + peek().value + "'");
    }
    return stmt;
}
Stmt* Parser::ParseClassDecl() {
    eat(); // consume 'crclass'
    Token name = eat();
    if (name.type != TokenType::Identifier) {
        throw std::runtime_error("Expected identifier for class name in class declaration\n");
    }
    // record struct name for later variable declarations
    this->structNames.insert(name.value);

    BlockStmt* body = parseBlock();
    
    // Parse constructor if present
    ConstructorDecl* constr = nullptr;
    for (auto stmt : body->body) {
        if (stmt->kind == NodeType::CallExpression) {
            // Check if this is a constructor (starts with "constr")
            if (auto callExpr = dynamic_cast<CallExpr*>(stmt)) {
                if (auto ident = dynamic_cast<Identifier*>(callExpr->callee)) {
                    if (ident->name == "constr") {
                        constr = new ConstructorDecl();
                        // Extract parameter types from the call arguments
                        for (auto arg : callExpr->arguments) {
                            if (auto argIdent = dynamic_cast<Identifier*>(arg)) {
                                // Try to find the matching variable declaration to get type
                                for (auto checkStmt : body->body) {
                                    if (checkStmt->kind == NodeType::VariableDeclaration) {
                                        auto varDecl = dynamic_cast<VarDecl*>(checkStmt);
                                        if (varDecl->name == argIdent->name) {
                                            constr->params.push_back({argIdent->name, varDecl->type});
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    
    return new StructDecl(name.value, body, constr);
}
Stmt* Parser::parseIf() {
    Token control = eat();
    if(peek().type!=TokenType::Backslash){
        throw std::runtime_error("Expected opening '\\' for condition block\n");
        return new IfStmt(nullptr, nullptr);
    }
    eat(); // consume backslash
    Expr* condition = ParseExpr();

    if(condition==nullptr){
        throw std::runtime_error("Expected condition expression after block keyword\n");
        return new IfStmt(nullptr, nullptr);
    }
    if(peek().type!=TokenType::Backslash){
        throw std::runtime_error("Expected backslash\n");
        return new IfStmt(nullptr, nullptr);
    }
    eat();
    Stmt* body = parseBlock();
    if(body==nullptr){
        throw std::runtime_error("Expected block statement for if body\n");
        return new IfStmt(condition, nullptr);
    }
    // if(condition->kind==NodeType::Literal&&condition)


    // if(control.value=="if"&&condition->kind!=NodeType::Literal){
    //     throw std::runtime_error("Expected literal condition for if statement\n");
    //     return new BlockStmt();
    // }
    return new IfStmt(condition, static_cast<BlockStmt*>(body));

}

BlockStmt* Parser::parseBlock() {
    auto block = new BlockStmt();
    if(peek().type!=TokenType::LBracket){
        throw std::runtime_error("Expected opening '[' for block\n");
        return new BlockStmt();
    }else{
        eat(); // consume [
    }
    while(peek().type!=TokenType::RBracket){
        if(peek().type==TokenType::EndOfFile){
            throw std::runtime_error("Unexpected end of file while parsing block\n");
            return new BlockStmt();
        }
        block->body.push_back(ParseStmt());
    }
    if(peek().type!=TokenType::RBracket){
        throw std::runtime_error("Expected closing ']' for block\n");
        return new BlockStmt();
    }
    eat(); // consume ]
    return block;
}




Stmt* Parser::ParseVarDecl() {
    // consume type
    ValueType type;
    std::string structName = "";
    bool isc=false; //is constant
    Token tem;
    Token te1 = eat();
    //see if constant
    if(te1.type==TokenType::Keyword&&te1.value=="mkimmutable"){
        isc=true;
        eat(); //consume bslash
        tem = eat();
    }else{
        tem = te1;
    }
    if(tem.type==TokenType::TypeIdent) {
        if(tem.value=="int"){
            type=ValueType::Integer;
        }else if(tem.value=="int64"){
            type=ValueType::Integer;
        }else if(tem.value=="float"){
            type=ValueType::Float;
        }else if(tem.value=="float64"){
            type=ValueType::Float;
        }else if(tem.value=="str"){
            type=ValueType::String;
        }else if(tem.value=="bool"){
            type=ValueType::Bool;
        }
    } else if (tem.type==TokenType::Identifier && this->structNames.contains(tem.value)) {
        // treat identifier as struct type
        type = ValueType::Structure;
        structName = tem.value;
    } else {
        throw std::runtime_error("Expected type identifier in variable declaration\n");
        return new VarDecl(ValueType::Null,"INVALID");
    }

    //consume identifier
    Token name = eat();
    if (name.type != TokenType::Identifier) {
        throw std::runtime_error("Expected identifier in variable declaration\n");
        return new VarDecl(ValueType::Null,"INVALID");
    }

    Expr* initializer = nullptr;

    if (peek().type == TokenType::AssignmentOp) {
        eat(); // =
        initializer = ParseExpr();

    }
    if(isc&&peek().type==TokenType::Backslash){
        eat(); //consume bslash
    }else if(isc){
        throw std::runtime_error("Expected closing '\\' \n");
    }
    return new VarDecl(type, name.value, initializer, isc, structName);
}

 


/* =======================
   EXPRESSIONS
   ======================= */

Expr* Parser::ParseExpr() {    std::cerr << "[PARSER] ParseExpr\n";    return ParseBinaryExpr();
}

Expr* Parser::ParseBinaryExpr(int precedence) {
    Expr* left = ParsePrimExpr();
    std::cerr << "[PARSER] ParseBinaryExpr left kind=" << (left ? (int)left->kind : -1) << "\n";

    while (notEOF()) {
        int nextPrec = getPrecedence(peek());
        if (nextPrec < precedence)
            break;

        Token op = eat();
        Expr* right = (op.type == TokenType::AssignmentOp)? ParseBinaryExpr(nextPrec): ParseBinaryExpr(nextPrec + 1);

        if (op.type == TokenType::AssignmentOp) {
            left = new Assignment(left, right);
        } else {
            left = new BinaryExpr(left, right, op.value);
        }
    }

    return left;
}

Expr* Parser::ParsePrimExpr() {
    Token tok = eat();
    std::cerr << "[PARSER] ParsePrimExpr tok="<<tok.value<<" type="<<(int)tok.type<<"\n";

    Expr* expr = nullptr;

    switch (tok.type) {
        case TokenType::Identifier:
            expr = new Identifier(tok.value);
            break;
        case TokenType::StringLiteral:
        case TokenType::IntLiteral:
        case TokenType::FloatLiteral:
        case TokenType::BoolLiteral:
            expr = new Literal(tok.value);
            break;

        case TokenType::Null:
            expr = new NullLiteral();
            break;

        // grouping: bslahs expr 
        case TokenType::Backslash:
            expr = ParseExpr();
            if (peek().type != TokenType::Backslash) {
               throw std::runtime_error("Expected closing '\\' \n");
            } else {
                eat();
            }
            break;

        // default:
        //     throw std::runtime_error("UNEXPECTED TOKEN: "+tok.value+'\n');
        //     return new Literal("INVALID");
    }
    // // Handle postfix expressions: calls and indexing
    while (true) {
        // function call: function name bslash arg1 ; arg2 bslash
        if (peek().type == TokenType::Comma) {

            eat(); // ,
            Token fieldTok = eat();
            if(fieldTok.type != TokenType::Identifier){
                throw std::runtime_error("Expected field name in struct call\n");
            }
            string fieldName = fieldTok.value;
            expr = new CallStructExpr(expr, fieldName);
            continue;
        }

        if(peek().type == TokenType::Backslash){
            eat();
             vector<Expr*> args;

            while (peek().type != TokenType::Backslash) {
                args.push_back(ParseExpr());
                if (peek().type == TokenType::Semicolon)
                    eat();
            }

            eat(); // 
            expr = new CallExpr(expr, args);
            continue;
        }

        // indexing or potential constructor parameter list
        if (peek().type == TokenType::LBracket) {
            if (auto ident = dynamic_cast<Identifier*>(expr)) {
                // special-case 'constr' inside class body
                if (ident->name == "constr") {
                    eat(); // consume [
                    vector<Expr*> args;
                    while (peek().type != TokenType::RBracket) {
                        args.push_back(ParseExpr());
                        if (peek().type == TokenType::Semicolon) eat();
                    }
                    eat(); // consume ]
                    // represent as call expression for easier handling later
                    expr = new CallExpr(new Identifier("constr"), args);
                    continue;
                }
            }
            // regular indexing
            eat(); // [
            Expr* index = ParseExpr();
            if (peek().type != TokenType::RBracket) {
                throw std::runtime_error("Expected ]\n");
            } else {
                eat();
            }
            expr = new IndexExpr(expr, index);
            continue;
        }

        break;
    }
    return expr;
    }


/* =======================
   PRECEDENCE
   ======================= */

int Parser::getPrecedence(const Token& tok) {
    switch (tok.type) {
        case TokenType::LogicalOp:
            if (tok.value == "and") return 3;
            if (tok.value == "or") return 2;
            if (tok.value == "nt") return 4;
            return 0;
        case TokenType::ArithmeticOp:
            if (tok.value == "+" || tok.value == "-") return 10;
            if (tok.value == "*" || tok.value == "/") return 20;
            return 0;

        case TokenType::ComparisonOp:
            return 5;

        case TokenType::AssignmentOp:
            return 1;

        default:
            return -1;
    }
}
