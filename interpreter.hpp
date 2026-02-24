#include"values.hpp"
#include"ast.hpp"
#include<string>
#include<memory>
#include "environment.hpp"
#pragma once

enum class LiteralType {
    Integer,
    Float,
    String,
    Boolean,
    Invalid
};

std::unique_ptr<RuntimeVal> Eval(Stmt* astNode, Environment& env) ;
std::unique_ptr<RuntimeVal> EvalBinaryExpr(BinaryExpr binop, Environment& env);
FloatVal EvalFloatBExpr(FloatVal left, FloatVal right, std::string op);
IntVal EvalIntBExpr(IntVal left, IntVal right, std::string op);
std::unique_ptr<RuntimeVal> EvalIdentifier(Identifier identifier, Environment& env);
std::unique_ptr<RuntimeVal> EvalProgram(Program* prog, Environment& env);

LiteralType cLiteral(std::string s);