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
    Literal ( literal_t value_ ) : value ( value_ )
    {}
    literal_t value;
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

