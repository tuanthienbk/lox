#include "lexer.h"

struct Assign;
struct Binary;
struct Grouping;
struct Literal;
struct Logical;
struct Unary;
struct Variable;

using Expr = std::variant<Assign*, Binary*, Grouping*, Literal*, Logical*, Unary*, Variable*, std::nullptr_t>;

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

std::string printAST(Expr expr);

template<class T>
void parenthesize_helper(std::stringstream& ss, T expr)
{
    ss << " ";
    ss << printAST(expr);
}

template<class T, class... Ts>
void parenthesize_helper(std::stringstream& ss, T expr, Ts... exprs)
{
    ss << " ";
    ss << printAST(expr);
    parenthesize_helper(ss, exprs...);
}

template<class... Ts>
std::string parenthesize(std::string name, Ts... exprs)
{
    std::stringstream ss;
    ss << "(" << name;
    parenthesize_helper(ss, exprs...);
    ss << ")";
    return ss.str();
}

std::string printAST(Expr expr)
{
    return std::visit(overloaded {
        [](const Assign* expr)
        {
            return std::string("");
        },
        [](const Binary* expr)
        {
            return parenthesize(expr->op.lexeme, expr->left, expr->right);
        },
        [](const Logical* expr)
        {
            return parenthesize(expr->op.lexeme, expr->left, expr->right);
        },
        [](const Grouping* expr)
        {
            return parenthesize("group", expr->expression);
        },
        [](const Literal* expr)
        {
            return nullable_literal_to_string(expr->value);
        },
        [](const Unary* expr)
        {
            return parenthesize(expr->op.lexeme, expr->right);
        },
        [](const Variable* expr)
        {
            return expr->name.lexeme;
        },
        [](const std::nullptr_t expr)
        {
            return std::string("");
        }
    }, expr);
}

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

