#pragma once
#include "token.h"

static bool has_error = false;
static bool has_runtime_error = false;

void report(int line, const std::string& where, const std::string& message)
{
    std::cout << "[line " << line << "] Error" << where << ": " << message << "\n";
    has_error = true;
}

void error(int line, const std::string& message)
{
    report(line, "", message);
}

void report_error(const Token& token, const std::string& message)
{
    if (token.type == ENDOFFILE)
        report(token.line, " at end", message);
    else
        report(token.line, "  at '" + token.lexeme + "'", message);
}

struct RuntimeError : public std::runtime_error
{
    using base = std::runtime_error;
    Token token;
    RuntimeError(Token t, const std::string& msg) : token(t), base(msg)
    {
    }
};

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

