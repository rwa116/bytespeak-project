# CMake Build Configuration for root of project
cmake_minimum_required(VERSION 3.18)
project(my_byte_speaker VERSION 1.0 DESCRIPTION "Byte Speaker" LANGUAGES CXX)

# Compiler options (inherited by sub-folders)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror -Wpedantic -Wextra)
add_compile_options(-fdiagnostics-color)

# Enable address sanitizer
# (Comment this out to make your code faster)
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

# What folders to build
add_subdirectory(hal)  
add_subdirectory(app)

