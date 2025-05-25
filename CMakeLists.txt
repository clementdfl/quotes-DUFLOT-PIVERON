cmake_minimum_required(VERSION 3.30)
project(quotes-DUFLOT-PIVERON_1 C)

set(SOURCES main.c test.c bmp8.c bmp24.c)
set(CMAKE_C_STANDARD 11)

add_executable(quotes-DUFLOT-PIVERON_1 ${SOURCES})