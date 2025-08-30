cmake_minimum_required(VERSION 3.16)
project(OffsetCurveExtendExample)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找OpenCascade
find_package(OpenCascade REQUIRED)

# 如果OpenCascade没有提供CMake配置文件，手动设置
if(NOT OpenCascade_FOUND)
    # 尝试查找OpenCascade的安装路径
    find_path(OpenCascade_INCLUDE_DIR
        NAMES Standard_Version.hxx
        PATHS
        /usr/include/opencascade
        /usr/local/include/opencascade
        /opt/opencascade/include
        $ENV{OpenCascade_ROOT}/include
    )
    
    find_library(OpenCascade_LIBRARY
        NAMES TKernel TKMath TKGeomBase TKGeomAlgo TKTopAlgo TKBRep TKPrim TKOffset TKBool
        PATHS
        /usr/lib
        /usr/local/lib
        /opt/opencascade/lib
        $ENV{OpenCascade_ROOT}/lib
    )
    
    if(OpenCascade_INCLUDE_DIR AND OpenCascade_LIBRARY)
        set(OpenCascade_FOUND TRUE)
        set(OpenCascade_INCLUDE_DIRS ${OpenCascade_INCLUDE_DIR})
        set(OpenCascade_LIBRARIES ${OpenCascade_LIBRARY})
    endif()
endif()

if(NOT OpenCascade_FOUND)
    message(FATAL_ERROR "OpenCascade not found. Please install OpenCascade or set OpenCascade_ROOT environment variable.")
endif()

# 创建可执行文件
add_executable(offset_curve_extend_example offset_curve_extend_example.cpp)

# 包含头文件目录
target_include_directories(offset_curve_extend_example PRIVATE ${OpenCascade_INCLUDE_DIRS})

# 链接库
target_link_libraries(offset_curve_extend_example ${OpenCascade_LIBRARIES})

# 设置编译选项
if(MSVC)
    target_compile_options(offset_curve_extend_example PRIVATE /bigobj)
endif()

# 打印OpenCascade信息
message(STATUS "OpenCascade include dirs: ${OpenCascade_INCLUDE_DIRS}")
message(STATUS "OpenCascade libraries: ${OpenCascade_LIBRARIES}")

# 安装目标
install(TARGETS offset_curve_extend_example
    RUNTIME DESTINATION bin
)
