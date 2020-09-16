#include <iostream>
#include <fstream>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"


static bool has_error = false;
static bool has_runtime_error = false;

void error(int line, std::string message);
void error(Token token, std::string message);
void report(int line, std::string where, std::string message);
void runtime_error(RuntimError error);
void run_file(const char *);
void run_prompt();
void run(std::string&);

int main(int argc, char** argv)
{
//    Expr expression = new Binary(
//    new Unary(
//        Token(TokenType::MINUS, "-", NULL, 1),
//        new Literal(123)),
//    Token(TokenType::STAR, "*", NULL, 1),
//    new Grouping(
//        new Literal(45.67)));
//
//    std::cout<< printAST(expression) << std::endl;
//
//    deleteAST(expression);
    
	if (argc > 2)
	{
		std::cout << "Usage: lox [script]" << std::endl;
	}
	else if (argc == 2)
	{
		run_file(argv[0]);
	}
	else
	{
		run_prompt();
	}

	return 0;
}

void error(int line, std::string message)
{
    report(line, "", message);
}

void error(Token token, std::string message)
{
    if (token.type == ENDOFFILE)
        report(token.line, " at end", message);
    else
        report(token.line, "  at '" + token.lexeme + "'", message);
}

void runtime_error(RuntimError error)
{
    std::cout << error.what() << "\n[line " << error.token.line << "]";
    has_runtime_error = true;
}

void report(int line, std::string where, std::string message)
{
    std::cout << "[line " << line << "] Error" << where << ": " << message << "\n";
    has_error = true;
}

std::string read_all_bytes(const char * filename)
{
    std::ifstream in(filename);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(std::ios::beg);
        in.read((char*)&contents[0], contents.size());
        return contents;
    }
    else
    {
        throw std::runtime_error("couldn't open the file");
    }
}

void run_file(const char * filename)
{
    std::string contents = read_all_bytes(filename);
    run(contents);
}

void run_prompt()
{
    for(;;)
    {
        std::cout << "> ";
        std::string line;
        if (!std::getline(std::cin, line))
            break;
        run(line);
    }
    std::cout << std::endl;
}

void run(std::string& source)
{
    Lexer lex(source);
    std::vector<Token> tokens = lex.scan_tokens();
    Parser parser(tokens);
    Expr expression = parser.parse();
    if (has_error) exit(65);
    Binary * binary = std::get<Binary*>(expression);
    //std::cout<< printAST(expression) << std::endl;
    interpret(expression);
    
    if (has_runtime_error) exit(70);
}

