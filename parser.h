#pragma once

#include <variant>
#include <sstream>

//#include <boost/preprocessor/seq/for_each.hpp>
//#include <boost/preprocessor/seq/for_each_i.hpp>
//#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>
//#include <boost/preprocessor/tuple/elem.hpp>
//#include <boost/preprocessor/punctuation/comma.hpp>
//#include <boost/preprocessor/control/if.hpp>
//#include <boost/preprocessor/arithmetic/sub.hpp>
//#include <boost/preprocessor/arithmetic/add.hpp>
//#include <boost/preprocessor/seq/size.hpp>
//
//#define DECLARE_DATA_MEMBER(R,_,i,TYPE_AND_NAME) \
//BOOST_PP_TUPLE_ELEM(2,0,TYPE_AND_NAME) BOOST_PP_TUPLE_ELEM(2,1,TYPE_AND_NAME);
//
//#define DECLARE_CONSTRUCTOR_ARGUMENT(R,DATA,i,TYPE_AND_NAME) \
//BOOST_PP_TUPLE_ELEM(2,0,TYPE_AND_NAME) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,1,TYPE_AND_NAME),_) BOOST_PP_IF(BOOST_PP_SUB(DATA,BOOST_PP_ADD(i,1)),BOOST_PP_COMMA, BOOST_PP_EMPTY)()
//
//#define INITIALIZE_CONSTRUCTOR_ARGUMENT(R,DATA,i,TYPE_AND_NAME) \
//BOOST_PP_TUPLE_ELEM(2,1,TYPE_AND_NAME) ( BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,1,TYPE_AND_NAME),_) ) BOOST_PP_IF(BOOST_PP_SUB(DATA,BOOST_PP_ADD(i,1)),BOOST_PP_COMMA, BOOST_PP_EMPTY)()
//
//#define DEFINE_CLASS(R,DATA,TYPES_AND_NAMES) \
//struct BOOST_PP_TUPLE_ELEM(2,0,TYPES_AND_NAMES) : public DATA \
//{\
//BOOST_PP_TUPLE_ELEM(2,0,TYPES_AND_NAMES) ( BOOST_PP_SEQ_FOR_EACH_I(DECLARE_CONSTRUCTOR_ARGUMENT,BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_SEQ_TO_SEQ(BOOST_PP_TUPLE_ELEM(2,1,TYPES_AND_NAMES))),BOOST_PP_VARIADIC_SEQ_TO_SEQ(BOOST_PP_TUPLE_ELEM(2,1,TYPES_AND_NAMES))) ) \
//: BOOST_PP_SEQ_FOR_EACH_I(INITIALIZE_CONSTRUCTOR_ARGUMENT,BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_SEQ_TO_SEQ(BOOST_PP_TUPLE_ELEM(2,1,TYPES_AND_NAMES))),BOOST_PP_VARIADIC_SEQ_TO_SEQ(BOOST_PP_TUPLE_ELEM(2,1,TYPES_AND_NAMES))) \
//{} \
//BOOST_PP_SEQ_FOR_EACH_I(DECLARE_DATA_MEMBER,_,BOOST_PP_VARIADIC_SEQ_TO_SEQ(BOOST_PP_TUPLE_ELEM(2,1,TYPES_AND_NAMES)))\
//};\
//
//#define DEFINE_AST(BASE_CLASS, TYPES)\
//struct BASE_CLASS {}; \
//BOOST_PP_SEQ_FOR_EACH(DEFINE_CLASS,BASE_CLASS,BOOST_PP_VARIADIC_SEQ_TO_SEQ(TYPES)) \
//
//DEFINE_AST(Expr,    (Binary, (Expr,left)(Token,op)(Expr,right))\
//                    (Grouping, (Expr,expression))\
//                    (Literal, (literal_t,value))\
//                    (Unary, (Token,op)(Expr,right))\
//           )

struct Binary;
struct Grouping;
struct Literal;
struct Unary;

using Expr = std::variant<Binary*, Grouping*, Literal*, Unary*>;

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

struct Unary
{
    Unary ( Token op_ , Expr right_ ) : op ( op_ ) , right ( right_ )
    {}
    Token op;
    Expr right;
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
        [](const Binary* expr)
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
        }
    }, expr);
}

void deleteAST(Expr expr)
{
    return std::visit(overloaded {
        [](const Binary* expr)
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
        }
    }, expr);
}


extern void error(Token token, std::string message);

class Parser
{
public:
    Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}
    
    Expr parse()
    {
        try
        {
            return expression();
        }
        catch (ParseError error)
        {
            return (Binary*)NULL;
        }
    }
    
private:
    Expr expression() { return equality(); }
    
    Expr equality()
    {
        Expr expr = comparison();
        while (match({BANG_EQUAL, EQUAL_EQUAL}))
        {
            Token& op = previous();
            Expr right = comparison();
            expr = new Binary(expr, op, right);
        }
        return expr;
    }
    
    Expr comparison()
    {
        Expr expr = addition();
        while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL}))
        {
            Token& op = previous();
            Expr right = addition();
            expr = new Binary(expr, op, right);
        }
        return expr;
    }
    
    Expr addition()
    {
        Expr expr = multiplication();
        while (match({MINUS, PLUS}))
        {
            Token& op = previous();
            Expr right = multiplication();
            expr = new Binary(expr, op, right);
        }
        return expr;
    }
    
    Expr multiplication()
    {
        Expr expr = unary();
        while (match({SLASH, STAR}))
        {
            Token& op = previous();
            Expr right = unary();
            expr = new Binary(expr, op, right);
        }
        return expr;
    }
    
    Expr unary()
    {
        if (match({BANG, MINUS}))
        {
            Token& op = previous();
            Expr right = unary();
            return new Unary(op, right);
        }
        return primary();
    }
    
    Expr primary()
    {
        if (match({FALSE})) return new Literal(false);
        if (match({TRUE})) return new Literal(true);
        if (match({NIL})) return new Literal(std::nullopt);

        if (match({NUMBER, STRING}))
        {
          return new Literal(previous().literal);
        }

        if (match({LEFT_PAREN}))
        {
          Expr expr = expression();
          consume(RIGHT_PAREN, "Expect ')' after expression.");
          return new Grouping(expr);
        }
        
        throw error(peek(), "Expect expression.");
    }
    
    Token consume(TokenType type, std::string msg)
    {
        if (check(type)) return advance();
        throw error(peek(), msg);
    }
    
    struct ParseError : public std::runtime_error
    {
        using base = std::runtime_error;
        using base::base;
    };
    
    ParseError error(Token token, std::string msg)
    {
        error(token, msg);
        return ParseError("");
    }
    
    
    
    bool match(std::initializer_list<TokenType> types)
    {
        for(const TokenType& type : types)
        {
            if (check(type))
            {
                advance();
                return true;
            }
        }
        return false;
    }
    
    bool check(const TokenType& type)
    {
        if (is_at_end()) return false;
        return peek().type == type;
    }
    
    Token advance()
    {
        if (!is_at_end()) m_current++;
        return previous();
    }
    
    bool is_at_end()
    {
        return peek().type == ENDOFFILE;
    }
    
    Token& peek()
    {
        return m_tokens[m_current];
    }
    
    Token& previous()
    {
        return m_tokens[m_current - 1];
    }

private:
    std::vector<Token> m_tokens;
    int m_current = 0;
};

