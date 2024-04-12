# CMake Build Configuration for root of project
cmake_minimum_required(VERSION 3.18)
project(my_byte_speaker VERSION 1.0 DESCRIPTION "Byte Speaker" LANGUAGES CXX)


# Compiler options (inherited by sub-folders)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror -Wpedantic -Wextra -g)
add_compile_options(-fdiagnostics-color)

# Enable address sanitizer
# (Comment this out to make your code faster)
# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

# # Set ASAN_OPTIONS environment variable
# set(ASAN_OPTIONS "verbosity=2")

# What folders to build
add_subdirectory(hal)  
add_subdirectory(app)
add_subdirectory(pru-bytespeak)

