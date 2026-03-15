/**
 * @file lexer.hpp
 * @brief Lexical analyzer header for the KWL interpreter.
 *
 * Defines the Token structure and TokenType enumeration for the
 * lexical analysis phase. The lexer converts source code text
 * into a stream of tokens.
 *
 * @author KWL Interpreter
 * @date 2026
 */

#pragma once
#include <vector>
#include <string>

/**
 * @enum TokenType
 * @brief Enumeration of all token types in KWL.
 *
 * Categorizes the different kinds of tokens that can appear in KWL source code.
 */
enum class TokenType {
    Null,               ///< No token (internal use)
    Identifier,         ///< Variable/function names
    Keyword,            ///< Language keywords (let, const, if, etc.)
    BlockKeyword,       ///< Block-scoped keywords (if, lp, etc.)
    BuiltinIdentifier,  ///< Built-in function names
    TypeIdent,          ///< Type identifiers
    IntLiteral,         ///< Integer literal (e.g., 42)
    FloatLiteral,       ///< Float literal (e.g., 3.14)
    StringLiteral,      ///< String literal (e.g., "hello")
    BoolLiteral,        ///< Boolean literal (true/false)
    ArithmeticOp,       ///< Arithmetic operators (+, -, *, /, mod, **)
    LogicalOp,          ///< Logical operators (and, or, nt)
    ComparisonOp,       ///< Comparison operators (==, n=, <, >, =<, >=)
    AssignmentOp,       ///< Assignment operators (=, +=, -=, etc.)
    RangeOp,            ///< Range operator (:)
    LBracket,           ///< Left bracket ([)
    RBracket,           ///< Right bracket (])
    Backslash,          ///< Backslash (\)
    Pipe,               ///< Pipe operator (|)
    Comma,              ///< Comma (,)
    Semicolon,          ///< Semicolon (;)
    Backtick,           ///< Backtick (`)
    CommentLine,        ///< Line comment (#)
    CommentBlock,       ///< Block comment (### ... ###)
    EndOfFile,          ///< End of file marker
    Invalid             ///< Invalid token
};

/**
 * @struct Token
 * @brief Represents a single token in the KWL language.
 *
 * Contains the token type, its string value, and source location information.
 */
struct Token {
    TokenType type;             ///< The type of this token
    std::string value;         ///< The token's text value
    int line = 1;              ///< Source line number (1-based)

    /**
     * @brief Constructs a Token with the given value and type.
     * @param in_val The token's text value
     * @param in_type The token type
     * @param ln Source line number (default: 1)
     */
    Token(std::string in_val, TokenType in_type, int ln = 1)
        : value(in_val), type(in_type), line(ln) {};

    /**
     * @brief Default constructor creating an invalid token.
     */
    Token() : type(TokenType::Invalid), value(""), line(0) {}
};

/**
 * @brief Tokenizes KWL source code into a vector of Tokens.
 * @param sourcecode The KWL source code as a string
 * @return A vector of Tokens representing the tokenized input
 */
std::vector<Token> tokenize(std::string sourcecode);
