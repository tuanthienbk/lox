#pragma once
#include <string>
#include <iostream>
#include <variant>
#include <optional>

class Interpreter;
class Callable;

using literal_t = std::variant<double, bool, std::string, Callable*>;
using nullable_literal = std::optional<literal_t>;

class Callable
{
public:
    virtual ~Callable() {}
    virtual nullable_literal call(Interpreter* interpreter, std::vector<nullable_literal>& arguments) = 0;
    virtual std::string to_string() = 0;
    virtual int arity() = 0;
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


