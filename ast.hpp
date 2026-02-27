#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include "values.hpp"
using namespace std;

enum class NodeType {
    Program,
    BlockStatement,
    VariableDeclaration,
    Assignment,
    BinaryExpression,
    UnaryExpression,
    Literal,
    Identifier,
    CallExpression,
    IndexExpression,
    IfStatement,
    ElseStatement,
    WhileStatement,
    ReturnStatement,
    BuiltinCall,
    NullLiteral,
};



class Stmt {
public:
    NodeType kind;
    explicit Stmt(NodeType k) : kind(k) {}
    virtual ~Stmt() = default;
};

class Expr : public Stmt {
public:
    explicit Expr(NodeType k) : Stmt(k) {}
};


class Program : public Stmt {
public:
    vector<Stmt*> body;
    Program() : Stmt(NodeType::Program) {}
};
class BlockStmt : public Stmt {
public:
    vector<Stmt*> body;
    BlockStmt() : Stmt(NodeType::BlockStatement) {}
};
class ElseStmt : public Stmt {
public:
    BlockStmt* body;

    ElseStmt(BlockStmt* b) : Stmt(NodeType::ElseStatement), body(b) {}
};
class IfStmt : public Stmt {
public:
    Expr* condition;
    BlockStmt* body;
    ElseStmt* elseBranch;

    IfStmt(Expr* cond, BlockStmt* b, ElseStmt* e = nullptr) : Stmt(NodeType::IfStatement), condition(cond), body(b), elseBranch(e) {}
};




class BinaryExpr : public Expr {
public:
    Expr* left;
    Expr* right;
    string op;

    BinaryExpr(Expr* lhs, Expr* rhs, const string& oper)
        : Expr(NodeType::BinaryExpression), left(lhs), right(rhs), op(oper) {}
};

class UnaryExpr : public Expr {
public:
    Expr* operand;
    string op;

    UnaryExpr(Expr* val, const string& oper)
        : Expr(NodeType::UnaryExpression), operand(val), op(oper) {}
};

class Literal : public Expr {
public:
    string value;
    explicit Literal(const string& val)
        : Expr(NodeType::Literal), value(val) {}
};

class Identifier : public Expr {
public:
    string name;
    explicit Identifier(const string& n)
        : Expr(NodeType::Identifier), name(n) {}
};

class CallExpr : public Expr {
public:
    Expr* callee;
    vector<Expr*> arguments;

    CallExpr(Expr* c, const vector<Expr*>& args)
        : Expr(NodeType::CallExpression), callee(c), arguments(args) {}
};

class IndexExpr : public Expr {
public:
    Expr* object;
    Expr* index;

    IndexExpr(Expr* obj, Expr* idx)
        : Expr(NodeType::IndexExpression), object(obj), index(idx) {}
};

class BuiltinCall : public Expr {
public:
    string name;
    vector<Expr*> arguments;

    BuiltinCall(const string& n, const vector<Expr*>& args = {})
        : Expr(NodeType::BuiltinCall), name(n), arguments(args) {}
};


class VarDecl : public Stmt {
public:
    string name;
    Expr* value; 
    bool immutable;
    ValueType type;

   VarDecl(ValueType tp, const string& n, Expr* init = nullptr, bool immut = false)
        : type(tp), Stmt(NodeType::VariableDeclaration), name(n), value(init), immutable(immut) {}
};

class Assignment : public Expr {
public:
    Expr* target;
    Expr* value;

    Assignment(Expr* t, Expr* val)
        : Expr(NodeType::Assignment), target(t), value(val) {}
};

class WhileStmt : public Stmt {
public:
    Expr* condition;
    Stmt* body;

    WhileStmt(Expr* cond, Stmt* b)
        : Stmt(NodeType::WhileStatement), condition(cond), body(b) {}
};

class ReturnStmt : public Stmt {
public:
    Expr* value;

    explicit ReturnStmt(Expr* val = nullptr)
        : Stmt(NodeType::ReturnStatement), value(val) {}

};
struct NullLiteral : public Expr {
    NullLiteral() : Expr(NodeType::NullLiteral) {}
};
#endif
