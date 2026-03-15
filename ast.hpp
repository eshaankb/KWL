/**
 * @file ast.hpp
 * @brief Abstract Syntax Tree (AST) node definitions for the KWL interpreter.
 *
 * This file defines all AST node types used to represent KWL source code
 * in an object-oriented tree structure. Nodes are divided into statements
 * (Stmt) and expressions (Expr), both deriving from a common base.
 *
 * @author KWL Interpreter
 * @date 2026
 */

#include <string>
#include <vector>
#include <variant>
#include "types.hpp"
#pragma once
using namespace std;

/**
 * @enum NodeType
 * @brief Enumeration of all possible AST node types in KWL.
 *
 * Each variant represents a different syntactic construct in the KWL language,
 * from simple literals to complex statements like functions and structures.
 */
enum class NodeType {
    Program,                ///< Root node containing the entire program
    BlockStatement,         ///< A sequence of statements in braces
    VariableDeclaration,    ///< Variable/constant declaration (let/const)
    Assignment,             ///< Variable assignment expression
    BinaryExpression,       ///< Binary operation (e.g., a + b)
    UnaryExpression,        ///< Unary operation (e.g., -x, not x)
    Literal,                ///< String, numeric, or boolean literal
    Identifier,             ///< Variable or function name reference
    CallExpression,         ///< Function or method call
    ConstructorDeclaration, ///< Constructor definition within a structure
    ConstructorCall,         ///< Object instantiation expression
    IndexExpression,        ///< Array or string indexing (e.g., arr[0])
    IfStatement,            ///< Conditional statement (if/else)
    ElseStatement,          ///< Else branch of conditional
    WhileStatement,         ///< While loop statement
    ReturnStatement,        ///< Return statement in function
    BuiltinCall,            ///< Built-in function call (wt, rd, etc.)
    NullLiteral,            ///< Null value literal
    StructureDeclaration,   ///< Class/structure type definition
    FunctionDeclaration,    ///< Function definition
    FunctionCall,           ///< Function invocation expression
    ImportStatement,        ///< Module import statement
    ExportDeclaration,      ///< Module export statement
    ArrayLiteral,           ///< Array literal (e.g., [1, 2, 3])
    RangeExpression         ///< Range literal (e.g., 1:10)
};



/**
 * @class Stmt
 * @brief Base class for all statement nodes in the AST.
 *
 * Statements are executable program elements that produce no value.
 * Examples include variable declarations, loops, and control flow.
 */
class Stmt {
public:
    NodeType kind;           ///< The type of AST node this statement represents
    explicit Stmt(NodeType k) : kind(k) {}
    virtual ~Stmt() = default;
};

/**
 * @class Expr
 * @brief Base class for all expression nodes in the AST.
 *
 * Expressions are program elements that evaluate to a value.
 * This class inherits from Stmt since expressions can appear in statement positions.
 */
class Expr : public Stmt {
public:
    explicit Expr(NodeType k) : Stmt(k) {}
};


/**
 * @class Program
 * @brief Root node of the AST representing a complete KWL source file.
 *
 * The Program node contains a body vector of all top-level statements
 * in the source file, representing the entry point for interpretation.
 */
class Program : public Stmt {
public:
    vector<Stmt*> body;     ///< List of top-level statements in the program
    Program() : Stmt(NodeType::Program) {}
};

/**
 * @class BlockStmt
 * @brief Represents a sequence of statements grouped together.
 *
 * Block statements are used in function bodies, if/else branches,
 * and anywhere multiple statements need to be grouped.
 */
class BlockStmt : public Stmt {
public:
    vector<Stmt*> body;     ///< Statements contained within the block
    BlockStmt() : Stmt(NodeType::BlockStatement) {}
};

class ElseStmt;

/**
 * @class IfStmt
 * @brief Represents a conditional (if/else) statement.
 *
 * Contains a condition expression, a main body (executed if condition is true),
 * and an optional else branch.
 */
