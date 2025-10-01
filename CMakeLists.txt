cmake_minimum_required(VERSION 3.20)
project(tomb5 LANGUAGES C CXX)

# Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Definizioni generali
add_compile_definitions(GL_SILENCE_DEPRECATION)
add_definitions(-D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L)

# Percorsi Homebrew SDL2 (se servono)
set(SDL2_INCLUDE_DIR /usr/local/include/SDL2)
set(SDL2_LIBRARY /usr/local/lib/libSDL2.dylib)
set(SDL2_MIXER_LIBRARY /usr/local/lib/libSDL2_mixer.dylib)
set(SDL2_IMAGE_LIBRARY /usr/local/lib/libSDL2_image.dylib)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/TOMB5
    ${CMAKE_CURRENT_SOURCE_DIR}/TOMB5/game
    ${CMAKE_CURRENT_SOURCE_DIR}/TOMB5/global
    ${CMAKE_CURRENT_SOURCE_DIR}/TOMB5/specific
    ${SDL2_INCLUDE_DIR}
    /usr/local/include
)

message(STATUS "SDL2 include dir: ${SDL2_INCLUDE_DIR}")
message(STATUS "SDL2 library: ${SDL2_LIBRARY}")

# Zlib (brew install zlib)
include_directories(/usr/local/opt/zlib/include)
link_directories(/usr/local/opt/zlib/lib)

# Raccogli solo i sorgenti del tuo progetto, evitando la cartella "libs"
file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/TOMB5/game/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TOMB5/global/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TOMB5/specific/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TOMB5/tomb5/*.cpp"
)

# Executable
add_executable(tomb5 ${GAME_SOURCES})

# Link libraries
target_link_libraries(tomb5 PRIVATE
    ${SDL2_LIBRARY}
    ${SDL2_MIXER_LIBRARY}
    ${SDL2_IMAGE_LIBRARY}
    z
)

# macOS frameworks
if(APPLE)
    target_link_libraries(tomb5 PRIVATE
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework AudioToolbox"
        "-framework ForceFeedback"
    )
endif()
