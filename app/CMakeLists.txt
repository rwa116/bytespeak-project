# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.cpp")
add_executable(byte_speaker ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(byte_speaker LINK_PRIVATE hal)

# Copy executable to final location (change `hello_world` to project name as needed)
add_custom_command(TARGET byte_speaker POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:byte_speaker>"
     "~/cmpt433/public/myApps/byte_speaker" 
  COMMENT "Copying ARM executable to public NFS directory")
