#pragma once

#include <unordered_map>

#include "lexer.h"

class Environment
{
public:
    Environment() : enclosing(NULL) {}
    Environment(Environment* enclosing_) : enclosing(enclosing) {}
    
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
    Environment* enclosing;
};
