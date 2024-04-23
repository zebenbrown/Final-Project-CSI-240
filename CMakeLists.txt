cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# policies
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# create project
project(MyProject)

# set where the compiled files goes
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/libs)

# emscripten settings
IF(DEFINED EMSCRIPTEN)
    message(STATUS "emscripten build")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Oz -s WASM=2 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s ASYNCIFY -s FULL_ES2=1 -s USE_GLFW=3")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Oz -s WASM=2 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s ASYNCIFY -s FULL_ES2=1 -s USE_GLFW=3")

    file(GLOB HTML "${CMAKE_CURRENT_SOURCE_DIR}/html/*")
    file(COPY ${HTML} DESTINATION ${CMAKE_BINARY_DIR}/bin/)
ENDIF()

# add dependencies
include(external/compilerchecks.cmake)
include(external/cpm.cmake)

# add libraries
include(external/glew.cmake)
include(external/sdl.cmake)
include(external/imgui.cmake)
include(external/cpr.cmake)
CPMAddPackage("gh:nlohmann/json@3.10.5")

# add executable
add_executable(demo main.cpp rosters.cpp)

find_package(OpenGL REQUIRED)

    target_link_libraries(demo  PUBLIC SDL2-static SDL2_image IMGUI)
    target_link_libraries(demo PRIVATE cpr::cpr)
    target_link_libraries(demo PUBLIC SDL2-static SDL2_image IMGUI nlohmann_json::nlohmann_json)
