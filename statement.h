#include "expression.h"

struct ExpressionStmt;
struct PrintStmt;
struct VarStmt;
struct IfStmt;
struct WhileStmt;
struct BlockSmt;
using Stmt = std::variant<ExpressionStmt*, PrintStmt*, VarStmt*, IfStmt*, WhileStmt*, BlockSmt*, std::nullptr_t>;

struct ExpressionStmt
{
    ExpressionStmt ( Expr expression_ ) : expression ( expression_ ) {}
    Expr expression;
};

struct PrintStmt
{
    PrintStmt ( Expr expression_ ) : expression ( expression_ ) {}
    Expr expression;
};

struct VarStmt
{
    VarStmt(Token name_, Expr expression_) : name(name_), initializer(expression_) {}
    Token name;
    Expr initializer;
};

struct IfStmt
{
    IfStmt(Expr cond_, Stmt then_, Stmt else_) : condition(cond_), thenBranch(then_), elseBranch(else_) {}
    Expr condition;
    Stmt thenBranch;
    Stmt elseBranch;
};

struct WhileStmt
{
    WhileStmt(Expr cond_, Stmt body_) : condition(cond_), body(body_) {}
    Expr condition;
    Stmt body;
};

struct BlockSmt
{
    BlockSmt(std::vector<Stmt>& statements_) : statements(statements_)  {}
    std::vector<Stmt> statements;
};


