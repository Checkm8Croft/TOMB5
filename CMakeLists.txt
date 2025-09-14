cmake_minimum_required(VERSION 3.20)
project(tomb5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(GL_SILENCE_DEPRECATION)
add_definitions(-D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/tomb5
    ${CMAKE_CURRENT_SOURCE_DIR}/tomb5/game
    ${CMAKE_CURRENT_SOURCE_DIR}/tomb5/global
    ${CMAKE_CURRENT_SOURCE_DIR}/tomb5/specific
)

# Source files
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/tomb5/game/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tomb5/global/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tomb5/specific/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tomb5/tomb5/*.cpp"
)

# Executable
add_executable(tomb5 ${SOURCES})

# Link libraries
find_library(SDL2_LIB SDL2)
find_library(SDL2_IMAGE_LIB SDL2_image)
find_library(SDL2_MIXER_LIB SDL2_mixer)

target_link_libraries(tomb5
    PRIVATE
        ${SDL2_LIB}
        ${SDL2_IMAGE_LIB}
        ${SDL2_MIXER_LIB}
)

# macOS frameworks
if(APPLE)
    target_link_libraries(tomb5
        PRIVATE
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework AudioToolbox"
            "-framework ForceFeedback"
    )
endif()