class IfStmt : public Stmt {
public:
    Expr* condition;        ///< Expression evaluated to determine which branch to take
    BlockStmt* body;        ///< Statements executed when condition is true
    Stmt* elseBranch;       ///< Optional else branch (ElseStmt or another IfStmt)

    IfStmt(Expr* cond, BlockStmt* b, Stmt* e = nullptr) : Stmt(NodeType::IfStatement), condition(cond), body(b), elseBranch(e) {}
};

/**
 * @class ElseStmt
 * @brief Represents an else branch in a conditional statement.
 */
class ElseStmt : public Stmt {
public:
    Stmt* body;             ///< Statements executed when preceding if condition was false

    ElseStmt(Stmt* b) : Stmt(NodeType::ElseStatement), body(b) {}
};



/**
 * @class BinaryExpr
 * @brief Represents a binary operation (operator with two operands).
 *
 * Examples: a + b, x * y, str == "hello"
 */
class BinaryExpr : public Expr {
public:
    Expr* left;             ///< Left-hand side operand
    Expr* right;           ///< Right-hand side operand
    string op;             ///< Operator symbol (e.g., "+", "==", "and")

    BinaryExpr(Expr* lhs, Expr* rhs, const string& oper)
        : Expr(NodeType::BinaryExpression), left(lhs), right(rhs), op(oper) {}
};

/**
 * @class UnaryExpr
 * @brief Represents a unary operation (operator with one operand).
 *
 * Examples: -5, not true, !x
 */
class UnaryExpr : public Expr {
public:
    Expr* operand;          ///< Operand to which the operator is applied
    string op;             ///< Operator symbol (e.g., "-", "not", "!")

    UnaryExpr(Expr* val, const string& oper)
        : Expr(NodeType::UnaryExpression), operand(val), op(oper) {}
};

/**
 * @class Literal
 * @brief Represents a literal value in the source code.
 *
 * Contains string representations of numeric, string, or boolean literals.
 */
class Literal : public Expr {
public:
    string value;          ///< The literal value as a string
    explicit Literal(const string& val)
        : Expr(NodeType::Literal), value(val) {}
};

/**
 * @class Identifier
 * @brief Represents a reference to a variable or function by name.
 */
class Identifier : public Expr {
public:
    string name;           ///< The identifier name
    explicit Identifier(const string& n)
        : Expr(NodeType::Identifier), name(n) {}
};

/**
 * @class CallExpr
 * @brief Represents a function or method call expression.
 *
 * Supports both regular function calls and method calls on objects.
 */
class CallExpr : public Expr {
public:
    Expr* callee;          ///< The function or object being called
    string method;         ///< Method name for method calls (e.g., "module.method")
    vector<Expr*> arguments;///< Arguments passed to the function

    CallExpr(Expr* c, const vector<Expr*>& args)
        : Expr(NodeType::CallExpression), callee(c), arguments(args) {}
    CallExpr(Expr* c, const string& m, const vector<Expr*>& args)
        : Expr(NodeType::CallExpression), callee(c), method(m), arguments(args) {}
};

/**
 * @class ConstructorCallExpr
 * @brief Represents instantiation of a new object using a constructor.
 *
 * Example: MyClass(arg1, arg2)
 */
class ConstructorCallExpr : public Expr {
public:
    string className;      ///< Name of the class being instantiated
    vector<Expr*> arguments;///< Arguments passed to the constructor

    ConstructorCallExpr(const string& cn, const vector<Expr*>& args)
        : Expr(NodeType::ConstructorCall), className(cn), arguments(args) {}
};

/**
 * @class IndexExpr
 * @brief Represents indexing into an array or string.
 *
 * Example: arr[0], str[5]
 */
class IndexExpr : public Expr {
public:
    Expr* object;          ///< The array or string being indexed
    Expr* index;           ///< The index expression

