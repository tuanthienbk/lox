#pragma once

#include <cctype>
#include <unordered_map>

#include "error.h"

class Lexer
{
public:
    explicit Lexer(std::string& source)
    : m_source(source)
    , m_start(0)
    , m_current(0)
    , m_line(0)
    {}
    
    virtual std::vector<Token> scan_tokens();
    
    std::unordered_map<std::string, TokenType>& keywords()
    {
        static std::unordered_map<std::string, TokenType> kws =
        {
            {"and", AND},
            {"class", CLASS},
            {"else", ELSE},
            {"false", FALSE},
            {"for", FOR},
            {"fun", FUN},
            {"if", IF},
            {"nil", NIL},
            {"or", OR},
            {"print", PRINT},
            {"return", RETURN},
            {"super", SUPER},
            {"this", THIS},
            {"true", TRUE},
            {"var", VAR},
            {"while", WHILE}
        };
        
        return kws;
    }
    
private:
    bool is_at_end() { return m_current >= m_source.size(); }
    void scan_token();
    char advance() { m_current++; return m_source.at(m_current - 1); }
    void add_token(TokenType type) { add_token(type, std::nullopt); }
    void add_token(TokenType type, nullable_literal literal)
    {
        std::string str = m_source.substr(m_start, m_current - m_start);
        m_tokens.emplace_back(type, str, literal, m_line);
    }
    bool match(char expected) // conditional advance
    {
        if (is_at_end()) return false;
        if (m_source.at(m_current) != expected) return false;
        m_current++;
        return true;
    }
    
    char peek() // look ahead
    {
        if (is_at_end()) return '\0';
        return m_source.at(m_current);
    }
    
    char peeknext() // look ahead
    {
        if (m_current + 1 >= m_source.size()) return '\0';
        return m_source.at(m_current + 1);
    }
    
    void string()
    {
        while (peek() != '"' && !is_at_end())
        {
            if (peek() == '\n') m_line++;
            advance();
        }
        if (is_at_end())
        {
            error(m_line, "non-terminated string");
            return;
        }
        advance();
        nullable_literal value = m_source.substr(m_start + 1, m_current - m_start - 2);
        add_token(STRING, value);
    }
    
    void number()
    {
        while (std::isdigit(peek()))
               advance();
        if (peek() == '.' && std::isdigit(peeknext()))
        {
            advance();
            while (std::isdigit(peek()))
                advance();
        }
        add_token(NUMBER, std::stod(m_source.substr(m_start, m_current - m_start)));
    }
    
    void identifier()
    {
        auto isalphanummeric = [](char c) -> bool { return std::isdigit(c) || std::isalpha(c); };
        while (isalphanummeric(peek())) advance();
        std::string str = m_source.substr(m_start, m_current - m_start);
        auto& kw = keywords();
        if (kw.find(str) == kw.end())
            add_token(IDENTIFIER);
        else
            add_token(kw[str]);
    }
    
private:
    std::string         m_source;
    std::vector<Token>  m_tokens;
    int m_start;
    int m_current;
    int m_line;
};

std::vector<Token> Lexer::scan_tokens()
{
    while (!is_at_end())
    {
        m_start = m_current;
        scan_token();
    }
    m_tokens.emplace_back(ENDOFFILE, "", std::nullopt, m_line);
    return m_tokens;
}

void Lexer::scan_token()
{
    char c = advance();
    switch (c)
    {
        case '(': add_token(LEFT_PAREN); break;
        case ')': add_token(RIGHT_PAREN); break;
        case '{': add_token(LEFT_BRACE); break;
        case '}': add_token(RIGHT_BRACE); break;
        case ',': add_token(COMMA); break;
        case '.': add_token(DOT); break;
        case '-': add_token(MINUS); break;
        case '+': add_token(PLUS); break;
        case ';': add_token(SEMICOLON); break;
        case '*': add_token(STAR); break;
        case '!': add_token(match('=') ? BANG_EQUAL : BANG); break;
        case '=': add_token(match('=') ? EQUAL_EQUAL : EQUAL); break;
        case '<': add_token(match('=') ? LESS_EQUAL : LESS); break;
        case '>': add_token(match('=') ? GREATER_EQUAL : GREATER); break;
        case '/':
            if (match('/'))
            {
              // A comment goes until the end of the line.
              while (peek() != '\n' && !is_at_end()) advance();
            }
            else
            {
              add_token(SLASH);
            }
            break;
        case ' ':
        case '\r':
        case '\t':
          // Ignore whitespace.
          break;

        case '\n':
          m_line++;
          break;
        case '"': string(); break;
        default:
            if (std::isdigit(c))
                number();
            else if (std::isalpha(c))
                identifier();
            else
                error(m_line, "Unexpected character.");
            break;
    }
}


