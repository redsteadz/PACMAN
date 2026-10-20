cmake_minimum_required(VERSION 3.16)

project(PACMAN
    VERSION 1.0.0
    DESCRIPTION "Pac-Man inspired arcade game built with raylib"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Build configuration
# ---------------------------------------------------------------------------

# Release by default: the CI builds and the published downloads want an
# optimised binary, but keep Debug available via -DCMAKE_BUILD_TYPE=Debug.
get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _is_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)   # the sources rely on GNU extensions, e.g. bits/stdc++.h

option(PACMAN_WERROR "Treat compiler warnings as errors" OFF)

# Put the binary next to the build directory root so that the relative
# "./assets/..." paths used by the game resolve from the run directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# raylib discovery
#
# Order of preference:
#   1. -DRAYLIB_INCLUDE_DIR=... -DRAYLIB_LIBRARY=...   explicit and most reliable
#   2. -DPACMAN_USE_BUNDLED_RAYLIB=ON                   compile a pinned copy
#   3. find_package(raylib CONFIG)                      raylib 5+ CMake install
#   4. pkg-config                                       distro packages, Homebrew
#   5. find_path + find_library                         manual fallback
#
# pkg-config is deliberately skipped while cross-compiling: it resolves .pc
# files against the build host and happily hands the linker a bare "-lraylib"
# with no matching -L, which fails at link time with a confusing error.
# ---------------------------------------------------------------------------

# When raylib is consumed as a bare library file rather than as a CMake target,
# its usage requirements do not come along for the ride. raylib's own build
# lists ${OPENGL_LIBRARIES} and winmm on Windows for exactly this reason:
# rcore calls timeBeginPeriod()/timeEndPeriod() from the WinMM API.
function(pacman_add_raylib_system_libs target)
    if(WIN32)
        target_link_libraries(${target} INTERFACE opengl32 winmm)
    endif()
endfunction()

set(RAYLIB_INCLUDE_DIR "" CACHE PATH "Explicit path to raylib.h")
set(RAYLIB_LIBRARY    "" CACHE FILEPATH "Explicit path to the raylib library")

if(RAYLIB_INCLUDE_DIR AND RAYLIB_LIBRARY AND EXISTS "${RAYLIB_LIBRARY}")
    message(STATUS "raylib: using explicit paths")
    message(STATUS "  include: ${RAYLIB_INCLUDE_DIR}")
    message(STATUS "  library: ${RAYLIB_LIBRARY}")
    add_library(pacman_raylib INTERFACE)
    target_include_directories(pacman_raylib SYSTEM INTERFACE "${RAYLIB_INCLUDE_DIR}")
    target_link_libraries(pacman_raylib INTERFACE "${RAYLIB_LIBRARY}")
    pacman_add_raylib_system_libs(pacman_raylib)
    set(PACMAN_RAYLIB_TARGET pacman_raylib)
elseif(PACMAN_USE_BUNDLED_RAYLIB)
    include(FetchContent)
    FetchContent_Declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        ${RAYLIB_GIT_TAG}
        GIT_SHALLOW    TRUE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(raylib)
    add_library(pacman_raylib INTERFACE)
    target_link_libraries(pacman_raylib INTERFACE raylib)
    set(PACMAN_RAYLIB_TARGET pacman_raylib)
