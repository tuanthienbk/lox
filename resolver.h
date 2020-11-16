#pragma once

#include "statement.h"
#include "error.h"

#include <stack>

template <typename Interpreter>
class Resolver
{
private:
    Interpreter* interpreter;
    std::vector<std::unordered_map<std::string, bool>> scopes;
    
    enum class FunctionType
    {
        NONE,
        FUNCTION,
        METHOD,
        INITIALIZER
    } currentFunction;
    
    enum class ClassType
    {
        NONE,
        CLASS
    } currentClass;

public:
    Resolver(Interpreter* interpreter_) :
        interpreter(interpreter_),
        currentFunction(FunctionType::NONE),
        currentClass(ClassType::NONE)
    {}
    
    void resolve(const std::vector<Stmt>& statements)
    {
        for(const Stmt& statement : statements)
            resolve(statement);
    }
    
private:
    void resolve(Stmt statement)
    {
        return std::visit(overloaded {
            [this](const PrintStmt* stmt)
            {
                resolve(stmt->expression);
            },
            [this](const ExpressionStmt* stmt)
            {
                resolve(stmt->expression);
            },
            [this](const VarStmt* stmt)
            {
                declare(stmt->name);
                if (!std::get_if<std::nullptr_t>(&stmt->initializer))
                {
                  resolve(stmt->initializer);
                }
                define(stmt->name);
            },
            [this](const IfStmt* stmt)
            {
                resolve(stmt->condition);
                resolve(stmt->thenBranch);
                if (!std::get_if<std::nullptr_t>(&stmt->elseBranch))
                    resolve(stmt->elseBranch);
            },
            [this](const WhileStmt* stmt)
            {
                resolve(stmt->condition);
                resolve(stmt->body);
            },
            [this](const BlockStmt* stmt)
            {
                begin_scope();
                resolve(stmt->statements);
                end_scope();
            },
            [this](const FunctionStmt* stmt)
            {
                declare(stmt->name);
                define(stmt->name);
                resolveFunction(stmt, FunctionType::FUNCTION);
            },
            [this](const ClassStmt* stmt)
            {
                ClassType enclosingClass = currentClass;
                currentClass = ClassType::CLASS;
                declare(stmt->name);
                define(stmt->name);
                begin_scope();
                scopes.back()["this"] = true;
                for(auto& method: stmt->methods)
                {
                    FunctionType declaration = FunctionType::METHOD;
                    FunctionStmt* fn = std::get<FunctionStmt*>(method);
                    if (fn->name.lexeme == "init")
                        declaration = FunctionType::INITIALIZER;
                    resolveFunction(fn, declaration);
                }
                end_scope();
                currentClass = enclosingClass;
            },
            [this](const ReturnStmt* stmt)
            {
                if (currentFunction == FunctionType::NONE)
                    error(stmt->keyword, "Can't return from top-level code.");
                
                if (!std::get_if<std::nullptr_t>(&stmt->value))
                {
                    if (currentFunction == FunctionType::INITIALIZER)
                        error(stmt->keyword, "Can't return from an initializer.");
                    
                    resolve(stmt->value);
                }
            },
            [this](const std::nullptr_t stmt)
            {
            }
        }, statement);
    }
    
    void begin_scope()
    {
        scopes.push_back(std::unordered_map<std::string, bool>());
    }
    
    void end_scope()
    {
        scopes.pop_back();
    }
    
    void declare(const Token& name)
    {
        if (scopes.empty()) return;
        auto& scope = scopes.back();
        if (scope.find(name.lexeme) != scope.end())
            error(name, "already variable with this name in this scope.");
        scope[name.lexeme] = false;
    }
    
    void define(const Token& name)
    {
        if (scopes.empty()) return;
        auto& scope = scopes.back();
        scope[name.lexeme] = true;
    }
    
    void resolveFunction(const FunctionStmt* function, FunctionType functionType)
    {
        FunctionType enclosingFunction = currentFunction;
        currentFunction = functionType;
        begin_scope();
        for (auto& param : function->params)
        {
          declare(param);
          define(param);
        }
        resolve(function->body);
        end_scope();
        currentFunction = enclosingFunction;
    }
    
    void resolve(Expr expr)
    {
        return std::visit(overloaded {
            [this](const Assign* expr)
            {
                resolve(expr->value);
                resolveLocal(expr, expr->name);
            },
            [this](const Binary* expr)
            {
                resolve(expr->right);
                resolve(expr->left);
            },
            [this](const Logical* expr)
            {
                resolve(expr->right);
                resolve(expr->left);
            },
            [this](const Call* expr)
            {
                resolve(expr->callee);
                for(auto& argument : expr->arguments)
                    resolve(argument);
            },
            [this](const Get* expr)
            {
                resolve(expr->object);
            },
            [this](const Set* expr)
            {
                resolve(expr->value);
                resolve(expr->object);
            },
            [this](const Grouping* expr)
            {
                resolve(expr->expression);
            },
            [this](const Literal* expr)
            {
            },
            [this](const Unary* expr)
            {
                resolve(expr->right);
            },
            [this](const This* expr)
            {
                if (currentClass == ClassType::NONE)
                {
                    error(expr->keyword, "Can't use 'this' outside of a class");
                    return;
                }
                resolveLocal(expr, expr->keyword);
            },
            [this](const Variable* expr)
            {
                if (!scopes.empty() &&
                    scopes.back().find(expr->name.lexeme) != scopes.back().end() &&
                    !scopes.back()[expr->name.lexeme])
                {
                    error(expr->name, "Can't read local variable in its own initializer.");
                }
                resolveLocal(expr, expr->name);
            },
            [this](const std::nullptr_t expr)
            {
            }
        }, expr);
    }
    
    template <typename expr_t>
    void resolveLocal(const expr_t* expr, const Token& name)
    {
        for (int i = scopes.size() - 1; i >= 0; i--)
        {
            if (scopes[i].find(name.lexeme) != scopes[i].end())
            {
                interpreter->resolve(expr, scopes.size() - 1 - i);
                return;
            }
        }

        // Not found. Assume it is global.
    }

};
