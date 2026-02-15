//NOT NEEDED

#include<fstream>
#include<string>
#include<vector>
#include<iostream>
#include<cctype>
#include<map>
#include<sstream>

using namespace std;

enum NodeType {
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
    WhileStatement,
    ReturnStatement,
    BuiltinCall,
};

class Stmt {
    public:
        NodeType kind;
        Stmt(NodeType k) : kind(k) {}
        virtual ~Stmt() = default;
        
};

class program : Stmt {
    public:
        program() : Stmt(Program) {}
        vector<Stmt> body();
};

class Expr : Stmt {
    public:
        Expr(NodeType k) : Stmt(k) {}
};

class binaryExpr : Expr {
    Expr* Left;
    Expr* Right;
    string Opertator;
    public:
    binaryExpr(Expr* lhs, Expr* rhs, string op) :
     Expr(BinaryExpression), Left(lhs), Right(rhs), Opertator(op) {}
};

class UnaryExpr : public Expr {
public:
    Expr* operand;
    string op;

    UnaryExpr(Expr* val, const string& oper)
        : Expr(UnaryExpression), operand(val), op(oper) {}
};

class Literal : public Expr {
public:
    string value;
    Literal(const string& val) : Expr(NodeType::Literal), value(val) {}
};

class Identifier : public Expr {
public:
    string name;
    Identifier(const string& n) : Expr(NodeType::Identifier), name(n) {}
};

class VarDecl : public Stmt {
public:
    string name;
    Expr* initializer; 
    bool immutable;

    VarDecl(const string& n, Expr* init = nullptr, bool immut = false)
        : Stmt(VariableDeclaration), name(n), initializer(init), immutable(immut) {}
};

class Assignment : public Stmt {
public:
    Expr* target;
    Expr* value;

    Assignment(Expr* t, Expr* val) : Stmt(NodeType::Assignment), target(t), value(val) {}
};

class BlockStmt : public Stmt {
public:
    vector<Stmt*> body;

    BlockStmt() : Stmt(BlockStatement) {}
};


class CallExpr : public Expr {
public:
    Expr* callee;
    vector<Expr*> arguments;

    CallExpr(Expr* c, const vector<Expr*>& args)
        : Expr(CallExpression), callee(c), arguments(args) {}
};


class IndexExpr : public Expr {
public:
    Expr* object;
    Expr* index;

    IndexExpr(Expr* obj, Expr* idx) : Expr(IndexExpression), object(obj), index(idx) {}
};


class IfStmt : public Stmt {
public:
    Expr* condition;
    Stmt* thenBranch;
    Stmt* elseBranch;

    IfStmt(Expr* cond, Stmt* thenB, Stmt* elseB = nullptr)
        : Stmt(IfStatement), condition(cond), thenBranch(thenB), elseBranch(elseB) {}
};


class WhileStmt : public Stmt {
public:
    Expr* condition;
    Stmt* body;

    WhileStmt(Expr* cond, Stmt* b) : Stmt(WhileStatement), condition(cond), body(b) {}
};


class ReturnStmt : public Stmt {
public:
    Expr* value;

    ReturnStmt(Expr* val = nullptr) : Stmt(ReturnStatement), value(val) {}
};


class BuiltinCall : public Expr {
public:
    string name;
    vector<Expr*> arguments;

    BuiltinCall(const string& n, const vector<Expr*>& args = {})
        : Expr(NodeType::BuiltinCall), name(n), arguments(args) {}
};

string nodeTypeName(NodeType t) {
    switch (t) {
        case NodeType::Program: return "Program";
        case NodeType::BlockStatement: return "BlockStatement";
        case NodeType::VariableDeclaration: return "VariableDeclaration";
        case NodeType::Assignment: return "Assignment";
        case NodeType::BinaryExpression: return "BinaryExpression";
        case NodeType::UnaryExpression: return "UnaryExpression";
        case NodeType::Literal: return "Literal";
        case NodeType::Identifier: return "Identifier";
        case NodeType::CallExpression: return "CallExpression";
        case NodeType::IndexExpression: return "IndexExpression";
        case NodeType::IfStatement: return "IfStatement";
        case NodeType::WhileStatement: return "WhileStatement";
        case NodeType::ReturnStatement: return "ReturnStatement";
        case NodeType::BuiltinCall: return "BuiltinCall";
        default: return "Unknown";
    }
}
