#pragma once
#include <vector>
#include <string>

enum class TokenType {
    Null,
    Identifier,
    Keyword,
    BuiltinIdentifier,
    TypeIdent,
    IntLiteral,
    FloatLiteral,
    StringLiteral,
    BoolLiteral,
    ArithmeticOp,   // + - * / mod
    LogicalOp,      // and or nt
    ComparisonOp,   // == n= < > =< >=
    AssignmentOp,   // = += -= etc
    RangeOp,        // ~
    LBracket,       // [
    RBracket,       // ]
    Backslash,       // \\
    
    Pipe,           // |
    Comma,          // ,
    Semicolon,      // ;

    // Special
    Backtick,       // `
    CommentLine,    // #
    CommentBlock,   // ### ###
    EndOfFile,
    Invalid
};

struct Token {
    TokenType type;
    std::string value;
    Token(std::string in_val, TokenType in_type) : value(in_val), type(in_type) {};
    Token() : type(TokenType::Invalid), value("") {}
};
std::vector<Token> tokenize(std::string sourcecode);
// std::string tokenTypeName(TokenType t){};
