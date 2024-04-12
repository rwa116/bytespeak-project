# Build the app, using the HAL

include_directories(include ../pru-bytespeak/)
file(GLOB MY_SOURCES "src/*.cpp")
add_executable(byte_speaker ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(byte_speaker LINK_PRIVATE hal)

# ALSA support
find_package(ALSA REQUIRED)
target_link_libraries(byte_speaker LINK_PRIVATE asound)

# PTHREAD support
find_package(Threads REQUIRED)
target_link_libraries(byte_speaker LINK_PRIVATE Threads::Threads)

# Copy executable to final location (change `hello_world` to project name as needed)
add_custom_command(TARGET byte_speaker POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:byte_speaker>"
     "~/cmpt433/public/myApps/byte_speaker" 
  COMMENT "Copying ARM executable to public NFS directory")

# Copy the WAV folder to NFS
add_custom_command(TARGET byte_speaker POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy_directory
   "${CMAKE_SOURCE_DIR}/bytespeak-wav-files"
   "~/cmpt433/public/myApps/bytespeak-wav-files" 
COMMENT "Copying WAVE files to public NFS directory")

add_custom_command(TARGET byte_speaker POST_BUILD 
COMMAND find ~/cmpt433/public/myApps/bytespeak-wav-files -type f ! -name 'custom*' -exec chmod 777 {} +
COMMENT "Giving perms to WAVE files in public NFS directory")

# Copy the WAV folder to NFS
add_custom_command(TARGET byte_speaker POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy_directory
   "${CMAKE_SOURCE_DIR}/nodeServer"
   "~/cmpt433/public/myApps/bytespeaker-server" 
COMMENT "Copying nodeServer files to public NFS directory")

# Copy the translator application to NFC
add_custom_command(TARGET byte_speaker POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy
   "${CMAKE_SOURCE_DIR}/trans"
   "~/cmpt433/public/myApps/trans" 
COMMENT "Copying translator application to public NFS directory")

# Copy the voices to NFC
add_custom_command(TARGET byte_speaker POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy_directory
   "${CMAKE_SOURCE_DIR}/bytespeak-voices"
   "~/cmpt433/public/myApps/bytespeak-voices" 
COMMENT "Copying bytespeak-voices to public NFS directory")

# Copy the piper application to NFC
add_custom_command(TARGET byte_speaker POST_BUILD 
COMMAND "${CMAKE_COMMAND}" -E copy_directory
   "${CMAKE_SOURCE_DIR}/piper"
   "~/cmpt433/public/myApps/piper" 
COMMENT "Copying piper application to public NFS directory")