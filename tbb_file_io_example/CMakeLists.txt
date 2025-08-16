cmake_minimum_required(VERSION 3.16)
project(TBBFileIOExample)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找oneTBB包
find_package(TBB REQUIRED)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# 包含目录
include_directories(${TBB_INCLUDE_DIRS})

# 创建可执行文件 - TBB文件I/O示例
add_executable(tbb_file_io_example tbb_file_io_example.cpp)

# 创建可执行文件 - 性能测试
add_executable(performance_test performance_test.cpp)

# 创建可执行文件 - 简单TBB测试
add_executable(simple_tbb_test simple_tbb_test.cpp)

# 链接oneTBB库
target_link_libraries(tbb_file_io_example TBB::tbb)
target_link_libraries(performance_test TBB::tbb)
target_link_libraries(simple_tbb_test TBB::tbb)

# 设置运行时库路径
set_target_properties(tbb_file_io_example PROPERTIES
    LINK_FLAGS "-Wl,-rpath,${TBB_LIBRARY_DIRS}"
)
set_target_properties(performance_test PROPERTIES
    LINK_FLAGS "-Wl,-rpath,${TBB_LIBRARY_DIRS}"
)
set_target_properties(simple_tbb_test PROPERTIES
    LINK_FLAGS "-Wl,-rpath,${TBB_LIBRARY_DIRS}"
)
