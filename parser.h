#pragma once

#include <variant>
#include <sstream>

#include "statement.h"


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

void report_error(const Token& token, const std::string& message)
{
    if (token.type == ENDOFFILE)
        report(token.line, " at end", message);
    else
        report(token.line, "  at '" + token.lexeme + "'", message);
}

class Parser
{
public:
    Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}
    
    std::vector<Stmt> parse()
    {
        std::vector<Stmt> statements;
        while (!is_at_end())
        {
            statements.push_back(declaration());
        }
        return statements;
    }
    
private:
    Stmt declaration()
    {
        try {
            if (match({VAR})) return var_declaration();
            return statement();
        } catch (ParseError) {
            synchronize();
            return (ExpressionStmt*)NULL;
        }
    }
    
    void synchronize()
    {
      advance();

      while (!is_at_end())
      {
        if (previous().type == SEMICOLON) return;

        switch (peek().type) {
          case CLASS:
          case FUN:
          case VAR:
          case FOR:
          case IF:
          case WHILE:
          case PRINT:
          case RETURN:
            return;
        }

        advance();
      }
    }
    
    Stmt var_declaration()
    {
        Token name = consume(IDENTIFIER, "Expect variable name");
        
        Expr initializer = nullptr;
        if (match({EQUAL}))
        {
            initializer = expression();
        }
        consume(SEMICOLON, "Expect ';'");
        return new VarStmt(name, initializer);
    }
    
    Stmt statement()
    {
        if (match({PRINT}))
            return print_statement();
        else if (match({IF}))
            return if_statement();
        else if (match({WHILE}))
            return while_statement();
        else if (match({FOR}))
            return for_statement();
        else if (match({LEFT_BRACE}))
            return block();
        else
            return expression_statement();
    }
    
    Stmt for_statement()
    {
        consume(LEFT_PAREN, "Expect '(' after 'for'.");
        Stmt initializer;
        if (match({SEMICOLON}))
        {
          initializer = nullptr;
        }
        else if (match({VAR}))
        {
          initializer = var_declaration();
        }
        else
        {
          initializer = expression_statement();
        }
        
        Expr condition = nullptr;
        if (!check(SEMICOLON))
        {
          condition = expression();
        }
        consume(SEMICOLON, "Expect ';' after loop condition.");
        
        Expr increment = nullptr;
        if (!check(RIGHT_PAREN))
        {
          increment = expression();
        }
        consume(RIGHT_PAREN, "Expect ')' after for clauses.");
        
        Stmt body = statement();
        if (std::get_if<std::nullptr_t>(&increment))
        {
            std::vector<Stmt> stmts = {body, new ExpressionStmt(increment)};
            body = new BlockSmt(stmts);
        }
        
        if (std::get_if<std::nullptr_t>(&condition))
            condition = new Literal(true);
        
        body = new WhileStmt(condition, body);
        
        if (std::get_if<std::nullptr_t>(&initializer))
        {
            std::vector<Stmt> stmts = {initializer, body};
            body = new BlockSmt(stmts);
        }
        
        return body;
    }
    
    Stmt while_statement()
    {
        consume(LEFT_PAREN, "Expect '(' after 'while'.");
        Expr condition = expression();
        consume(RIGHT_PAREN, "Expect ')' after condition.");
        Stmt body = statement();

        return new WhileStmt(condition, body);
    }
    
    Stmt if_statement()
    {
        consume(LEFT_PAREN, "Expect '(' after 'if'.");
        Expr condition = expression();
        consume(RIGHT_PAREN, "Expect ')' after if condition.");

        Stmt thenBranch = statement();
        Stmt elseBranch = nullptr;
        if (match({ELSE}))
        {
          elseBranch = statement();
        }
        
        return new IfStmt(condition, thenBranch, elseBranch);
    }
    
    Stmt block()
    {
        std::vector<Stmt> statements;
        while (!check(RIGHT_BRACE) && !is_at_end())
        {
            statements.push_back(declaration());
        }
        consume(RIGHT_BRACE, "Expect '}' after block");
        return new BlockSmt(statements);
    }
    
    Stmt print_statement()
    {
        Expr value = expression();
        consume(SEMICOLON, "Expect ';' after value.");
        return new PrintStmt(value);
    }
    
    Stmt expression_statement()
    {
        Expr value = expression();
        consume(SEMICOLON, "Expect ';' after value.");
        return new ExpressionStmt(value);
    }
    
    Expr expression() { return assignment(); }
    
    Expr assignment()
    {
        Expr expr = or_expr();

        if (match({EQUAL}))
        {
            Token equals = previous();
            Expr value = assignment();

            if (auto vptr = std::get_if<Variable*>(&expr))
            {
                Token name = (*vptr)->name;
                return new Assign(name, value);
            }
            error(equals, "Invalid assignment target.");
        }
        return expr;
    }
    
    Expr or_expr()
    {
        Expr expr = and_expr();
        while (match({OR}))
        {
            Token op = previous();
            Expr right = and_expr();
            expr = new Logical(expr, op, right);
        }
        return expr;
    }
    
    Expr and_expr()
    {
        Expr expr = equality();
        while (match({AND}))
        {
            Token op = previous();
            Expr right = equality();
            expr = new Logical(expr, op, right);
        }
        return expr;
    }
    
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
        
        if (match({IDENTIFIER}))
        {
            return new Variable(previous());
        }

        if (match({LEFT_PAREN}))
        {
          Expr expr = expression();
          consume(RIGHT_PAREN, "Expect ')' after expression.");
          return new Grouping(expr);
        }
        
        throw error(peek(), "Expect expression.");
    }
    
    Token consume(TokenType type, const std::string& msg)
    {
        if (check(type)) return advance();
        throw error(peek(), msg);
    }
    
    struct ParseError : public std::runtime_error
    {
        using base = std::runtime_error;
        using base::base;
    };
    
    ParseError error(const Token& token, const std::string& msg)
    {
        report_error(token, msg);
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

