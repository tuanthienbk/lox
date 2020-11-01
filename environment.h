#pragma once

#include <unordered_map>

#include "token.h"

class Environment
{
public:
    Environment() : enclosing(NULL) {}
    Environment(std::shared_ptr<Environment> enclosing_) : enclosing(enclosing_) {}
    ~Environment()
    {
        for(auto& value : values)
        {
            if (value.second)
            {
                if (auto callable = std::get_if<Callable*>(&value.second.value()))
                {
                    delete (*callable);
                }
            }
        }
    }
    
    void define(const std::string& name, nullable_literal value)
    {
        values[name] = value;
    }
    
    void assign(const Token& name, nullable_literal value)
    {
        if (values.find(name.lexeme) != values.end())
        {
            values[name.lexeme] = value;
        }
        else if (enclosing)
        {
            enclosing->assign(name, value);
        }
        else
        {
            throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
        }
    }
    
    nullable_literal get(const Token& name)
    {
        if (values.find(name.lexeme) != values.end())
            return values[name.lexeme];
        
        if (enclosing)
            return enclosing->get(name);
        
        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }
private:
    std::unordered_map<std::string, nullable_literal> values;
    std::shared_ptr<Environment> enclosing;
};

class ScopeEnvironment
{
private:
    std::shared_ptr<Environment> previous;
    std::shared_ptr<Environment>* current;
public:
    ScopeEnvironment(std::shared_ptr<Environment>* env) : previous(*env), current(env) {}
    ~ScopeEnvironment()
    {
        *current = previous;
    }
};
