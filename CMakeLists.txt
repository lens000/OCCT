cmake_minimum_required(VERSION 3.16)
project(PointToCurveDistance)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCASCADE REQUIRED)

if(NOT OpenCASCADE_FOUND)
    set(OpenCASCADE_POSSIBLE_PATHS
        "/usr/local/opencascade"
        "/opt/opencascade"
        "/usr/include/opencascade"
        "$ENV{CASROOT}"
    )
    
    foreach(path ${OpenCASCADE_POSSIBLE_PATHS})
        if(EXISTS "${path}")
            set(OpenCASCADE_INCLUDE_DIR "${path}/include/opencascade")
            set(OpenCASCADE_LIBRARY_DIR "${path}/lib")
            break()
        endif()
    endforeach()
    
    if(NOT OpenCASCADE_INCLUDE_DIR)
        message(FATAL_ERROR "OpenCASCADE not found. Please install OpenCASCADE or set CASROOT environment variable.")
    endif()
endif()

if(OpenCASCADE_INCLUDE_DIR)
    include_directories(${OpenCASCADE_INCLUDE_DIR})
else()
    include_directories(${OpenCASCADE_INCLUDE_DIRS})
endif()

if(OpenCASCADE_LIBRARY_DIR)
    link_directories(${OpenCASCADE_LIBRARY_DIR})
endif()

add_executable(point_to_curve_distance point_to_curve_distance.cpp)

set(OPENCASCADE_LIBS
    TKernel
    TKMath
    TKG2d
    TKG3d
    TKGeomBase
    TKGeomAlgo
    TKBRep
    TKTopAlgo
)

if(OpenCASCADE_LIBRARIES)
    target_link_libraries(point_to_curve_distance ${OpenCASCADE_LIBRARIES})
else()
    target_link_libraries(point_to_curve_distance ${OPENCASCADE_LIBS})
endif()

target_compile_definitions(point_to_curve_distance PRIVATE
    $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
)

set_target_properties(point_to_curve_distance PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "OpenCASCADE found: ${OpenCASCADE_FOUND}")
if(OpenCASCADE_INCLUDE_DIR)
    message(STATUS "OpenCASCADE include directory: ${OpenCASCADE_INCLUDE_DIR}")
endif()