#include"values.hpp"
#include"ast.hpp"
#include<string>
#include<memory>
#pragma once

enum class LiteralType {
    Integer,
    Float,
    String,
    Invalid
};

std::unique_ptr<RuntimeVal>Eval(Stmt* astNode);
std::unique_ptr<RuntimeVal> EvalBinaryExpr(BinaryExpr binop);
FloatVal EvalFloatBExpr(FloatVal left, FloatVal right, std::string op);
IntVal EvalIntBExpr(IntVal left, IntVal right, std::string op);
std::unique_ptr<RuntimeVal> EvalProgram(Program* prog);

LiteralType cLiteral(std::string s);