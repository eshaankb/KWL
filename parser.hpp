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
    size_t pos = 0;
    // keep track of struct/class names seen so far
    std::unordered_set<std::string> structNames;
    // flag to prevent backslash postfix operators during argument parsing
    bool inArgumentList = false;
    // flag for grouping expressions (e.g., return value \x\)
    bool inGrouping = false;

    bool notEOF();
    Token eat();
    Token& peek(int offset = 0);

    // parsing
    Stmt* ParseStmt();
    Expr* ParseExpr();
    Expr* ParseBinaryExpr(int precedence = 0);
    Expr* ParseUnaryExpr();
    Expr* ParsePrimExpr();
    Stmt* ParseVarDecl();
    Stmt* ParseTrueClassDecl();
    Stmt* parseIf();
    Stmt* ParseEl();
    BlockStmt* parseBlock();
    Stmt* ParseClassDecl();
    Stmt* ParseFunctionDecl();
    Expr* ParseFunctionCall(const string& fnName);
    Expr* ParseReturn();
    Expr* ParseArrayLiteral();
    void expect(TokenType type, const std::string& message);


    int getPrecedence(const Token& tok);
};

#endif