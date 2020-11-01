#pragma once
#include "literal.h"
#include "environment.h"
#include "statement.h"

#include <chrono>

class Function : public Callable
{
public:
    Function(FunctionStmt* decl, std::shared_ptr<Environment> closure) : m_declaration(decl), m_closure(closure) {}
    
    nullable_literal call(Interpreter* interpreter, std::vector<nullable_literal>& arguments) override;
    std::string to_string() override;
    int arity() override;
private:
    FunctionStmt* m_declaration;
    std::shared_ptr<Environment> m_closure;
};

class ClockGlobalFcn : public Callable
{
public:
    nullable_literal call(Interpreter* interpreter, std::vector<nullable_literal>& arguments) override
    {
        return (double)std::chrono::steady_clock::now().time_since_epoch().count();
    }
    
    std::string to_string() override
    {
        return std::string("native fn - clock");
    }
    
    int arity() override
    {
        return 0;
    }
};
