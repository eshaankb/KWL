/**
 * @file parser.hpp
 * @brief Parser header for the KWL interpreter.
 *
 * Defines the Parser class that performs syntactic analysis, converting
 * a token stream from the lexer into an Abstract Syntax Tree (AST).
 *
 * @author KWL Interpreter
 * @date 2026
 */

#ifndef PARSER_HPP
#define PARSER_HPP
#include <string>
#include <vector>
#include "lexer.hpp"
#include "ast.hpp"
#include <unordered_set>

/**
 * @class Parser
 * @brief Transforms a token stream into an Abstract Syntax Tree.
 *
 * The parser uses recursive descent parsing to convert tokens into
 * AST nodes. It handles operator precedence, grouping, and all
 * KWL language constructs including functions, classes, and control flow.
 */
class Parser {
public:
    /**
     * @brief Produces an AST from KWL source code.
     * @param sourcecode The KWL source code to parse
     * @return A Program node representing the entire source file
     */
    Program produceAST(std::string sourcecode);

private:
    std::vector<Token> tokens;      ///< The token stream being parsed
    size_t pos = 0;                ///< Current position in the token stream
    std::unordered_set<std::string> structNames; ///< Known struct/class names
    bool inArgumentList = false;   ///< Whether currently parsing arguments
    int groupingDepth = 0;         ///< Nesting depth for grouped expressions

    /**
     * @brief Checks if there are more tokens to process.
     * @return true if not at end of token stream
     */
    bool notEOF();

    /**
     * @brief Consumes and returns the current token.
     * @return The consumed Token
     */
    Token eat();

    /**
     * @brief Peeks at a token without consuming it.
     * @param offset Offset from current position (default: 0)
     * @return Reference to the peeked Token
     */
    Token& peek(int offset = 0);

    // ===== Parsing Methods =====

    /**
     * @brief Parses any statement (variable declaration, control flow, etc.)
     * @return Pointer to the parsed Stmt node
     */
    Stmt* ParseStmt();

    /**
     * @brief Parses any expression
     * @return Pointer to the parsed Expr node
     */
    Expr* ParseExpr();

    /**
     * @brief Parses binary expressions with precedence handling.
     * @param precedence Current operator precedence level
     * @return Pointer to the parsed binary expression
     */
    Expr* ParseBinaryExpr(int precedence = 0);

    /**
     * @brief Parses unary expressions (e.g., -5, not true)
     * @return Pointer to the parsed unary expression
     */
    Expr* ParseUnaryExpr();

    /**
     * @brief Parses primary expressions (literals, identifiers, grouped)
     * @return Pointer to the parsed primary expression
     */
    Expr* ParsePrimExpr();

    /**
     * @brief Parses variable/constant declarations
     * @return Pointer to the parsed variable declaration
     */
    Stmt* ParseVarDecl();

    /**
     * @brief Parses a class declaration with methods (TrueClass)
     * @return Pointer to the parsed class declaration
     */
    Stmt* ParseTrueClassDecl();

    /**
     * @brief Parses if/else conditional statements
     * @return Pointer to the parsed if statement
     */
    Stmt* parseIf();

    /**
     * @brief Parses else branches
     * @return Pointer to the parsed else statement
     */
    Stmt* ParseEl();

    /**
     * @brief Parses a block of statements in braces
     * @return Pointer to the parsed BlockStmt
     */
    BlockStmt* parseBlock();

    /**
     * @brief Parses a structure/class declaration
     * @return Pointer to the parsed struct declaration
     */
    Stmt* ParseClassDecl();

    /**
     * @brief Parses a function declaration
     * @return Pointer to the parsed function declaration
     */
    Stmt* ParseFunctionDecl();

    /**
     * @brief Parses a function call
     * @param fnName The name of the function being called
     * @return Pointer to the parsed call expression
     */
    Expr* ParseFunctionCall(const string& fnName);

    /**
     * @brief Parses a return statement
     * @return Pointer to the parsed return statement
     */
    Expr* ParseReturn();

    /**
     * @brief Parses an array literal
     * @return Pointer to the parsed array literal
     */
    Expr* ParseArrayLiteral();

    /**
     * @brief Parses a while loop statement
     * @return Pointer to the parsed while statement
     */
    Stmt* ParseLp();

    /**
     * @brief Expects a specific token type or reports an error.
     * @param type Expected token type
     * @param message Error message if token doesn't match
     */
    void expect(TokenType type, const std::string& message);

    /**
     * @brief Gets the precedence of an operator token.
     * @param tok The token to get precedence for
     * @return Integer precedence value (higher = binds tighter)
     */
    int getPrecedence(const Token& tok);
};

#endif