else()
    find_package(raylib 5.0 QUIET CONFIG)
    if(TARGET raylib)
        message(STATUS "raylib: using CMake package config")
        set(PACMAN_RAYLIB_TARGET raylib)
    else()
        set(_raylib_found FALSE)
        if(NOT CMAKE_CROSSCOMPILING)
            find_package(PkgConfig QUIET)
            if(PkgConfig_FOUND)
                pkg_check_modules(RAYLIB_PC QUIET IMPORTED_TARGET raylib)
            endif()
            if(TARGET PkgConfig::RAYLIB_PC)
                message(STATUS "raylib: using pkg-config (${RAYLIB_PC_VERSION})")
                set(PACMAN_RAYLIB_TARGET PkgConfig::RAYLIB_PC)
                set(_raylib_found TRUE)
            endif()
        endif()
        if(NOT _raylib_found)
            find_path(RAYLIB_INCLUDE_DIR_FOUND
                NAMES raylib.h
                HINTS ENV RAYLIB_INSTALL_PATH
                PATH_SUFFIXES include)
            find_library(RAYLIB_LIBRARY_FOUND
                NAMES raylib libraylib
                HINTS ENV RAYLIB_INSTALL_PATH
                PATH_SUFFIXES lib lib64)
            if(RAYLIB_INCLUDE_DIR_FOUND AND RAYLIB_LIBRARY_FOUND)
                message(STATUS "raylib: ${RAYLIB_LIBRARY_FOUND}")
                add_library(pacman_raylib INTERFACE)
                target_include_directories(pacman_raylib SYSTEM INTERFACE "${RAYLIB_INCLUDE_DIR_FOUND}")
                target_link_libraries(pacman_raylib INTERFACE "${RAYLIB_LIBRARY_FOUND}")
                pacman_add_raylib_system_libs(pacman_raylib)
                set(PACMAN_RAYLIB_TARGET pacman_raylib)
            else()
                message(FATAL_ERROR
                    "raylib was not found.\n\n"
                    "Install it:\n"
                    "  Arch / CachyOS : sudo pacman -S raylib\n"
                    "  Debian / Ubuntu : sudo apt install libraylib-dev\n"
                    "  macOS          : brew install raylib\n\n"
                    "Or point CMake straight at an existing install:\n"
                    "  -DRAYLIB_INCLUDE_DIR=/path/to/include -DRAYLIB_LIBRARY=/path/to/libraylib.a\n\n"
                    "Or let CMake build a pinned copy for you:\n"
                    "  -DPACMAN_USE_BUNDLED_RAYLIB=ON")
            endif()
        endif()
    endif()
endif()
# ---------------------------------------------------------------------------
# Game
# ---------------------------------------------------------------------------

add_executable(Pacman WIN32 main.cpp)
target_link_libraries(Pacman PRIVATE ${PACMAN_RAYLIB_TARGET})
target_compile_definitions(Pacman PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

if(MSVC)
    target_compile_options(Pacman PRIVATE /W3 /utf-8)
    if(PACMAN_WERROR)
        target_compile_options(Pacman PRIVATE /WX)
    endif()
else()
    target_compile_options(Pacman PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers)
    if(PACMAN_WERROR)
        target_compile_options(Pacman PRIVATE -Werror)
    endif()
endif()

# MinGW: link the GCC runtime statically so a published bundle only needs the
# .exe plus assets/. With PACMAN_STATIC_RUNTIME the whole binary is static.
option(PACMAN_STATIC_RUNTIME
    "Produce a fully static, self-contained executable" OFF)
if(MINGW)
    if(PACMAN_STATIC_RUNTIME)
        target_link_options(Pacman PRIVATE -static)
    else()
        target_link_options(Pacman PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()

# ---------------------------------------------------------------------------
# Assets
#
# Assets are staged next to the executable *after* a build so that running
# `./bin/Pacman` from the build tree works, and so that the copy can be
# refreshed simply by rebuilding.
# ---------------------------------------------------------------------------

set(PACMAN_ASSET_STAGE "${CMAKE_BINARY_DIR}/bin/assets")

add_custom_command(TARGET Pacman POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${PACMAN_ASSET_STAGE}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets" "${PACMAN_ASSET_STAGE}"
    COMMENT "Staging PACMAN assets into ${PACMAN_ASSET_STAGE}"
    VERBATIM)

# ---------------------------------------------------------------------------
# Install / portable bundle
#
#   cmake --install build --prefix dist
#
# produces a self contained folder:
#
#   dist/Pacman(.exe)
#   dist/assets/...
#
# The executable deliberately sits at the archive root rather than in a bin/
# subfolder: the game loads "./assets/..." relative to its working directory, so
# keeping the two as siblings is what makes the bundle runnable straight after
# unzipping with no arguments.
# ---------------------------------------------------------------------------

install(TARGETS Pacman
    RUNTIME DESTINATION .
    BUNDLE  DESTINATION .)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/"
    DESTINATION assets
    USE_SOURCE_PERMISSIONS)

if(WIN32)
    # Ship the MinGW runtime next to the executable so the bundle runs as-is.
    foreach(_dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
        find_file(PACMAN_MINGW_DLL_${_dll} NAMES ${_dll}
                  HINTS ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES})
        if(PACMAN_MINGW_DLL_${_dll})
            install(FILES "${PACMAN_MINGW_DLL_${_dll}}" DESTINATION bin OPTIONAL)
        endif()
    endforeach()
endif()

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------

message(STATUS "")
message(STATUS "  ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  output     : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "")