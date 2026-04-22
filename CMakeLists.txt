cmake_minimum_required(VERSION 3.10)
project(train_ticket_system)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable some warnings that might be triggered by our custom containers
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

add_executable(code main.cpp)