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

struct RuntimeError : public std::runtime_error
{
    using base = std::runtime_error;
    Token token;
    RuntimeError(Token t, const std::string& msg) : token(t), base(msg)
    {
    }
};

