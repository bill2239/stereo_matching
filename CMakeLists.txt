cmake_minimum_required(VERSION 3.16)
project(stereo_matching LANGUAGES CXX)

# Use C++17 (needed for clamp etc.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Find OpenCV
find_package(OpenCV REQUIRED)

# Source files
set(SOURCES
    ssd_stereo.h
    stereo_matching.cpp
)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-mpopcnt" COMPILER_HAS_POPCNT)
if(COMPILER_HAS_POPCNT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mpopcnt")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fopenmp")

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})
# Include dirs
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${OpenCV_LIBS}
)

# On Linux, make sure RPATH works
if(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
endif()
