#pragma once
#include "expression.h"
#include <stdexcept>

struct ExpressionStmt;
struct PrintStmt;
struct VarStmt;
struct IfStmt;
struct WhileStmt;
struct BlockStmt;
struct FunctionStmt;
struct ClassStmt;
struct ReturnStmt;

using Stmt = std::variant
<
    ExpressionStmt*,
    PrintStmt*,
    VarStmt*,
    IfStmt*,
    WhileStmt*,
    BlockStmt*,
    FunctionStmt*,
    ClassStmt*,
    ReturnStmt*,
    std::nullptr_t
>;

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

struct BlockStmt
{
    BlockStmt(std::vector<Stmt>&& statements_) : statements(statements_)  {}
    BlockStmt(std::vector<Stmt>& statements_) : statements(std::move(statements_))  {}
    
    std::vector<Stmt> statements;
};

struct FunctionStmt
{
    FunctionStmt(Token name_, std::vector<Token>& params_, std::vector<Stmt> body_) :
        name(name_), params(std::move(params_)), body(std::move(body_))
    {}
    
    Token name;
    std::vector<Token> params;
    std::vector<Stmt> body;
};

struct ClassStmt
{
    ClassStmt(Token name_, std::vector<Stmt>& methods_) :
        name(name_), methods(std::move(methods_))
    {}
    
    Token name;
    std::vector<Stmt> methods;
};

struct ReturnStmt
{
    ReturnStmt(Token keyword_, Expr value_) : keyword(keyword_), value(value_) {}
    Token keyword;
    Expr value;
};

class Return : public std::runtime_error
{
public:
    nullable_literal value;
    explicit Return(nullable_literal value_) : std::runtime_error(""), value(value_)
    {
    }
};


void deleteStmt(Stmt statement)
{
    return std::visit(overloaded {
        [](const PrintStmt* stmt)
        {
            deleteExpr(stmt->expression);
            delete stmt;
        },
        [](const ExpressionStmt* stmt)
        {
            deleteExpr(stmt->expression);
            delete stmt;
        },
        [](const VarStmt* stmt)
        {
            deleteExpr(stmt->initializer);
            delete stmt;
        },
        [](const IfStmt* stmt)
        {
            deleteExpr(stmt->condition);
            deleteStmt(stmt->thenBranch);
            deleteStmt(stmt->elseBranch);
            delete stmt;
        },
        [](const WhileStmt* stmt)
        {
            deleteExpr(stmt->condition);
            deleteStmt(stmt->body);
        },
        [](const BlockStmt* stmt)
        {
            for(const Stmt& statement: stmt->statements)
                deleteStmt(statement);
        },
        [](const FunctionStmt* stmt)
        {
            for(const Stmt& statement: stmt->body)
                deleteStmt(statement);
            delete stmt;
        },
        [](const ClassStmt* stmt)
        {
            for(const Stmt& statement: stmt->methods)
                deleteStmt(statement);
            delete stmt;
        },
        [](const ReturnStmt* stmt)
        {
            deleteExpr(stmt->value);
            delete stmt;
        },
        [](const std::nullptr_t stmt)
        {
        }
    }, statement);
}
