#ifndef PARSER_HPP
#define PARSER_HPP
#include <string>
#include <vector>
#include "lexer.hpp"
#include "ast.hpp"
#include <unordered_set>

class Parser {
public:
    Program produceAST(std::string sourcecode);

private:
    std::vector<Token> tokens;
    // keep track of struct/class names seen so far
    std::unordered_set<std::string> structNames;
    // flag to prevent backslash postfix operators during argument parsing
    bool inArgumentList = false;

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
    BlockStmt* parseBlock();
    Stmt* ParseClassDecl();
    void expect(TokenType type, const std::string& message);


    int getPrecedence(const Token& tok);
};

#endif