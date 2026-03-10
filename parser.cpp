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
            std::cerr << "[PARSER] ParseStmt returned nullptr, skipping\n";
        } else {
            program.body.push_back(s);
            stmtCount++;
        }
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

void Parser::expect(TokenType type, const std::string& message) {
    if (peek().type != type) {
        throw std::runtime_error(message);
    }
    eat();
}

/* =======================
   STATEMENTS
   ======================= */

Stmt* Parser::ParseStmt() {
    if (!notEOF()) {
        return nullptr;
    }
    std::cerr << "[PARSER] ParseStmt start, next token=" << peek().value << " type=" << (int)peek().type << "\n";
    std::cerr << "[PARSER] structNames size=" << structNames.size();
    std::cerr << "[DEBUG] Token: " << peek().value << " Type: " << (int)peek().type << "\n";
    if (structNames.contains(peek().value)) std::cerr << " (matches struct)";
    std::cerr << "\n";
    // skip any comment tokens (lexer now removes them, but be safe)
    while (notEOF() && (peek().type == TokenType::CommentLine || peek().type == TokenType::CommentBlock)) {
        eat();
    }
    // variable declaration: int x = ...
    Stmt* stmt = nullptr;
    // Handle import statement
    if (peek().type == TokenType::Keyword && peek().value == "import") {
        eat(); // consume 'import'
        expect(TokenType::Backslash, "Expected '\\' after 'import'");
        std::string path;
        if (peek().type == TokenType::Identifier || peek().type == TokenType::StringLiteral) {
            path = eat().value;
        } else {
            throw std::runtime_error("Expected module path after 'import\\'");
        }
        expect(TokenType::Backslash, "Expected '\\' after module path");
        if (peek().type == TokenType::Keyword && peek().value == "with") {
            eat(); // consume 'with'
            std::string alias;
            if (peek().type == TokenType::Identifier) {
                alias = eat().value;
            } else {
                throw std::runtime_error("Expected alias after 'with'");
            }
            stmt = new ImportStmt(path, alias, false);
        } else if (peek().type == TokenType::Keyword && peek().value == "all") {
            eat(); // consume 'all'
            stmt = new ImportStmt(path, "", true);
        } else {
            throw std::runtime_error("Expected 'with' or 'all' after import path");
        }
        expect(TokenType::Semicolon, "Expected ';' after import statement");
    }
    else if (peek().type == TokenType::Keyword && peek().value == "pub") {
        eat(); // consume 'pub'
        if (peek().type == TokenType::TypeIdent && peek(1).type == TokenType::Backtick) {
            stmt = new ExportDecl(ParseFunctionDecl());
        } else if (peek().type == TokenType::TypeIdent) {
            stmt = new ExportDecl(ParseVarDecl());
        } else if (peek().type == TokenType::Keyword && peek().value == "crclass") {
            stmt = new ExportDecl(ParseClassDecl());
        } else {
            throw std::runtime_error("Expected declaration after 'pub'");
        }
    }
    else if(peek().type == TokenType::Keyword&&peek().value=="crclass"){ 
        stmt = ParseClassDecl();
    }else if(peek().type == TokenType::Keyword&&peek().value=="tobj"){
        stmt = ParseTrueClassDecl();
    }
    else if(peek().type == TokenType::Keyword && peek().value == "return") {
        stmt = ParseReturn();
    }
    else if(peek().type == TokenType::BlockKeyword && peek().value=="if"){
        stmt = parseIf();}
    else if (peek().type == TokenType::TypeIdent) {
        std::cerr << "[PARSER] Matched TypeIdent\n";
        if (peek(1).type == TokenType::Backtick) {
            stmt = ParseFunctionDecl();
        }else{
            stmt = ParseVarDecl();}
    }
    // Check for namespaced type: identifier , identifier identifier (e.g., ta,dog jim = ...)
    else if (peek().type == TokenType::Identifier && peek(1).type == TokenType::Comma && peek(2).type == TokenType::Identifier && peek(3).type == TokenType::Identifier) {
        std::cerr << "[PARSER] Matched namespaced type: " << peek().value << "," << peek(2).value << "\n";
        stmt = ParseVarDecl();
    }
    else if (peek().type == TokenType::Identifier && structNames.contains(peek().value)) {
        std::cerr << "[PARSER] Matched Identifier struct: " << peek().value << "\n";
        stmt= ParseVarDecl();
    }
    else if (peek().type == TokenType::Keyword && peek().value == "mkimmutable") {
        std::cerr << "[PARSER] Matched mkimmutable\n";
        stmt= ParseVarDecl();
    }
    else{
        //  fallback: expression statement
        std::cerr << "[PARSER] ParseStmt falling through to ParseExpr, peek type=" << (int)peek().type << " value=" << peek().value << "\n";
        stmt = ParseExpr();
    }

    if (peek().type == TokenType::Semicolon&&(!(stmt->kind==NodeType::IfStatement||stmt->kind==NodeType::FunctionDeclaration||stmt->kind==NodeType::StructureDeclaration))) {
        std::cerr << "[PARSER] ParseStmt eating semicolon\n";
        eat();
    } else if(!(stmt->kind==NodeType::IfStatement||stmt->kind==NodeType::FunctionDeclaration||stmt->kind==NodeType::StructureDeclaration)){
        throw std::runtime_error("Syntax Error: Expected ';' at the end of statement, but found '" + peek().value + "'");
    }
    std::cerr << "[PARSER] ParseStmt returning\n";
    return stmt;
}
Stmt* Parser::ParseFunctionDecl() {
    eat(); // consume return type
    eat(); // consume `
    if(peek().type != TokenType::Keyword || peek().value != "fn") {
        throw std::runtime_error("Expected fn after type for function declaration");
        return nullptr;
    }
    eat(); // consume fn
    string fnName = eat().value;
    expect(TokenType::Backslash, "Expected '\\' after function name");
    vector<VarDecl*> params;
    while (peek().type != TokenType::Backslash) {
        params.push_back(static_cast<VarDecl*>(ParseVarDecl()));
        if (peek().type == TokenType::Semicolon) eat();
    }
    eat(); // consume ']'
    BlockStmt* body = parseBlock();
    FunctionDecl* decl = new FunctionDecl(fnName, params, body);
    return decl;
}

