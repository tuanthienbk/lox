#pragma once
#include "literal.h"
#include <vector>


enum TokenType {
  // Single-character tokens.
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

  // One or two character tokens.
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    // Literals.
    IDENTIFIER,
    STRING,
    NUMBER,

  // Keywords.
    AND,
    CLASS,
    ELSE,
    FALSE,
    FUN,
    FOR,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,
    ENDOFFILE
};

std::vector<std::string> TokenString =
{
    "LEFT_PAREN",
    "RIGHT_PAREN",
    "LEFT_BRACE",
    "RIGHT_BRACE",
    "COMMA",
    "DOT",
    "MINUS",
    "PLUS",
    "SEMICOLON",
    "SLASH",
    "STAR",

    // One or two character tokens.
    "BANG",
    "BANG_EQUAL",
    "EQUAL",
    "EQUAL_EQUAL",
    "GREATER",
    "GREATER_EQUAL",
    "LESS",
    "LESS_EQUAL",

    // Literals.
    "IDENTIFIER",
    "STRING",
    "NUMBER",

    // Keywords.
    "AND",
    "CLASS",
    "ELSE",
    "FALSE",
    "FUN",
    "FOR",
    "IF",
    "NIL",
    "OR",
    "PRINT",
    "RETURN",
    "SUPER",
    "THIS",
    "TRUE",
    "VAR",
    "WHILE",

    "EOF"
};


struct Token
{
    TokenType type;
    std::string lexeme;
    nullable_literal literal;
    int line;

    Token(TokenType type_, std::string lexeme_, nullable_literal literal_, int line_)
    : type(type_)
    , lexeme(lexeme_)
    , literal(literal_)
    , line(line_)
    {
    }

    std::string to_string()
    {
        return TokenString[(int)type] + " " + lexeme + " " + nullable_literal_to_string(literal);
    }

    friend std::ostream& operator<<(std::ostream& os, Token& token)
    {
        os << token.to_string();
        return os;
    }
};
