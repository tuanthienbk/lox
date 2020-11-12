cmake_minimum_required(VERSION 3.1.0)

project(lox VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.72.0)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

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

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
