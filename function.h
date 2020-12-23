#pragma once
#include "literal.h"
#include "environment.h"
#include "statement.h"

#include <time.h>

class ClassInstance;

class Function : public Callable
{
public:
    Function(FunctionStmt* decl, std::shared_ptr<Environment> closure, bool isInitializer) :
        m_declaration(decl),
        m_closure(closure),
        m_isInitializer(isInitializer)
    {}
    
    nullable_literal call(Interpreter* interpreter, std::vector<nullable_literal>& arguments) override;
    std::string to_string() override;
    int arity() override;
    
    Function* bind(ClassInstance* instance);
private:
    FunctionStmt* m_declaration;
    std::shared_ptr<Environment> m_closure;
    bool m_isInitializer;
};

class ClockGlobalFcn : public Callable
{
public:
    nullable_literal call(Interpreter* interpreter, std::vector<nullable_literal>& arguments) override
    {
        return (double)clock() / CLOCKS_PER_SEC;
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
