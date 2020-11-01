#pragma once
#include "parser.h"
#include "environment.h"
#include "function.h"

bool is_truthy(nullable_literal literal)
{
    if (literal == std::nullopt)
        return false;
    
    if (auto ptruth = std::get_if<bool>(&literal.value()))
        return *ptruth;
    else
        return true;
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



void runtime_error(const RuntimeError& error)
{
    std::cout << error.what() << "\n[line " << error.token.line << "]";
    has_runtime_error = true;
}

class Interpreter
{
    friend Function;
public:
    Interpreter()
    {
        Callable * clock_fcn = new ClockGlobalFcn();
        m_globals.define("clock", clock_fcn);
        m_environment = &m_globals;
    }
    
    ~Interpreter()
    {
    }
    
    void interpret(const std::vector<Stmt>& statements)
    {
        try
        {
           for(const Stmt statement : statements)
           {
               execute(statement);
           }
        }
        catch (RuntimeError error)
        {
            runtime_error(error);
        }
        catch (...)
        {
            std::cout << "Unknown error" << std::endl;
        }
    }
private:
    void execute(Stmt statement)
    {
        return std::visit(overloaded {
            [this](const PrintStmt* stmt)
            {
                nullable_literal value = evaluate(stmt->expression);
                std::cout << stringify(value) << "\n";
            },
            [this](const ExpressionStmt* stmt)
            {
                evaluate(stmt->expression);
            },
            [this](const VarStmt* stmt)
            {
                nullable_literal value = evaluate(stmt->initializer);
                m_environment->define(stmt->name.lexeme, value);
            },
            [this](const IfStmt* stmt)
            {
                if (is_truthy(evaluate(stmt->condition)))
                    execute(stmt->thenBranch);
                else
                    execute(stmt->elseBranch);
            },
            [this](const WhileStmt* stmt)
            {
                while (is_truthy(evaluate(stmt->condition)))
                    execute(stmt->body);
            },
            [this](const BlockStmt* stmt)
            {
                Environment current_env(m_environment);
                execute_block(stmt->statements, &current_env);
            },
            [this](const FunctionStmt* stmt)
            {
                Callable* function = new Function(const_cast<FunctionStmt*>(stmt));
                m_environment->define(stmt->name.lexeme, function);
            },
            [this](const ReturnStmt* stmt)
            {
                nullable_literal value = std::nullopt;
                if (!std::get_if<std::nullptr_t>(&stmt->value))
                    value = evaluate(stmt->value);
                throw Return(value);
            },
            [this](const std::nullptr_t stmt)
            {
            }
        }, statement);
    }
    
    void execute_block(const std::vector<Stmt>& statements, Environment* env)
    {
        //Environment* previous = m_environment;
        ScopeEnvironment scopeEnv(&m_environment);
        try
        {
            m_environment = env;
            for(auto& stmt : statements)
                execute(stmt);
        }
        catch (const Return& return_value)
        {
            throw;
        }
    }
    
    nullable_literal evaluate(Expr expr)
    {
        return std::visit(overloaded {
            [this](const Assign* expr) -> nullable_literal
            {
                nullable_literal value = evaluate(expr->value);
                m_environment->assign(expr->name, value);
                return value;
            },
            [this](const Binary* expr) -> nullable_literal
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
                        throw RuntimeError(expr->op, "Operand must be number or string");
                    }
                    case MINUS:
                    {
                        auto v1 = std::get_if<double>(&left.value());
                        auto v2 = std::get_if<double>(&right.value());
                        if (v1 && v2)
                            return *v1 - *v2;
                        else
                            throw RuntimeError(expr->op, "Operand must be number");
                    }
                    case SLASH:
                        {
                            auto v1 = std::get_if<double>(&left.value());
                            auto v2 = std::get_if<double>(&right.value());
                            if (v1 && v2)
                                return *v1 / *v2;
                            else
                                throw RuntimeError(expr->op, "Operand must be number");
                        }
                    case STAR:
                        {
                            auto v1 = std::get_if<double>(&left.value());
                            auto v2 = std::get_if<double>(&right.value());
                            if (v1 && v2)
                                return (*v1) * (*v2);
                            else
                                throw RuntimeError(expr->op, "Operand must be number");
                        }
                    case GREATER:
                        {
                            auto v1 = std::get_if<double>(&left.value());
                            auto v2 = std::get_if<double>(&right.value());
                            if (v1 && v2)
                                return *v1 > *v2;
                            else
                                throw RuntimeError(expr->op, "Operand must be number");
                        }
                    case GREATER_EQUAL:
                        {
                            auto v1 = std::get_if<double>(&left.value());
                            auto v2 = std::get_if<double>(&right.value());
                            if (v1 && v2)
                                return *v1 >= *v2;
                            else
                                throw RuntimeError(expr->op, "Operand must be number");
                        }
                    case LESS:
                        {
                            auto v1 = std::get_if<double>(&left.value());
                            auto v2 = std::get_if<double>(&right.value());
                            if (v1 && v2)
                                return *v1 < *v2;
                            else
                                throw RuntimeError(expr->op, "Operand must be number");
                        }
                    case LESS_EQUAL:
                        {
                            auto v1 = std::get_if<double>(&left.value());
                            auto v2 = std::get_if<double>(&right.value());
                            if (v1 && v2)
                                return *v1 <= *v2;
                            else
                                throw RuntimeError(expr->op, "Operand must be number");
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
            [this](const Logical* expr) -> nullable_literal
            {
                nullable_literal left = evaluate(expr->left);
                
                if (expr->op.type == TokenType::OR)
                {
                  if (is_truthy(left)) return left;
                }
                else
                {
                  if (!is_truthy(left)) return left;
                }

                return evaluate(expr->right);
            },
            [this](const Call* expr) -> nullable_literal
            {
                nullable_literal callee = evaluate(expr->callee);

                std::vector<nullable_literal> arguments;
                for (auto argument : expr->arguments)
                {
                  arguments.push_back(evaluate(argument));
                }
                
                if (auto fptr = std::get_if<Callable*>(&callee.value()))
                {
                    if (arguments.size() != (*fptr)->arity())
                        throw RuntimeError(expr->paren, "expected " + std::to_string((*fptr)->arity()) + " arguments but got "
                                           + std::to_string(arguments.size()) + "." );
                    return (*fptr)->call(this, arguments);
                }
                else
                {
                    throw RuntimeError(expr->paren, "can only call functions and classes");
                }
                
            },
            [this](const Grouping* expr) -> nullable_literal
            {
                return evaluate(expr->expression);
            },
            [this](const Literal* expr) -> nullable_literal
            {
                return expr->value;
            },
            [this](const Unary* expr) -> nullable_literal
            {
                nullable_literal right = evaluate(expr->right);
                switch (expr->op.type)
                {
                    case MINUS:
                    {
                        if (auto vptr = std::get_if<double>(&right.value()))
                            return -(*vptr);
                        else
                            throw RuntimeError(expr->op, "Operand must be number");
                    }
                    case BANG:
                        return !is_truthy(right);
                    default:
                        return std::nullopt;
                }
            },
            [this](const Variable* expr) -> nullable_literal
            {
                return m_environment->get(expr->name);
            }
            ,
            [this](const std::nullptr_t expr) -> nullable_literal
            {
                return std::nullopt;
            }
        }, expr);
    }

private:
    Environment* m_environment;
    Environment m_globals;
};


nullable_literal Function::call(Interpreter* interpreter, std::vector<nullable_literal>& arguments)
{
    Environment env(&interpreter->m_globals);
    for (int i = 0; i < declaration->params.size(); i++)
    {
        env.define(declaration->params[i].lexeme, arguments[i]);
    }
    
    try
    {
        interpreter->execute_block(declaration->body, &env);
    }
    catch (const Return& return_value)
    {
        return return_value.value;
    }
    catch (...)
    {
        std::cout << "something wrong" << std::endl;
        return;
    }
    return std::nullopt;
}

std::string Function::to_string()
{
    return std::string("<fn " + declaration->name.lexeme + ">");
}

int Function::arity()
{
    return (int)declaration->params.size();
}
