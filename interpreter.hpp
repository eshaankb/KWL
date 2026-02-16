#include"values.hpp"
#include"ast.hpp"
#include<string>

enum class LiteralType {
    Integer,
    Float,
    String,
    Invalid
};

RuntimeVal Eval(Stmt* astNode);

LiteralType cLiteral(std::string s);