Expr* Parser::ParseFunctionCall(const string& fnName) {
    vector<Expr*> args;
    expect(TokenType::Backslash, "Expected '\\' after function name in call expression");
    while (peek().type != TokenType::Backslash) {
        args.push_back(ParseExpr());
        if (peek().type == TokenType::Semicolon) eat();
    }
    eat(); // consume ']'
    return new FunctionCall(fnName, args);
}

Expr* Parser::ParseReturn(){
    eat(); // consume return keyword
    Expr* value = ParseExpr();
    return new ReturnStmt(value);
}

Stmt* Parser::ParseClassDecl() {
    eat(); // crclass
    string className = eat().value;
    structNames.insert(className);
    
    expect(TokenType::LBracket, "Expected '[' after class name");
    
    vector<pair<string, ValueType>> fields;
    vector<pair<string, ValueType>> constrParams;
    
    while (notEOF() && peek().type != TokenType::RBracket) {
        if (peek().value == "constr") {
            eat(); // constr
            expect(TokenType::LBracket, "Expected '[' for constructor params");
            while (peek().type != TokenType::RBracket) {
                Token paramName = eat();
                if (paramName.type != TokenType::Identifier) {
                    throw runtime_error("Expected parameter name in constructor");
                }
                constrParams.push_back({paramName.value, ValueType::Null}); // Placeholder; you can infer types later if needed
                if (peek().type == TokenType::Semicolon) eat();
            }
            eat(); // ]
            expect(TokenType::Semicolon, "Expected ';' after constr[...] ");
        } else {
            // Parse field: type name;
            Token typeTok = eat();
            ValueType fieldType;
            if (typeTok.value == "int") fieldType = ValueType::Integer;
            else if (typeTok.value == "str") fieldType = ValueType::String;
            else if (typeTok.value == "float") fieldType = ValueType::Float;
            else if (typeTok.value == "bool") fieldType = ValueType::Bool;
            else throw runtime_error("Unknown field type: " + typeTok.value);
            
            Token nameTok = eat();
            if (nameTok.type != TokenType::Identifier) {
                throw runtime_error("Expected field name after type");
            }
            fields.push_back({nameTok.value, fieldType});
            expect(TokenType::Semicolon, "Expected ';' after field declaration");
        }
    }
    eat(); // ]
    //StructDecl(const string& n, BlockStmt* init = nullptr, ConstructorDecl* constr = nullptr, bool immut = false) 
    StructDecl* decl = new StructDecl(className, fields);
    return decl;
}
 
