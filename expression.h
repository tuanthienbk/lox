#pragma once
#include "token.h"

struct Assign;
struct Binary;
struct Call;
struct Grouping;
struct Literal;
struct Logical;
struct Unary;
struct Variable;

using Expr = std::variant<Assign*, Binary*, Call*, Grouping*, Literal*, Logical*, Unary*, Variable*, std::nullptr_t>;

struct Assign
{
    Assign (Token name_, Expr value_) : name ( name_ ), value ( value_ )
    {}
    
    Token name;
    Expr value;
};

struct Binary
{
    Binary ( Expr left_ , Token op_ , Expr right_ ) : left ( left_ ) , op ( op_ ) , right ( right_ )
    {}
    
    Expr left;
    Token op;
    Expr right;
    
};

struct Call
{
    Call (Expr callee_, Token paren_, std::vector<Expr>& arg_) : callee(callee_), paren(paren_), arguments(std::move(arg_))
    {}
    Expr callee;
    Token paren;
    std::vector<Expr> arguments;
};

struct Grouping
{
    Grouping ( Expr expression_ ) : expression ( expression_ )
    {}
    Expr expression;
};

struct Literal
{
    Literal ( nullable_literal value_ ) : value ( value_ )
    {}
    nullable_literal value;
};

struct Logical
{
    Logical ( Expr left_ , Token op_ , Expr right_ ) : left ( left_ ) , op ( op_ ) , right ( right_ )
    {}
    
    Expr left;
    Token op;
    Expr right;
};

struct Unary
{
    Unary ( Token op_ , Expr right_ ) : op ( op_ ) , right ( right_ )
    {}
    Token op;
    Expr right;
};

struct Variable
{
    Variable(Token name_): name(name_) {}
    Token name;
};

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void deleteAST(Expr expr)
{
    return std::visit(overloaded {
        [](const Assign* expr)
        {
            deleteAST(expr->value);
            delete expr;
        },
        [](const Binary* expr)
        {
            deleteAST(expr->left);
            deleteAST(expr->right);
            delete expr;
        },
        [](const Call* expr)
        {
            deleteAST(expr->callee);
            for (auto expr_arg : expr->arguments)
                deleteAST(expr_arg);
            delete expr;
        },
        [](const Logical* expr)
        {
            deleteAST(expr->left);
            deleteAST(expr->right);
            delete expr;
        },
        [](const Grouping* expr)
        {
            deleteAST(expr->expression);
            delete expr;
        },
        [](const Literal* expr)
        {
            delete expr;
        },
        [](const Unary* expr)
        {
            deleteAST(expr->right);
            delete expr;
        },
        [](const Variable* expr)
        {
            delete expr;
        },
        [](const std::nullptr_t expr)
        {
        }
    }, expr);
}

