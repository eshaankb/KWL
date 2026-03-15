/**
 * @file parser.cpp
 * @brief Parser implementation for the KWL interpreter.
 *
 * Implements the recursive descent parser that transforms a token stream
 * into an Abstract Syntax Tree (AST). Handles all KWL language constructs
 * including expressions, statements, functions, classes, and control flow.
 *
 * @author KWL Interpreter
 * @date 2026
 */

#include "parser.hpp"
#include"values.hpp"
#include<string>
#include<variant>
#include <stdexcept>

using namespace std;

Program Parser::produceAST(string sourcecode) {
    tokens = tokenize(sourcecode);

    Program program;

    int stmtCount = 0;
    while (notEOF()) {
        Stmt* s = ParseStmt();
        if (s == nullptr) {
        } else {
            program.body.push_back(s);
            stmtCount++;
        }
        if (stmtCount > 1000) {
            break;
        }
    }

    return program;
}

bool Parser::notEOF() {
    return pos < tokens.size() && tokens[pos].type != TokenType::EndOfFile;
}

Token& Parser::peek(int offset) {
    if (pos + offset >= tokens.size()) {
        return tokens.back();
    }
    return tokens[pos + offset];
}


Token Parser::eat() {
    Token t = tokens[pos++];
    return t;
}

