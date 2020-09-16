#pragma once
#include "parser.h"

bool is_truthy(nullable_literal literal)
{
    if (literal == std::nullopt)
        return false;
    
    if (auto ptruth = std::get_if<bool>(&literal.value()))
        return *ptruth;
    else
        return false;
}

bool is_equal(nullable_literal a, nullable_literal b)
{
    if (a == std::nullopt && b == std::nullopt)
        return true;
    else if (a == std::nullopt)
        return false;
    else if (b == std::nullopt)
        return false;
    else
        return a.value() == b.value();
}

struct RuntimError : public std::runtime_error
{
    using base = std::runtime_error;
    Token token;
    RuntimError(Token t, const std::string& msg) : token(t), base(msg)
    {
    }
};

std::string stringify(nullable_literal literal)
{
    if (literal == std::nullopt) return "nil";
    
    if (auto doublevalueptr = std::get_if<double>(&literal.value()))
    {
        double doublevalue = *doublevalueptr;
        if (doublevalue == std::floor(doublevalue))
            return std::to_string((int)doublevalue);
        else
            return std::to_string(doublevalue);
    }
    else if (auto boolvalueptr = std::get_if<bool>(&literal.value()))
    {
        if(*boolvalueptr) return "true"; else return "false";
    }
    else
    {
        return std::get<std::string>(literal.value());
    }
}

nullable_literal evaluate(Expr expr)
{
    return std::visit(overloaded {
        [](const Binary* expr) -> nullable_literal
        {
            nullable_literal left = evaluate(expr->left);
            nullable_literal right = evaluate(expr->right);
            switch (expr->op.type)
            {
                case PLUS:
                {
                    auto v1 = std::get_if<double>(&left.value());
                    auto v2 = std::get_if<double>(&right.value());
                    if (v1 && v2)
                        return *v1 + *v2;
                    else
                    {
                        auto s1 = std::get_if<std::string>(&left.value());
                        auto s2 = std::get_if<std::string>(&right.value());
                        if (s1 && s2)
                            return *s1 + *s2;
                    }
                    throw RuntimError(expr->op, "Operand must be number or string");
                }
                case MINUS:
                {
                    auto v1 = std::get_if<double>(&left.value());
                    auto v2 = std::get_if<double>(&right.value());
                    if (v1 && v2)
                        return *v1 - *v2;
                    else
                        throw RuntimError(expr->op, "Operand must be number");
                }
                case SLASH:
                    {
                        auto v1 = std::get_if<double>(&left.value());
                        auto v2 = std::get_if<double>(&right.value());
                        if (v1 && v2)
                            return *v1 / *v2;
                        else
                            throw RuntimError(expr->op, "Operand must be number");
                    }
                case STAR:
                    {
                        auto v1 = std::get_if<double>(&left.value());
                        auto v2 = std::get_if<double>(&right.value());
                        if (v1 && v2)
                            return (*v1) * (*v2);
                        else
                            throw RuntimError(expr->op, "Operand must be number");
                    }
                case GREATER:
                    {
                        auto v1 = std::get_if<double>(&left.value());
                        auto v2 = std::get_if<double>(&right.value());
                        if (v1 && v2)
                            return *v1 > *v2;
                        else
                            throw RuntimError(expr->op, "Operand must be number");
                    }
                case GREATER_EQUAL:
                    {
                        auto v1 = std::get_if<double>(&left.value());
                        auto v2 = std::get_if<double>(&right.value());
                        if (v1 && v2)
                            return *v1 >= *v2;
                        else
                            throw RuntimError(expr->op, "Operand must be number");
                    }
                case LESS:
                    {
                        auto v1 = std::get_if<double>(&left.value());
                        auto v2 = std::get_if<double>(&right.value());
                        if (v1 && v2)
                            return *v1 < *v2;
                        else
                            throw RuntimError(expr->op, "Operand must be number");
                    }
                case LESS_EQUAL:
                    {
                        auto v1 = std::get_if<double>(&left.value());
                        auto v2 = std::get_if<double>(&right.value());
                        if (v1 && v2)
                            return *v1 <= *v2;
                        else
                            throw RuntimError(expr->op, "Operand must be number");
                    }
                case EQUAL:
                    {
                        return is_equal(left, right);
                    }
                case BANG_EQUAL:
                    {
                        return !is_equal(left, right);
                    }
                default:
                    return std::nullopt;
            }
        },
        [](const Grouping* expr) -> nullable_literal
        {
            return evaluate(expr->expression);
        },
        [](const Literal* expr) -> nullable_literal
        {
            return expr->value;
        },
        [](const Unary* expr) -> nullable_literal
        {
            nullable_literal right = evaluate(expr->right);
            switch (expr->op.type)
            {
                case MINUS:
                {
                    if (auto vptr = std::get_if<double>(&right.value()))
                        return -(*vptr);
                    else
                        throw RuntimError(expr->op, "Operand must be number");
                }
                case BANG:
                    return !is_truthy(right);
                default:
                    return std::nullopt;
            }
        }
    }, expr);
}

extern void runtime_error(RuntimError error);

void interpret(Expr expression)
{
    try
    {
        nullable_literal result = evaluate(expression);
        std::cout << stringify(result) << "\n";
    }
    catch (RuntimError error)
    {
        runtime_error(error);
    }
}

