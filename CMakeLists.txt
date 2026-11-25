cmake_minimum_required(VERSION 3.21)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug") # "Debug" or "Release"
endif()

project(simple-triangle-opengl-sdl3-ubuntu-c LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# =============================================================================
# Paths (All library versions visible in one place)
# =============================================================================
set(LIBS_ROOT "$ENV{HOME}/libs")
set(SDL3_PATH "${LIBS_ROOT}/sdl3-3.4.20-ubuntu")
set(GLAD_PATH "${LIBS_ROOT}/glad-0.1.36/opengl-3.3")

# =============================================================================
# Target
# =============================================================================
set(APP_TARGET app)

set(PROJECT_SOURCES
    src/main.c
)

add_executable(${APP_TARGET} ${PROJECT_SOURCES})

target_include_directories(${APP_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =============================================================================
# Packages & Libraries
# =============================================================================

# --- SDL3 (Official Config -> handles X11/Wayland/pthread/dl/m/...) -----------
find_package(SDL3 REQUIRED CONFIG
    PATHS "${SDL3_PATH}/lib/cmake/SDL3"
    NO_DEFAULT_PATH
    NO_CMAKE_FIND_ROOT_PATH
)

if(TARGET SDL3::SDL3-static)
    set(SDL3_TARGET SDL3::SDL3-static)
else()
    set(SDL3_TARGET SDL3::SDL3)
endif()

# --- OpenGL ------------------------------------------------------------------
find_package(OpenGL REQUIRED)

# --- GLAD (OpenGL 3.3 Core loader) -------------------------------------------
add_library(glad STATIC
    "${GLAD_PATH}/src/glad.c"
)
target_include_directories(glad PUBLIC
    "${GLAD_PATH}/include"
)

# =============================================================================
# Link Libraries, Flags & Stripping
# =============================================================================
target_link_libraries(${APP_TARGET} PRIVATE
    ${SDL3_TARGET}
    glad
    OpenGL::GL
)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${APP_TARGET} PRIVATE -Os)
    target_link_options(${APP_TARGET} PRIVATE -s)
endif()