void Parser::expect(TokenType type, const std::string& message) {
    if (peek().type != type) {
        std::string lineInfo = (peek().line > 0) ? " (line " + std::to_string(peek().line) + ")" : "";
        throw std::runtime_error(message + lineInfo);
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
    #ifdef DEBUG_MODE
    std::cerr << "[DEBUG] Token: " << peek().value << " Type: " << (int)peek().type << "\n";
    if (structNames.find(peek().value) != structNames.end()) std::cerr << " (matches struct)";
    std::cerr << "\n";
    #endif
    // skip any comment tokens (lexer now removes them, but be safe)
    while (notEOF() && (peek().type == TokenType::CommentLine || peek().type == TokenType::CommentBlock)) {
        eat();
    }
    // Reset parsing state for each new statement
    inArgumentList = false;
    groupingDepth = 0;
    // variable declaration: int x = ...
    Stmt* stmt = nullptr;
    // Handle import statement
    if (peek().type == TokenType::Keyword && peek().value == "import") {
        eat(); // consume 'import'
        expect(TokenType::Backslash, "Expected '\\' after 'import'");
        std::string path;
        while (notEOF() && peek().type != TokenType::Backslash) {
            if (peek().type == TokenType::Identifier || peek().type == TokenType::StringLiteral) {
                if (!path.empty()) path += ">";
                path += peek().value;
            } else if (peek().type == TokenType::ComparisonOp || peek().type == TokenType::ArithmeticOp) {
                if (!path.empty()) path += peek().value;
            } else {
                path += peek().value;
            }
            eat();
        }
        if (path.empty()) {
            throw std::runtime_error("Expected module path after 'import\\'");
        }
        expect(TokenType::Backslash, "Expected '\\' after module path");
        if (peek().type == TokenType::Keyword && peek().value == "with") {
            eat(); // consume 'with'
            std::string alias;
            if (peek().type == TokenType::Identifier || peek().type == TokenType::Keyword) {
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
    else if(peek().value=="if"){
        stmt = parseIf();}
    else if (peek().type == TokenType::BlockKeyword && peek().value == "lp") {
        stmt = ParseLp();
    }
    else if (peek().type == TokenType::TypeIdent) {
        if (peek(1).type == TokenType::Backtick) {
            stmt = ParseFunctionDecl();
        }else{
            stmt = ParseVarDecl();}
    }
    // Check for namespaced type: identifier , identifier identifier (e.g., ta,dog jim = ...)
    else if (peek().type == TokenType::Identifier && peek(1).type == TokenType::Comma && peek(2).type == TokenType::Identifier && peek(3).type == TokenType::Identifier) {
        stmt = ParseVarDecl();
    }
    else if (peek().type == TokenType::Identifier && structNames.find(peek().value) != structNames.end()) {
        stmt= ParseVarDecl();
    }
    else if (peek().type == TokenType::Keyword && peek().value == "mkimmutable") {
        stmt= ParseVarDecl();
    }
    else{
        //  fallback: expression statement
        stmt = ParseExpr();
        if (!stmt) { throw std::runtime_error("Unexpected token: " + peek().value); }
    }

    // Determine whether this statement type needs a trailing semicolon consumed.
    // Blocks (if/while/fn/struct) do not - their closing ']' already terminates them.
    // ExportDecl wraps another statement: a wrapped block-statement needs no ';',
    // but a wrapped variable/expression declaration does.
    auto needsSemicolon = [](Stmt* s) -> bool {
        if (s->kind == NodeType::IfStatement    ||
            s->kind == NodeType::WhileStatement ||
            s->kind == NodeType::ElseStatement  ||
            s->kind == NodeType::ImportStatement) return false;
        if (s->kind == NodeType::FunctionDeclaration ||
            s->kind == NodeType::StructureDeclaration) return false;
        if (s->kind == NodeType::ExportDeclaration) {
            // Only skip ';' if the wrapped decl itself doesn't need one
            auto* ex = static_cast<ExportDecl*>(s);
            if (ex->decl && (ex->decl->kind == NodeType::FunctionDeclaration ||
                             ex->decl->kind == NodeType::StructureDeclaration)) return false;
            return true; // wrapped var decl needs ';'
        }
        return true;
    };
    if (peek().type == TokenType::Semicolon && needsSemicolon(stmt)) {
        eat();
    } else if (needsSemicolon(stmt) && peek().type != TokenType::Semicolon) {
        throw std::runtime_error("Syntax Error: Expected ';' at the end of statement, but found '" + peek().value + "'");
    }
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
    eat(); // consume closing '\'
    if (peek().type == TokenType::Semicolon) eat(); // optional ';' after param list
    BlockStmt* body = parseBlock();
    FunctionDecl* decl = new FunctionDecl(fnName, params, body);
    return decl;
}

Expr* Parser::ParseFunctionCall(const string& fnName) {
    vector<Expr*> args;
    expect(TokenType::Backslash, "Expected '\\' after function name in call expression");
    while (peek().type != TokenType::Backslash) {
        if (peek().type == TokenType::EndOfFile) {
            throw std::runtime_error("Unclosed function call: missing closing '\\'");
        }
        args.push_back(ParseExpr());
        if (peek().type == TokenType::Semicolon) eat();
    }
    eat(); // consume closing backslash
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
            else if (typeTok.value == "int64") fieldType = ValueType::Integer64;
            else if (typeTok.value == "str") fieldType = ValueType::String;
            else if (typeTok.value == "fl") fieldType = ValueType::Float;
            else if (typeTok.value == "fl64") fieldType = ValueType::Float64;
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
    eat(); // tobj
    string className = eat().value;
    structNames.insert(className);
    expect(TokenType::LBracket, "Expected '[' after tobj name");
    
    vector<pair<string, ValueType>> fields;
    vector<pair<string, FunctionDecl>> methods;
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
            if(peek().type != TokenType::TypeIdent) {
                throw runtime_error("Expected type identifier");
                return nullptr;
            } 
            if(peek(1).type != TokenType::Backtick) {
            Token typeTok = eat();
            ValueType fieldType;
            if (typeTok.value == "int") fieldType = ValueType::Integer;
            else if (typeTok.value == "int64") fieldType = ValueType::Integer64;
            else if (typeTok.value == "fl") fieldType = ValueType::Float;
            else if (typeTok.value == "fl64") fieldType = ValueType::Float64;
            else if (typeTok.value == "str") fieldType = ValueType::String;
            else if (typeTok.value == "bool") fieldType = ValueType::Bool;
            else throw runtime_error("Unknown field type: " + typeTok.value);
            
            Token nameTok = eat();
            if (nameTok.type != TokenType::Identifier) {
                throw runtime_error("Expected field name after type");
            }
            fields.push_back({nameTok.value, fieldType});
            expect(TokenType::Semicolon, "Expected ';' after field declaration");
            }else{
                // method
                FunctionDecl* methodDecl = static_cast<FunctionDecl*>(ParseFunctionDecl());
                methods.push_back({methodDecl->name, *methodDecl});
            }
        }
    }

    eat(); // consume closing ]
    return new TrueObj(className, fields, methods, new ConstructorDecl(constrParams));

}
Stmt* Parser::parseIf() {
    Token control = eat();
    if(peek().type!=TokenType::Backslash){
        throw std::runtime_error("Expected opening '\\' for condition block\n");
        return new IfStmt(nullptr, nullptr);
    }
    eat(); // consume backslash
    groupingDepth++;
    Expr* condition = ParseBinaryExpr();
    groupingDepth--;

    if(condition==nullptr){
        throw std::runtime_error("Expected condition expression after block keyword\n");
        return new IfStmt(nullptr, nullptr);
    }
    // Accept either backslash or LBracket after condition (KWL syntax allows either)
    if(peek().type!=TokenType::Backslash && peek().type!=TokenType::LBracket){
        throw std::runtime_error("Expected backslash or '[' after condition\n");
        return new IfStmt(nullptr, nullptr);
    }
    if (peek().type == TokenType::Backslash) {
        eat(); // consume optional closing backslash
    }
    Stmt* body = parseBlock();
    if(body==nullptr){
        throw std::runtime_error("Expected block statement for if body\n");
        return new IfStmt(condition, nullptr);
    }
    ElseStmt* elseBranch = nullptr;
    if(peek().type==TokenType::BlockKeyword&&peek().value=="el"){
        elseBranch = static_cast<ElseStmt*>(ParseEl());
    }
    // if(condition->kind==NodeType::Literal&&condition)


    // if(control.value=="if"&&condition->kind!=NodeType::Literal){
    //     throw std::runtime_error("Expected literal condition for if statement\n");
    //     return new BlockStmt();
    // }
    return new IfStmt(condition, static_cast<BlockStmt*>(body), elseBranch);

}
Stmt* Parser::ParseEl(){
    eat(); // consume 'else'
    if(peek().type==TokenType::BlockKeyword&&peek().value=="if"){
        IfStmt* ifBranch = static_cast<IfStmt*>(parseIf());
        return new ElseStmt(ifBranch);
    }else{
        Stmt* body = parseBlock();
        if(body==nullptr){
            throw std::runtime_error("Expected block statement for else body\n");
            return new BlockStmt();
        }
        return new ElseStmt(body);
    }
}

BlockStmt* Parser::parseBlock() {
    auto block = new BlockStmt();
    // Reset state when entering a new block to prevent stale grouping state
    groupingDepth = 0;
    inArgumentList = false;
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
            type=ValueType::Integer64;
        }else if(tem.value=="fl"){
            type=ValueType::Float;
        }else if(tem.value=="fl64"){
            type=ValueType::Float64;
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
    } else if (tem.type==TokenType::Identifier && this->structNames.find(tem.value) != this->structNames.end()) {
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
    if (peek().type == TokenType::Keyword && peek().value == "gbl") {
        eat(); // skip the 'gbl' storage modifier
    }
    Token name = eat();
    // Accept TypeIdent too - "fl", "int", "str" etc. are valid variable names
    if (name.type != TokenType::Identifier && name.type != TokenType::TypeIdent) {
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
    // TODO: Pass moduleAlias to VarDecl if needed for module-namespaced types
    return new VarDecl(type, name.value, initializer, isc, structName);
}

 


/* =======================
   EXPRESSIONS
   ======================= */
Expr* Parser::ParseExpr() {  
    #ifdef DEBUG_MODE 
    std::cerr << "[PARSER] ParseExpr\n";  
    #endif
    
    return ParseBinaryExpr();
}

Expr* Parser::ParseBinaryExpr(int precedence) {
    Expr* left = ParseUnaryExpr();
    #ifdef DEBUG_MODE
    if (!left) { std::cerr << "[BINEXPR-NULL-LEFT] prec=" << precedence << "\n"; return nullptr; }
    #endif
    while (notEOF()) {
        if (peek().type == TokenType::Backslash) {
            break;
        }


        int nextPrec = getPrecedence(peek());
        if (nextPrec < precedence)
            break;

        Token op = eat();
        Expr* right = (op.type == TokenType::AssignmentOp)? ParseBinaryExpr(nextPrec): ParseBinaryExpr(nextPrec + 1);

        if (op.type == TokenType::AssignmentOp) {
            if (op.value.length() == 2 && op.value[1] == '=') {
                char baseOp = op.value[0];
                if (baseOp == '+' || baseOp == '-' || baseOp == '*' || baseOp == '/' || baseOp == '%') {
                    std::string binOp(1, baseOp);
                    Expr* combined = new BinaryExpr(left, right, binOp);
                    left = new Assignment(left, combined);
                } else {
                    left = new Assignment(left, right);
                }
            } else {
                left = new Assignment(left, right);
            }
        } else {
            left = new BinaryExpr(left, right, op.value);
        }
    }

    return left;
}

Expr* Parser::ParseUnaryExpr() {
    if (peek().type == TokenType::ArithmeticOp && peek().value == "-") {
        eat();
        Expr* operand = ParseUnaryExpr();
        return new UnaryExpr(operand, "-");
    }
    if (peek().type == TokenType::LogicalOp && peek().value == "nt") {
        eat();
        Expr* operand = ParseUnaryExpr();
        return new UnaryExpr(operand, "nt");
    }
    return ParsePrimExpr();
}

Expr* Parser::ParseArrayLiteral() {
    eat(); // consume '['

    vector<Expr*> elements;

    if (peek().type == TokenType::RBracket) {
        eat(); // empty array
        return new ArrayLiteral(elements);
    }

    // first element determines array type
    TokenType elementType = peek().type;

    if (!(elementType == TokenType::IntLiteral ||
          elementType == TokenType::FloatLiteral ||
          elementType == TokenType::StringLiteral ||
          elementType == TokenType::BoolLiteral)) {
        throw std::runtime_error("Invalid start of array literal");
    }

    while (notEOF() && peek().type != TokenType::RBracket) {

        Token elemTok = eat();

        if (elemTok.type != elementType) {
            throw std::runtime_error("Array elements must be the same type");
        }

        elements.push_back(new Literal(elemTok.value));

        if (peek().type == TokenType::Semicolon) {
            eat();
        } else if (peek().type != TokenType::RBracket) {
            throw std::runtime_error("Expected ';' or ']' in array literal");
        }
    }

    expect(TokenType::RBracket, "Expected ']' after array literal");

    return new ArrayLiteral(elements);
}

Expr* Parser::ParsePrimExpr() {
    Token tok = eat();

    if (tok.type == TokenType::EndOfFile) {
        return nullptr;
    }
    Expr* expr = nullptr;

    switch (tok.type) {
        case TokenType::Identifier: {
            // Lookahead: `\` after an identifier is a FUNCTION CALL opener only when the
            // token after that `\` is a value/expression token (i.e., there are arguments).
            // If peek(1) is a terminator (`;`, `]`, `\`, operator) the `\` is a closing
            // delimiter belonging to a surrounding grouping - treat identifier as a variable.
            bool isFunctionCall = false;
            if (peek().type == TokenType::Backslash) {
                TokenType nextNext = peek(1).type;
                bool p1IsValue = (nextNext == TokenType::Identifier   ||
                                  nextNext == TokenType::Keyword       ||
                                  nextNext == TokenType::TypeIdent     ||
                                  nextNext == TokenType::IntLiteral    ||
                                  nextNext == TokenType::FloatLiteral  ||
                                  nextNext == TokenType::StringLiteral ||
                                  nextNext == TokenType::BoolLiteral   ||
                                  nextNext == TokenType::Null          ||
                                  nextNext == TokenType::LogicalOp);   // ArithmeticOp removed: +/-/* after \ = closing delimiter
                // \  after identifier = zero-arg call only at top level (not inside grouping)
                bool p1IsZeroArgCall = (nextNext == TokenType::Backslash && groupingDepth == 0);
                isFunctionCall = p1IsValue || p1IsZeroArgCall;
            }
            if (isFunctionCall) {
                if (structNames.count(tok.value)) {
                    // Constructor call - parse args then wrap as ConstructorCallExpr
                    FunctionCall* fc = static_cast<FunctionCall*>(ParseFunctionCall(tok.value));
                    expr = new ConstructorCallExpr(tok.value, fc->arguments);
                } else {
                    expr = ParseFunctionCall(tok.value);
                }
            } else {
                expr = new Identifier(tok.value);
            }
            break;
        }
        case TokenType::StringLiteral:
        case TokenType::IntLiteral:
        case TokenType::FloatLiteral:
        case TokenType::BoolLiteral:
            expr = new Literal(tok.value);
            break;

        case TokenType::Null:
            expr = new NullLiteral();
            break;

        case TokenType::Keyword: {
            // Keywords in expression context are treated as identifiers.
            // Covers module aliases (log, wt, rd) and names the lexer over-classifies.
            bool kwIsCall = false;
            if (peek().type == TokenType::Backslash) {
                TokenType nn = peek(1).type;
                bool nnIsValue = (nn == TokenType::Identifier   ||
                                  nn == TokenType::Keyword       ||
                                  nn == TokenType::TypeIdent     ||
                                  nn == TokenType::IntLiteral    ||
                                  nn == TokenType::FloatLiteral  ||
                                  nn == TokenType::StringLiteral ||
                                  nn == TokenType::BoolLiteral   ||
                                  nn == TokenType::Null          ||
                                  nn == TokenType::LogicalOp);   // ArithmeticOp removed
                bool nnIsZeroArgCall = (nn == TokenType::Backslash && groupingDepth == 0);
                kwIsCall = nnIsValue || nnIsZeroArgCall;
            }
            if (kwIsCall) {
                expr = ParseFunctionCall(tok.value);
            } else {
                expr = new Identifier(tok.value);
            }
            break;
        }

        // grouping: bslash expr bslash
        case TokenType::Backslash: {
            // For grouping, suppress postfix handlers - use counter for nesting
            groupingDepth++;
            expr = ParseExpr();
            // After expression, expect closing backslash or other terminators
            if (peek().type == TokenType::Backslash) {
                eat(); // consume closing backslash
                groupingDepth--;
            } else if (peek().type == TokenType::RBracket) {
                // RBracket can end grouping in if-blocks - don't consume it
                groupingDepth--;
            } else if (peek().type == TokenType::Semicolon) {
                // Semicolon can end grouping - don't consume it
                groupingDepth--;
            } else if (peek().type == TokenType::BlockKeyword) {
                // Block keyword (like 'if', 'el') can end grouping
                groupingDepth--;
            } else {
                throw std::runtime_error("Expected closing '\\', RBracket, Semicolon, or block keyword after grouping, got: " + peek().value);
            }
            break;
        }

        case TokenType::LBracket: {
            // Note: '[' was already consumed by tok=eat() above.
            // peek() is now the token AFTER '['.
            // Treat as ARRAY LITERAL only if the first element is a plain value.
            // Anything else (Identifier, TypeIdent, Keyword, return, etc.) means
            // this '[' is a BLOCK belonging to an if/lp/fn - put it back.
            TokenType p0 = peek().type;
            bool looksLikeArray = (p0 == TokenType::IntLiteral    ||
                                   p0 == TokenType::FloatLiteral   ||
                                   p0 == TokenType::StringLiteral  ||
                                   p0 == TokenType::BoolLiteral    ||
                                   p0 == TokenType::RBracket);      // empty array []
            if (!looksLikeArray) {
                // put '[' back so parseBlock can consume it
                pos--;
                return nullptr;
            }
            // parse array literal - put '[' back first so ParseArrayLiteral eats it
            pos--;
            return ParseArrayLiteral();
        }

        // default:
        //     throw std::runtime_error("UNEXPECTED TOKEN: "+tok.value+'\n');
        //     return new Literal("INVALID");
    }
    // // Handle postfix expressions: calls and indexing
    while (notEOF()) {
        // A backslash is a CALL OPENER (don't break) if peek(1) is an argument-starting token.
        // Otherwise it is a closing delimiter and we should break.
        auto isArgStart = [&](TokenType t) {
            return t == TokenType::Identifier   ||
                   t == TokenType::Keyword       ||
                   t == TokenType::TypeIdent     ||
                   t == TokenType::IntLiteral    ||
                   t == TokenType::FloatLiteral  ||
                   t == TokenType::StringLiteral ||
                   t == TokenType::BoolLiteral   ||
                   t == TokenType::Null          ||
                   t == TokenType::LogicalOp     ||
                   t == TokenType::ArithmeticOp;
        };
        // `\` after an identifier is a call opener only when:
        //   - peek(1) is a true value-start token (args follow), OR
        //   - peek(1) is also `\` AND we are NOT inside a grouping (zero-arg call: foo\)
        // Inside a grouping `\` means two successive closing delimiters, not a call.
        // A backslash is a call opener only when:
        // 1. expr is something callable (Identifier or member access, NOT an already-resolved call/expression)
        // 2. peek(1) is an argument-starting token (args follow)
        // 3. OR peek(1) is also \ AND we are at top-level AND expr is a plain Identifier (zero-arg: foo\)
        // This prevents sin\n\ / cos\n\ from treating the closing \ as a new call to sin.
        bool exprIsIdentifier = expr && expr->kind == NodeType::Identifier;
        bool exprIsCallable = expr && (expr->kind == NodeType::Identifier ||
                                       expr->kind == NodeType::CallExpression);
        bool bslashIsCallOpener = false;
        if (peek().type == TokenType::Backslash && exprIsCallable) {
            TokenType p1 = peek(1).type;
            // ArithmeticOp or ComparisonOp after \ means the \ is closing a grouping,
            // not opening a call — operators follow a closed expression, not arg lists.
            // Only treat as call opener when p1 is a genuine value-start token.
            bool p1StartsValue = (p1 == TokenType::Identifier   ||
                                  p1 == TokenType::Keyword       ||
                                  p1 == TokenType::TypeIdent     ||
                                  p1 == TokenType::IntLiteral    ||
                                  p1 == TokenType::FloatLiteral  ||
                                  p1 == TokenType::StringLiteral ||
                                  p1 == TokenType::BoolLiteral   ||
                                  p1 == TokenType::Null          ||
                                  p1 == TokenType::LogicalOp);   // unary 'nt'
            // unary minus: only treat as arg-start when NOT inside a grouping
            // (inside grouping, \ followed by -expr means closing + unary on next expr)
            bool p1IsUnaryMinus = (p1 == TokenType::ArithmeticOp && peek(1).value == "-");
            if (p1StartsValue || (p1IsUnaryMinus && groupingDepth == 0)) {
                bslashIsCallOpener = true;
            } else if (p1 == TokenType::Backslash && groupingDepth == 0 && exprIsCallable) {
                bslashIsCallOpener = true; // zero-arg call (e.g. identifier with no args)
            }
        }
        // A non-opener \ always closes - even inside a grouping context.
        // The groupingDepth counter prevents INNER expressions from being mistaken
        // as calls; here we are at the postfix level of the CURRENT expression and
        // a \ that is not a call opener means "done with this expression".
        if (!inArgumentList && (peek().type == TokenType::RBracket ||
                                 peek().type == TokenType::Semicolon ||
                                 peek().type == TokenType::LBracket  ||
                                 (peek().type == TokenType::Backslash && !bslashIsCallOpener))) {
            break;
        }
        // function call: function name bslash arg1 ; arg2 bslash
        if (peek().type == TokenType::Comma) {

            eat(); // ,
            Token fieldTok = eat();
            if(fieldTok.type != TokenType::Identifier && fieldTok.type != TokenType::Keyword){
                throw std::runtime_error("Expected field name in struct call\n");
            }
            string fieldName = fieldTok.value;
            expr = new CallStructExpr(expr, fieldName);
            continue;
        }

        if (peek().type == TokenType::Backslash && !inArgumentList && exprIsCallable && peek(1).type != TokenType::LBracket) {
            eat();
            vector<Expr*> args;

            // read arguments until closing backslash
            int savedGroupingDepth = groupingDepth;  // restore after call, don't zero global state
            inArgumentList = true;
            while (notEOF()) {
                #ifdef DEBUG_MODE
                std::cerr << "[PARSER] In backslash arg loop, peek=" << peek().value << " type=" << (int)peek().type << "\n"; 
                #endif
                bool foundClBslash = false;
                if (peek().type == TokenType::Backslash) {
                    eat();
                    foundClBslash = true;
                    break;
                }
                args.push_back(ParseExpr());
                if (peek().type == TokenType::Semicolon && !foundClBslash) {
                    eat();
                    continue;
                }
            }
            inArgumentList = false;
            groupingDepth = savedGroupingDepth;  // restore — don't clobber outer grouping context

            if (auto ident = dynamic_cast<Identifier*>(expr)) {
                if (structNames.count(ident->name)) {
                    expr = new ConstructorCallExpr(ident->name, args);
                } else {
                    expr = new FunctionCall(ident->name, args);
                }
            } else {
                expr = new CallExpr(expr, args);
            }
            continue;
        }

        // indexing or potential constructor parameter list
        // Only enter if we have an expression to index and next token is [
        if (peek().type == TokenType::LBracket && expr != nullptr) {
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
   LP (LOOP) STATEMENT
   lp [ body ]              -- infinite loop
   lp \cond\ [ body ]       -- conditional (while) loop
   ======================= */
Stmt* Parser::ParseLp() {
    eat(); // consume 'lp'

    // Infinite loop: lp [ ... ]
    if (peek().type == TokenType::LBracket) {
        BlockStmt* body = parseBlock();
        // Represent as WhileStmt with literal true condition
        return new WhileStmt(new Literal("true"), body);
    }

    // Conditional loop: lp \cond\ [ ... ]
    if (peek().type == TokenType::Backslash) {
        eat(); // consume opening '\'
        groupingDepth++;
        Expr* condition = ParseBinaryExpr();
        groupingDepth--;
        if (peek().type == TokenType::Backslash) eat(); // consume closing '\'
        BlockStmt* body = parseBlock();
        return new WhileStmt(condition, body);
    }

    throw std::runtime_error("Syntax Error: Expected '[' or '\\' after 'lp'");
    return nullptr;
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
            if (tok.value == "**") return 25;          // exponentiation: tighter than * and /
            if (tok.value == "*" || tok.value == "/") return 20;
            return 0;
        case TokenType::Keyword:
            if (tok.value == "mod") return 20;
            return 0;

        case TokenType::ComparisonOp:
            return 5;

        case TokenType::AssignmentOp:
            return 1;

        default:
            return -1;
    }
}