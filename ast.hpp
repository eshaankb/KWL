#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include "types.hpp"
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
    ConstructorDeclaration,
    ConstructorCall,
    IndexExpression,
    IfStatement,
    ElseStatement,
    WhileStatement,
    ReturnStatement,
    BuiltinCall,
    NullLiteral,
    StructureDeclaration,
    FunctionDeclaration,
    FunctionCall,
    ImportStatement,
    ExportDeclaration,
    ArrayLiteral,
    RangeExpression
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
    Stmt* body;

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

class ConstructorCallExpr : public Expr {
public:
    string className;
    vector<Expr*> arguments;

    ConstructorCallExpr(const string& cn, const vector<Expr*>& args)
        : Expr(NodeType::ConstructorCall), className(cn), arguments(args) {}
};

class IndexExpr : public Expr {
public:
    Expr* object;
    Expr* index;

    IndexExpr(Expr* obj, Expr* idx)
        : Expr(NodeType::IndexExpression), object(obj), index(idx) {}
};

class CallStructExpr : public Expr {
public:
    Expr* object;
    string field;

    CallStructExpr(Expr* obj, const string& f)
        : Expr(NodeType::CallExpression), object(obj), field(f) {}
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
    // for structures we remember the class name
    std::string structTypeName;

   VarDecl(ValueType tp, const string& n, Expr* init = nullptr, bool immut = false, const std::string& structName = "")
        : type(tp), Stmt(NodeType::VariableDeclaration), name(n), value(init), immutable(immut), structTypeName(structName) {}
};

struct ConstructorDecl : public Stmt {
    vector<pair<string, ValueType>> params; // parameter name and type

    ConstructorDecl(const vector<pair<string, ValueType>>& p) : Stmt(NodeType::ConstructorDeclaration), params(p)  {}
};

class StructDecl : public Stmt {
public:
    string name;
    vector<pair<string, ValueType>> vars;
    ConstructorDecl* constructor;
    ValueType type = ValueType::Structure;

   StructDecl(const string& n, const vector<pair<string, ValueType>>& v, ConstructorDecl* constr = nullptr, bool immut = false)
        : Stmt(NodeType::StructureDeclaration), name(n), vars(v), constructor(constr) {}
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

class ReturnStmt : public Expr {
public:
    Expr* value;

    explicit ReturnStmt(Expr* val = nullptr)
        : Expr(NodeType::ReturnStatement), value(val) {}

};
struct NullLiteral : public Expr {
    NullLiteral() : Expr(NodeType::NullLiteral) {}
};

struct FunctionDecl : public Stmt {
    string name;
    vector<VarDecl*> parameters;
    ValueType* returnType;
    BlockStmt* body;
    FunctionDecl(const string& n, const vector<VarDecl*>& params, BlockStmt* b, ValueType* rt = nullptr)
        : Stmt(NodeType::FunctionDeclaration), name(n), parameters(params), body(b), returnType(rt) {}
};

struct FunctionCall : public Expr {
    string functionName;
    vector<Expr*> arguments;
    FunctionCall(const string& fn, const vector<Expr*>& args)
        : Expr(NodeType::FunctionCall), functionName(fn), arguments(args) {}
};
struct ImportStmt : public Stmt {
    std::string modulePath;
    std::string alias;
    bool importAll = false;
    ImportStmt(const std::string& path, const std::string& as = "", bool all = false)
        : Stmt(NodeType::ImportStatement), modulePath(path), alias(as), importAll(all) {}
};
struct ExportDecl : public Stmt {
    Stmt* decl;
    ExportDecl(Stmt* d) : Stmt(NodeType::ExportDeclaration), decl(d) {}
};

class ArrayLiteral : public Expr {
public:
    vector<Expr*> elements;
    explicit ArrayLiteral(const vector<Expr*>& elems)
        : Expr(NodeType::ArrayLiteral), elements(elems) {}
};

class RangeExpr : public Expr {
public:
    Expr* start;
    Expr* end;
    RangeExpr(Expr* s, Expr* e)
        : Expr(NodeType::RangeExpression), start(s), end(e) {}
};


#endif