    IndexExpr(Expr* obj, Expr* idx)
        : Expr(NodeType::IndexExpression), object(obj), index(idx) {}
};

/**
 * @class CallStructExpr
 * @brief Represents accessing a field or method on an object.
 *
 * Example: obj.field, obj.method()
 */
class CallStructExpr : public Expr {
public:
    Expr* object;          ///< The object whose field is being accessed
    string field;          ///< The field or method name

    CallStructExpr(Expr* obj, const string& f)
        : Expr(NodeType::CallExpression), object(obj), field(f) {}
};

/**
 * @class BuiltinCall
 * @brief Represents a call to a built-in function.
 *
 * Built-in functions include print, len, type, etc.
 */
class BuiltinCall : public Expr {
public:
    string name;           ///< Name of the built-in function
    vector<Expr*> arguments;///< Arguments passed to the function

    BuiltinCall(const string& n, const vector<Expr*>& args = {})
        : Expr(NodeType::BuiltinCall), name(n), arguments(args) {}
};


/**
 * @class VarDecl
 * @brief Represents a variable or constant declaration.
 *
 * Handles both mutable (let) and immutable (const) declarations.
 */
class VarDecl : public Stmt {
public:
    string name;           ///< Variable name
    Expr* value;           ///< Initial value expression
    bool immutable;        ///< Whether the variable is immutable (const)
    ValueType type;        ///< Declared type of the variable
    std::string structTypeName;///< Class name for structure types

   VarDecl(ValueType tp, const string& n, Expr* init = nullptr, bool immut = false, const std::string& structName = "")
        : type(tp), Stmt(NodeType::VariableDeclaration), name(n), value(init), immutable(immut), structTypeName(structName) {}
};

/**
 * @struct ConstructorDecl
 * @brief Represents a constructor definition within a structure/class.
 *
 * Constructors define how objects of a class are initialized.
 */
struct ConstructorDecl : public Stmt {
    vector<pair<string, ValueType>> params; ///< Constructor parameters (name, type)

    ConstructorDecl(const vector<pair<string, ValueType>>& p) : Stmt(NodeType::ConstructorDeclaration), params(p)  {}
};

/**
 * @class StructDecl
 * @brief Represents a structure/class type definition.
 *
 * Defines a new user-defined type with fields and a constructor.
 */
class StructDecl : public Stmt {
public:
    string name;                   ///< Name of the structure/class
    vector<pair<string, ValueType>> vars;///< Field definitions (name, type)
    ConstructorDecl* constructor; ///< Optional constructor definition
    ValueType type = ValueType::Structure;

   StructDecl(const string& n, const vector<pair<string, ValueType>>& v, ConstructorDecl* constr = nullptr, bool immut = false)
        : Stmt(NodeType::StructureDeclaration), name(n), vars(v), constructor(constr) {}
};


/**
 * @class Assignment
 * @brief Represents an assignment to a variable.
 *
 * Note: This is an expression that can appear anywhere a value is expected.
 */
class Assignment : public Expr {
public:
    Expr* target;          ///< Variable being assigned to
    Expr* value;           ///< Value being assigned

    Assignment(Expr* t, Expr* val)
        : Expr(NodeType::Assignment), target(t), value(val) {}
};

/**
 * @class WhileStmt
 * @brief Represents a while loop statement.
 *
 * Executes the body repeatedly while the condition is true.
 */
class WhileStmt : public Stmt {
public:
    Expr* condition;       ///< Loop continuation condition
    Stmt* body;            ///< Statements executed each iteration

    WhileStmt(Expr* cond, Stmt* b)
        : Stmt(NodeType::WhileStatement), condition(cond), body(b) {}
};

/**
 * @class ReturnStmt
 * @brief Represents a return statement in a function.
 *
 * Optionally returns a value to the caller.
 */
class ReturnStmt : public Expr {
public:
    Expr* value;          ///< Value to return (can be nullptr for void return)

