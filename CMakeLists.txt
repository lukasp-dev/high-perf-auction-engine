cmake_minimum_required(VERSION 3.15)
project(DigitalArtAuctionEngine LANGUAGES CXX)

# [Lukas Standard] Enforce C++20 for modern syntax and M4 Pro performance
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags for Apple Silicon (M4 Pro)
if(APPLE)
    add_compile_options(
        -Wall -Wextra          # Enable warnings for safer code
        -O3                    # Maximum optimization level
        -mcpu=native           # Target specific M4 Pro architecture features
        -ffast-math            # Speed up floating-point calculations for auction pricing
        -flto                  # Link Time Optimization (Global optimization)
    )
    message(STATUS "Configuring for Apple Silicon (M4 Pro Targeted)")
endif()

# Include current directory for header files
include_directories(.)

# Main executable setup
add_executable(auction_bench main.cpp)

# Build summary output
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")