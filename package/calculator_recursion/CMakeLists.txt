cmake_minimum_required(VERSION 3.27)
project(calculator_recursion C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(calculator_recursion
        codeGen.c
        codeGen.h
        lex.c
        lex.h
        main.c
        parser.c
        parser.h)
