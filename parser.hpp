#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>
#include "lexer.hpp"
#include "ast.hpp"

class Parser {
public:
    Program produceAST(std::string sourcecode);

private:
    std::vector<Token> tokens;
    bool notEOF();
    Token eat();
    Token& peek(int offset = 0);

    // parsing
    Stmt* ParseStmt();
    Expr* ParseExpr();
    Expr* ParseBinaryExpr(int precedence = 0);
    Expr* ParsePrimExpr();
    Stmt* ParseVarDecl();
    Stmt* parseIf();
    Stmt* parseBlock();


    int getPrecedence(const Token& tok);
};

#endif