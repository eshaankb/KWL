#include"values.hpp"
#include"ast.hpp"
#include<string>

enum class LiteralType {
    Integer,
    Float,
    String,
    Invalid
};

RuntimeVal Interpret(Stmt* astNode);

LiteralType cLiteral(std::string s);