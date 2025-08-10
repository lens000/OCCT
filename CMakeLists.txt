cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(AdvancingFrontMesh VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译器标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 在Debug模式下添加调试信息
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")

# 添加数学库
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lm")
endif()

# 源文件
set(SOURCES
    AdvancingFront.cpp
    main.cpp
)

# 头文件
set(HEADERS
    Point2D.h
    Edge.h
    Triangle.h
    AdvancingFront.h
)

# 创建可执行文件
add_executable(advancing_front_demo ${SOURCES} ${HEADERS})

# 链接数学库（在Linux上需要）
if(UNIX)
    target_link_libraries(advancing_front_demo m)
endif()

# 设置输出目录
set_target_properties(advancing_front_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 安装规则（可选）
install(TARGETS advancing_front_demo
    RUNTIME DESTINATION bin
)

# 创建库版本（可选）
add_library(advancing_front_lib STATIC
    AdvancingFront.cpp
)

target_include_directories(advancing_front_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置库的输出目录
set_target_properties(advancing_front_lib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# 安装库和头文件（可选）
install(TARGETS advancing_front_lib
    ARCHIVE DESTINATION lib
)

install(FILES ${HEADERS}
    DESTINATION include
)

# 打印配置信息
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# 创建构建说明
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/BUILD.md)
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/BUILD.md
"# 构建说明

## 编译方法

### 使用 CMake (推荐)
```bash
mkdir build
cd build
cmake ..
make
```

### 直接使用 g++
```bash
g++ -std=c++11 -O2 -Wall -o advancing_front_demo main.cpp AdvancingFront.cpp -lm
```

## 运行
```bash
./bin/advancing_front_demo
```

生成的网格文件将保存在运行目录中。
")
endif()
