/**
 * @file interpreter.hpp
 * @brief Interpreter header for the KWL language.
 *
 * Declares evaluation functions that traverse the AST and execute
 * KWL code by evaluating each node in the appropriate environment context.
 *
 * @author KWL Interpreter
 * @date 2026
 */

#include"values.hpp"
#include"ast.hpp"
#include<string>
#include "environment.hpp"
#pragma once

/**
 * @enum LiteralType
 * @brief Classification of literal value types.
 *
 * Used during parsing/evaluation to determine the type of a literal value.
 */
enum class LiteralType {
    Integer,   ///< Integer literal (e.g., 42)
    Float,     ///< Float literal (e.g., 3.14)
    String,    ///< String literal (e.g., "hello")
    Boolean,   ///< Boolean literal (true/false)
    Invalid    ///< Not a valid literal
};

/**
 * @brief Evaluates any AST node and returns its runtime value.
 * @param astNode The AST node to evaluate
 * @param env The current environment (scope)
 * @return RuntimeVal* representing the result of evaluation
 */
RuntimeVal* Eval(Stmt* astNode, Environment& env);

/**
 * @brief Evaluates a binary expression (two operands with operator).
 * @param binop The BinaryExpr AST node
 * @param env The current environment
 * @return RuntimeVal* with the result
 */
RuntimeVal* EvalBinaryExpr(BinaryExpr binop, Environment& env);

/**
 * @brief Evaluates a binary expression on FloatVal operands.
 * @param left Left operand
 * @param right Right operand
 * @param op Operator string
 * @return FloatVal result
 */
FloatVal EvalFloatBExpr(FloatVal left, FloatVal right, std::string op);

/**
 * @brief Evaluates a binary expression on IntVal operands.
 * @param left Left operand
 * @param right Right operand
 * @param op Operator string
 * @return IntVal result
 */
IntVal EvalIntBExpr(IntVal left, IntVal right, std::string op);

/**
 * @brief Evaluates an identifier (variable lookup).
 * @param identifier The Identifier AST node
 * @param env The current environment
 * @return RuntimeVal* with the variable's value
 */
RuntimeVal* EvalIdentifier(Identifier identifier, Environment& env);

/**
 * @brief Evaluates the entire program (list of statements).
 * @param prog The Program AST node
 * @param env The current environment
 * @return RuntimeVal* with the last statement's value
 */
RuntimeVal* EvalProgram(Program* prog, Environment& env);

/**
 * @brief Evaluates a constructor call (object instantiation).
 * @param call The ConstructorCallExpr AST node
 * @param env The current environment
 * @return RuntimeVal* with the new object instance
 */
RuntimeVal* EvalConstructorCall(ConstructorCallExpr call, Environment& env);

/**
 * @brief Classifies a string as a literal type.
 * @param s The string to classify
 * @return LiteralType classification
 */
LiteralType classifyLiteral(const std::string& s);
