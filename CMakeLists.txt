cmake_minimum_required(VERSION 3.1.0)

project(lox VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_STANDARD 17)

set(HEADERS
    environment.h
    error.h
    token.h
    literal.h
    function.h
    class.h
    expression.h
    statement.h
	lexer.h
	parser.h
    resolver.h
	interpreter.h
)

set(SOURCES
	main.cpp
)

add_executable(jlox ${SOURCES} ${HEADERS})

set(CLOX_HEADERS
    clox/common.h
    clox/chunk.h
    clox/memory.h
    clox/value.h
    clox/table.h
    clox/object.h
    clox/debug.h
    clox/vm.h
    clox/compiler.h
    clox/scanner.h
)

set(CLOX_SOURCES
    clox/chunk.cpp
    clox/memory.cpp
    clox/value.cpp
    clox/table.cpp
    clox/object.cpp
    clox/debug.cpp
    clox/vm.cpp
    clox/compiler.cpp
    clox/scanner.cpp
    clox/main.cpp
)

add_executable(clox ${CLOX_SOURCES} ${CLOX_HEADERS})
