#include "parser.hpp"
#include"values.hpp"
#include <iostream>

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
    if (peek().type == TokenType::TypeIdent) {
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
    //add constant support later
    ValueType type;
    if(true){
        Token tem = eat();
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
        cout << "Expected identifier in variable declaration\n";
        return new VarDecl("INVALID");
    }

    Expr* initializer = nullptr;

    if (peek().type == TokenType::AssignmentOp) {
        eat(); // =
        initializer = ParseExpr();
    }

    if (peek().type == TokenType::Semicolon)
        eat();

    return new VarDecl(name.value, initializer);
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
        Expr* right = ParseBinaryExpr(nextPrec + 1);

        left = new BinaryExpr(left, right, op.value);
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
                cout << "Expected closing \\n";
            } else {
                eat();
            }
            break;

        default:
            cout << "UNEXPECTED TOKEN: " << tok.value << endl;
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
                cout << "Expected ]\n";
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
            return 3;

        default:
            return -1;
    }
}
