cmake_minimum_required(VERSION 3.10)
project(RecordCalibratedCam CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Build record_cam
add_executable(record_cam srcs/record_cam.cpp)
target_link_libraries(record_cam ${OpenCV_LIBS})

# Copy calibration.yml to build directory after build
add_custom_command(TARGET record_cam POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/config/calibration.yml"
    "${CMAKE_CURRENT_BINARY_DIR}/calibration.yml"
)