    explicit ReturnStmt(Expr* val = nullptr)
        : Expr(NodeType::ReturnStatement), value(val) {}

};

/**
 * @struct NullLiteral
 * @brief Represents the null literal value.
 */
struct NullLiteral : public Expr {
    NullLiteral() : Expr(NodeType::NullLiteral) {}
};

/**
 * @struct FunctionDecl
 * @brief Represents a function definition.
 *
 * Contains the function name, parameters, return type, and body.
 */
struct FunctionDecl : public Stmt {
    string name;                   ///< Function name
    vector<VarDecl*> parameters;   ///< Function parameters
    ValueType* returnType;         ///< Return type (can be nullptr)
    BlockStmt* body;               ///< Function body statements

    FunctionDecl(const string& n, const vector<VarDecl*>& params, BlockStmt* b, ValueType* rt = nullptr)
        : Stmt(NodeType::FunctionDeclaration), name(n), parameters(params), body(b), returnType(rt) {}
};

/**
 * @class TrueObj
 * @brief Represents a full class definition with methods.
 *
 * A class-like structure that can have both fields and methods.
 */
class TrueObj : public Stmt {
public:
    string name;                                   ///< Class name
    vector<pair<string, ValueType>> vars;          ///< Field definitions
    vector<pair<string, FunctionDecl> > methods;   ///< Method definitions
    ConstructorDecl* constructor;                  ///< Constructor definition
    ValueType type = ValueType::Structure;

   TrueObj(const string& n, const vector<pair<string, ValueType>>& v, const vector<pair<string, FunctionDecl> >& m, ConstructorDecl* constr = nullptr, bool immut = false)
        : Stmt(NodeType::StructureDeclaration), name(n), vars(v), methods(m), constructor(constr) {}
};

/**
 * @struct FunctionCall
 * @brief Represents a function call expression (alternative form).
 */
struct FunctionCall : public Expr {
    string functionName;    ///< Name of the function being called
    vector<Expr*> arguments;///< Arguments passed to the function

    FunctionCall(const string& fn, const vector<Expr*>& args)
        : Expr(NodeType::FunctionCall), functionName(fn), arguments(args) {}
};

/**
 * @struct ImportStmt
 * @brief Represents an import statement for loading modules.
 *
 * Example: import "path/to/module" as alias
 */
struct ImportStmt : public Stmt {
    std::string modulePath;  ///< Path to the module file
    std::string alias;      ///< Optional alias for the imported module
    bool importAll = false; ///< Whether to import all symbols (import *)

    ImportStmt(const std::string& path, const std::string& as = "", bool all = false)
        : Stmt(NodeType::ImportStatement), modulePath(path), alias(as), importAll(all) {}
};

/**
 * @struct ExportDecl
 * @brief Represents an export declaration for modules.
 *
 * Marks a declaration as available for export to other modules.
 */
struct ExportDecl : public Stmt {
    Stmt* decl;             ///< The declaration being exported

    ExportDecl(Stmt* d) : Stmt(NodeType::ExportDeclaration), decl(d) {}
};

/**
 * @class ArrayLiteral
 * @brief Represents an array literal expression.
 *
 * Example: [1, 2, 3, "four", true]
 */
class ArrayLiteral : public Expr {
public:
    vector<Expr*> elements; ///< Array element expressions

    explicit ArrayLiteral(const vector<Expr*>& elems)
        : Expr(NodeType::ArrayLiteral), elements(elems) {}
};

/**
 * @class RangeExpr
 * @brief Represents a range expression.
 *
 * Used for iteration or slicing. Example: 1:10
 */
class RangeExpr : public Expr {
public:
    Expr* start;            ///< Starting value (inclusive)
    Expr* end;              ///< Ending value (exclusive)

    RangeExpr(Expr* s, Expr* e)
        : Expr(NodeType::RangeExpression), start(s), end(e) {}
};
