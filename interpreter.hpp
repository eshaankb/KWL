#include"values.hpp"
#include"ast.hpp"
#include<string>
#include "environment.hpp"
#pragma once

enum class LiteralType {
    Integer,
    Float,
    String,
    Boolean,
    Invalid
};

RuntimeVal* Eval(Stmt* astNode, Environment& env) ;
RuntimeVal* EvalBinaryExpr(BinaryExpr binop, Environment& env);
FloatVal EvalFloatBExpr(FloatVal left, FloatVal right, std::string op);
IntVal EvalIntBExpr(IntVal left, IntVal right, std::string op);
RuntimeVal* EvalIdentifier(Identifier identifier, Environment& env);
RuntimeVal* EvalProgram(Program* prog, Environment& env);
RuntimeVal* EvalConstructorCall(ConstructorCallExpr call, Environment& env);

LiteralType cLiteral(std::string s);