cmake_minimum_required(VERSION 3.10)
project(WavefrontMeshGenerator)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
endif()

# 查找必要的包
find_package(PackageHandleStandardArgs REQUIRED)

# 源文件
set(SOURCES
    WavefrontMeshGenerator.cpp
    main.cpp
)

# 头文件
set(HEADERS
    WavefrontMeshGenerator.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 链接库
if(UNIX)
    target_link_libraries(${PROJECT_NAME} m)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 输出信息
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++编译器: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")

# 测试配置（可选）
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
