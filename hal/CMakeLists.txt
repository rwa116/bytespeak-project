# CMakeList.txt for HAL
#   Build a library (`hal`) which exposes the header files as "hal/*.hpp"

include_directories(hal/include ../pru-bytespeak/)
include_directories(../app/include)
file(GLOB MY_SOURCES "src/*.cpp")

add_library(hal STATIC ${MY_SOURCES})

target_include_directories(hal PUBLIC include)
