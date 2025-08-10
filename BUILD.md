# 构建说明

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