Stmt* Parser::ParseTrueClassDecl() {

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
    std::string moduleAlias = "";  // for namespaced types like ta,dog
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
        }else if(tem.value=="fl"){
            type=ValueType::Float;
        }else if(tem.value=="fl64"){
            type=ValueType::Float;
        }else if(tem.value=="str"){
            type=ValueType::String;
        }else if(tem.value=="bool"){
            type=ValueType::Bool;
        }else if(tem.value=="arr"){
            type=ValueType::Array;
            // Handle arr\type\ syntax
            if(peek().type == TokenType::Backslash) {
                eat(); // consume backslash
                // Get the element type
                if(peek().type == TokenType::TypeIdent) {
                    // Store element type info - for now just consume it
                    Token elemType = eat();
                    // Could store element type in a separate field if needed
                }
                if(peek().type == TokenType::Backslash) {
                    eat(); // consume closing backslash
                } else {
                    throw std::runtime_error("Expected closing '\\' after array element type\n");
                }
            }
        }
    } else if (tem.type==TokenType::Identifier && this->structNames.contains(tem.value)) {
        // treat identifier as struct type
        type = ValueType::Structure;
        structName = tem.value;
    } else if (tem.type==TokenType::Identifier && peek().type==TokenType::Comma) {
        // Handle namespaced type: module_alias,class_name
        moduleAlias = tem.value;
        eat(); // consume comma
        Token className = eat();
        if(className.type != TokenType::Identifier) {
            throw std::runtime_error("Expected class name after namespace comma\n");
        }
        type = ValueType::Structure;
        structName = className.value;
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
    std::cerr << "[PARSER] ParseVarDecl returning, peek()=" << peek().value << " type=" << (int)peek().type << "\n";
    // TODO: Pass moduleAlias to VarDecl if needed for module-namespaced types
    return new VarDecl(type, name.value, initializer, isc, structName);
}

 


/* =======================
   EXPRESSIONS
   ======================= */

Expr* Parser::ParseExpr() {    std::cerr << "[PARSER] ParseExpr\n";    return ParseBinaryExpr();
}

