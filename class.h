#pragma once
#include "literal.h"
#include "environment.h"
#include "statement.h"

#include <chrono>

class Klass : public Callable
{
public:
    Klass(const std::string& name_, Klass* super_, std::unordered_map<std::string, Function*>& methods_) :
        name(name_),
        superclass(super_),
        methods(std::move(methods_))
    {}
    
    ~Klass()
    {
        for(auto& p : methods)
            delete p.second;
    }
    
    nullable_literal call(Interpreter* interpreter, std::vector<nullable_literal>& arguments) override;
    std::string to_string() override;
    int arity() override;
    
    Function* find_method(const std::string& name);
    
private:
    std::string name;
    std::unordered_map<std::string, Function*> methods;
    Klass* superclass;
};

class ClassInstance : public ClassInstanceInterface
{
private:
    Klass* klass;
    std::unordered_map<std::string, nullable_literal> fields;
public:
    ClassInstance(Klass* klass_) : klass(klass_) {}
    ~ClassInstance()
    {
        deleteLiteralMap(fields);
    }
    std::string to_string() override;
    nullable_literal get(const Token& name) override;
    void set(const Token& name, nullable_literal value) override;
};

