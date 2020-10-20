#pragma once


static bool has_error = false;
static bool has_runtime_error = false;

void report(int line, const std::string& where, const std::string& message)
{
    std::cout << "[line " << line << "] Error" << where << ": " << message << "\n";
    has_error = true;
}



