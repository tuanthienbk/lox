#pragma once
#include <string>
#include <iostream>
#include <variant>
#include <optional>

class Interpreter;
class Callable;
class ClassInstanceInterface;
struct Token;

using literal_t = std::variant<double, bool, std::string, Callable*, ClassInstanceInterface*>;
using nullable_literal = std::optional<literal_t>;

class Callable
{
public:
    virtual ~Callable() {}
    virtual nullable_literal call(Interpreter* interpreter, std::vector<nullable_literal>& arguments) = 0;
    virtual std::string to_string() = 0;
    virtual int arity() = 0;
};

class ClassInstanceInterface
{
public:
    virtual ~ClassInstanceInterface() {}
    virtual nullable_literal get(const Token& name) = 0;
    virtual void set(const Token& name, nullable_literal value) = 0;
    virtual std::string to_string() = 0;
};

std::string literal_to_string(literal_t literal)
{
    return std::visit([](auto&& arg) -> std::string
    {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, std::string>)
            return arg;
        else if constexpr (std::is_same_v<T, Callable*>)
            return arg->to_string();
        else if constexpr (std::is_same_v<T, ClassInstanceInterface*>)
            return arg->to_string();
        else
            return std::to_string(arg);
    }, literal);
}

std::string nullable_literal_to_string(nullable_literal literal)
{
    if (literal == std::nullopt)
        return "NULL";
    else
        return literal_to_string(literal.value());
}

void deleteLiteral(literal_t literal)
{
    return std::visit([](auto&& arg) -> void
    {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, Callable*>)
            delete arg;
        else if constexpr (std::is_same_v<T, ClassInstanceInterface*>)
            delete arg;
    }, literal);
}

template <typename MapType>
void deleteLiteralMap(MapType& literal_map)
{
    for(auto& value_pair : literal_map)
    {
        if (value_pair.second)
        {
            deleteLiteral(value_pair.second.value());
        }
    }
}


