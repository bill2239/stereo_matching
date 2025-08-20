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

# Enable Qt's automoc for signals/slots
set(CMAKE_AUTOMOC ON)

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