Expr* Parser::ParseBinaryExpr(int precedence) {
    Expr* left = ParsePrimExpr();
    if (!left) return nullptr;
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

    if (tok.type == TokenType::EndOfFile) {
        return nullptr;
    }
    Expr* expr = nullptr;

    switch (tok.type) {
        case TokenType::Identifier:
            if(peek(1).type == TokenType::Backslash) {
                // function call
                expr = ParseFunctionCall(tok.value);
            } else {
                // variable reference
            expr = new Identifier(tok.value);}
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

        case TokenType::LBracket: {
            vector<Expr*> elements;
            // Check if this is a range [start:end]
            if (peek().type == TokenType::IntLiteral && peek(1).type == TokenType::RangeOp) {
                Token startTok = eat();
                eat(); // consume :
                Token endTok = eat();
                Expr* startExpr = new Literal(startTok.value);
                Expr* endExpr = new Literal(endTok.value);
                if (peek().type == TokenType::RBracket) {
                    eat(); // consume ]
                }
                expr = new RangeExpr(startExpr, endExpr);
                break;
            }
            TokenType ArrayType;
            // Otherwise, parse array literal
            if(peek().type==TokenType::RBracket){
                // empty array
                eat(); // consume ]
                expr = new ArrayLiteral(elements);
                break;
            }else if(peek().type==TokenType::IntLiteral||peek().type==TokenType::FloatLiteral||peek().type==TokenType::StringLiteral||peek().type==TokenType::BoolLiteral){
                ArrayType=peek().type;
            }
            while (notEOF() && peek().type != TokenType::RBracket) {
                // Parse array element - handle literal only for simple case
                Token elemTok = eat();
                if(elemTok.type != ArrayType) {
                    if(elemTok.type == TokenType::IntLiteral || elemTok.type == TokenType::FloatLiteral|| elemTok.type == TokenType::StringLiteral || elemTok.type == TokenType::BoolLiteral) {
                            throw std::runtime_error("Array elements must be of the same type");
                    } else {
                   throw std::runtime_error("Unexpected token in array: " + elemTok.value);}
                }
                if (peek().type == TokenType::Semicolon) {
                    eat(); // consume semicolon
                }else if (peek().type != TokenType::RBracket) {
                    throw std::runtime_error("Expected ';' or ']' in array literal");
                }else{
                    // allow missing semicolon for last element
                }
            }
            if (peek().type == TokenType::RBracket) {
                eat(); // consume ]
            }
            expr = new ArrayLiteral(elements);
            break;
        }

        // default:
        //     throw std::runtime_error("UNEXPECTED TOKEN: "+tok.value+'\n');
        //     return new Literal("INVALID");
    }
    // // Handle postfix expressions: calls and indexing
    while (notEOF()) {
        std::cerr << "[PARSER] In postfix loop, peek=" << peek().value << " type=" << (int)peek().type << "\n";
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

        if(peek().type == TokenType::Backslash && !inArgumentList){
            std::cerr << "[PARSER] Found backslash in postfix for expr: " << (expr ? (int)expr->kind : -1) << "\n";
            eat();
            vector<Expr*> args;

            // read arguments until closing backslash
            bool prevInArgumentList = inArgumentList;
            inArgumentList = true;
            while (notEOF()) {
                std::cerr << "[PARSER] In backslash arg loop, peek=" << peek().value << " type=" << (int)peek().type << "\n";
                if (peek().type == TokenType::Backslash) {
                    std::cerr << "[PARSER] Found closing backslash, eating and breaking\n";
                    eat(); // consume closing backslash
                    break;
                }
                std::cerr << "[PARSER] About to parse arg expression\n";
                args.push_back(ParseExpr());
                std::cerr << "[PARSER] Parsed arg, now peek=" << peek().value << " type=" << (int)peek().type << "\n";
                if (peek().type == TokenType::Semicolon) {
                    std::cerr << "[PARSER] Eating semicolon separator\n";
                    eat();
                    continue;
                }
                std::cerr << "[PARSER] No semicolon after arg, continuing\n";
                // if neither semicolon nor backslash, continue parsing next expr
            }
            inArgumentList = prevInArgumentList;
            std::cerr << "[PARSER] Exited backslash arg loop\n";
            std::cerr << "[PARSER] Exited backslash arg loop\n";

            if (auto ident = dynamic_cast<Identifier*>(expr)) {
                if (structNames.contains(ident->name)) {
                    expr = new ConstructorCallExpr(ident->name, args);
                } else {
                    expr = new CallExpr(expr, args);
                }
            } else {
                expr = new CallExpr(expr, args);
            }
            continue;
        }

        // indexing or potential constructor parameter list
        if (peek().type == TokenType::LBracket) {
            if (auto ident = dynamic_cast<Identifier*>(expr)) {
                // special-case 'constr' inside class body
                if (ident->name == "constr") {
                    eat(); // consume [
                    vector<Expr*> args;
                    while (notEOF() && peek().type != TokenType::RBracket) {
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
