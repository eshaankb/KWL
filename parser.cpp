#include "parser.hpp"
#include"values.hpp"
#include <stdexcept>

using namespace std;

Program Parser::produceAST(string sourcecode) {
    tokens = tokenize(sourcecode);

    Program program;

    while (notEOF()) {
        program.body.push_back(ParseStmt());
    }

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
    // variable declaration: int x = ...
    if (peek().type == TokenType::TypeIdent || 
       (peek().type == TokenType::Keyword && peek().value == "mkimmutable")) {
        return ParseVarDecl();
    }

    // fallback: expression statement
    Expr* expr = ParseExpr();

    if (peek().type == TokenType::Semicolon)
        eat();

    return expr;
}

Stmt* Parser::ParseVarDecl() {
    // consume type
    ValueType type;
    bool isc=false; //is constant
    if(true){
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
        if(tem.type!=TokenType::TypeIdent){

        }
        if(tem.value=="int"){
            type=ValueType::Integer;
        }if(tem.value=="int64"){
            type=ValueType::Integer;
        }if(tem.value=="float"){
            type=ValueType::Float;
        }if(tem.value=="float64"){
            type=ValueType::Float;
        }if(tem.value=="string"){
            type=ValueType::String;
        }
    //add size support for each
    }
    //consume identifier
    Token name = eat();
    if (name.type != TokenType::Identifier) {
        throw std::runtime_error("Expected identifier in variable declaration\n");
        return new VarDecl("INVALID");
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
    if (peek().type == TokenType::Semicolon)
        eat();
    else{
        throw std::runtime_error("Syntax Error: Expected ';' at the end of statement, but found '" + peek().value + "'");
    }
    return new VarDecl(name.value, initializer, isc);
}




/* =======================
   EXPRESSIONS
   ======================= */

Expr* Parser::ParseExpr() {
    return ParseBinaryExpr();
}

Expr* Parser::ParseBinaryExpr(int precedence) {
    Expr* left = ParsePrimExpr();

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

    Expr* expr = nullptr;

    switch (tok.type) {
        case TokenType::Identifier:
            expr = new Identifier(tok.value);
            break;

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

        default:
            throw std::runtime_error("UNEXPECTED TOKEN: "+tok.value+'\n');
            return new Literal("INVALID");
    }
    // Handle postfix expressions: calls and indexing
    while (true) {
        // function call: expr , arg1 ; arg2 bslash
        if (peek().type == TokenType::Comma) {
            eat(); // ,
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

        // indexing: expr [ index ]
        if (peek().type == TokenType::LBracket) {
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
