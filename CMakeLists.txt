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

# ---------------------------------------------------------------------------
# raylib discovery
#
# Tried in order:
#   1. find_package(raylib CONFIG)  - raylib >= 5 installed via CMake
#   2. pkg-config                   - distro packages / Homebrew
#   3. manual header + library lookup
#   4. -DPACMAN_USE_BUNDLED_RAYLIB=ON to compile a pinned copy from source
# ---------------------------------------------------------------------------

set(PACMAN_USE_BUNDLED_RAYLIB OFF CACHE BOOL
    "Build the bundled raylib sources instead of using an installed raylib")
set(RAYLIB_GIT_TAG "5.5" CACHE STRING "raylib tag used when building from source")

if(PACMAN_USE_BUNDLED_RAYLIB)
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
    if(raylib_FOUND OR TARGET raylib)
        message(STATUS "raylib: using CMake package config")
        set(PACMAN_RAYLIB_TARGET raylib)
    else()
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(RAYLIB_PC QUIET IMPORTED_TARGET raylib)
        endif()
        if(TARGET PkgConfig::RAYLIB_PC)
            message(STATUS "raylib: using pkg-config (${RAYLIB_PC_VERSION})")
            set(PACMAN_RAYLIB_TARGET PkgConfig::RAYLIB_PC)
        else()
            find_path(RAYLIB_INCLUDE_DIR
                NAMES raylib.h
                HINTS ENV RAYLIB_INSTALL_PATH
                PATH_SUFFIXES include)
            find_library(RAYLIB_LIBRARY
                NAMES raylib libraylib
                HINTS ENV RAYLIB_INSTALL_PATH
                PATH_SUFFIXES lib lib64)
            if(NOT RAYLIB_INCLUDE_DIR OR NOT RAYLIB_LIBRARY)
                message(FATAL_ERROR
                    "raylib was not found.\n"
                    "Install it (Arch: 'pacman -S raylib', Debian/Ubuntu: 'apt install libraylib-dev',\n"
                    "macOS: 'brew install raylib', Windows: 'vcpkg install raylib'), or configure with\n"
                    "  -DPACMAN_USE_BUNDLED_RAYLIB=ON\n"
                    "to compile raylib ${RAYLIB_GIT_TAG} from source automatically.")
            endif()
            message(STATUS "raylib: ${RAYLIB_LIBRARY}")
            add_library(pacman_raylib INTERFACE)
            target_include_directories(pacman_raylib INTERFACE ${RAYLIB_INCLUDE_DIR})
            target_link_libraries(pacman_raylib INTERFACE ${RAYLIB_LIBRARY})
            set(PACMAN_RAYLIB_TARGET pacman_raylib)
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
# produces a self contained folder: dist/Pacman(.exe) + dist/assets/...
# ---------------------------------------------------------------------------

install(TARGETS Pacman
    RUNTIME DESTINATION bin
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