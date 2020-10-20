#pragma once

#include <unordered_map>

#include "lexer.h"

class Environment
{
public:
    void define(const std::string& name, nullable_literal value)
    {
        values[name] = value;
    }
    
    nullable_literal get(Token name)
    {
        if (values.find(name.lexeme) != values.end())
            return values[name.lexeme];
        
        throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
    }
private:
    std::unordered_map<std::string, nullable_literal> values;
};
