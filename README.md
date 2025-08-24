# 波前法网格生成器 (Wavefront Mesh Generator)

这是一个基于波前法（Advancing Front Method）的二维网格生成器，使用C++实现。该实现参考了Open CASCADE Technology (OCCT)中的波前法算法，提供了完整的网格生成功能。

## 功能特性

- **波前法算法**: 实现了完整的波前推进网格生成算法
- **多种几何形状**: 支持圆形、矩形、L形等复杂几何体的网格生成
- **网格优化**: 包含网格质量检查和Laplacian平滑优化
- **PLY导出**: 支持将生成的网格导出为PLY格式，便于可视化
- **C++17标准**: 使用现代C++特性，代码清晰易读

## 算法原理

波前法的基本思想是从已知的边界开始，逐步向内部推进，生成新的网格单元：

1. **边界初始化**: 从几何体的边界开始，建立初始的"波前"
2. **波前推进**: 沿着波前边界逐步生成新的网格单元
3. **质量优化**: 确保生成的网格满足质量要求
4. **约束处理**: 处理内部约束和边界条件

## 项目结构

```
.
├── WavefrontMeshGenerator.h    # 头文件
├── WavefrontMeshGenerator.cpp  # 实现文件
├── main.cpp                    # 主程序
├── CMakeLists.txt             # CMake构建文件
└── README.md                  # 项目说明
```

## 编译要求

- **编译器**: 支持C++17的编译器（GCC 7+, Clang 5+, MSVC 2017+）
- **CMake**: 3.10或更高版本
- **操作系统**: Windows, Linux, macOS

## 编译步骤

### 1. 克隆项目
```bash
git clone <repository-url>
cd WavefrontMeshGenerator
```

### 2. 创建构建目录
```bash
mkdir build
cd build
```

### 3. 配置和编译
```bash
cmake ..
make
```

### 4. 运行程序
```bash
./WavefrontMeshGenerator
```

## 使用方法

### 基本用法

```cpp
#include "WavefrontMeshGenerator.h"

// 创建网格生成器
WavefrontMeshGenerator meshGenerator;

// 设置边界点
std::vector<Point2D> boundaryPoints = {
    Point2D(0, 0), Point2D(1, 0), Point2D(1, 1), Point2D(0, 1)
};
meshGenerator.setBoundaryPoints(boundaryPoints);

// 添加内部点（可选）
std::vector<Point2D> internalPoints = {
    Point2D(0.5, 0.5)
};
meshGenerator.addInternalPoints(internalPoints);

// 生成网格
if (meshGenerator.generateMesh()) {
    // 打印统计信息
    meshGenerator.printMeshStatistics();
    
    // 导出为PLY文件
    meshGenerator.exportToPLY("output.ply");
}
```

### 预定义几何形状

程序提供了多种预定义的几何形状生成函数：

- `generateCircleBoundary()`: 生成圆形边界
- `generateRectangleBoundary()`: 生成矩形边界
- `generateLShapeBoundary()`: 生成L形边界
- `generateInternalPoints()`: 生成内部点

## 输出格式

生成的网格可以导出为PLY格式，这是一种常用的3D文件格式，可以被以下软件读取：

- **MeshLab**: 开源的3D网格处理软件
- **Blender**: 功能强大的3D建模软件
- **ParaView**: 科学可视化软件
- **其他支持PLY格式的软件**

## 算法参数

可以通过修改头文件中的常量来调整算法行为：

```cpp
static constexpr double EPSILON = 1e-10;        // 数值精度
static constexpr double MIN_ANGLE = 0.1;        // 最小角度（弧度）
static constexpr double MIN_AREA = 1e-8;        // 最小面积
```

## 性能优化

该实现包含多种优化策略：

1. **边界框优化**: 使用边界框进行快速相交检测
2. **角度优化**: 选择最佳角度进行波前推进
3. **内存管理**: 使用增量分配器优化内存使用
4. **连接性缓存**: 维护节点到边和三角形的映射关系

## 扩展性

代码设计具有良好的扩展性：

- **模块化设计**: 各个功能模块清晰分离
- **接口设计**: 清晰的公共接口便于扩展
- **数据结构**: 灵活的数据结构支持复杂场景

## 示例输出

程序会生成多种几何形状的网格，并输出详细的统计信息：

```
=== 网格统计信息 ===
顶点数量: 45
边数量: 67
三角形数量: 23
边界顶点数量: 20
有效三角形数量: 23
总面积: 25.1327
最小三角形面积: 0.0123
最大三角形面积: 1.2345
平均三角形面积: 1.0927
```

## 故障排除

### 常见问题

1. **编译错误**: 确保使用支持C++17的编译器
2. **内存不足**: 对于复杂几何体，可能需要增加系统内存
3. **网格质量差**: 调整`MIN_ANGLE`和`MIN_AREA`参数

### 调试模式

在Debug模式下编译可以获得更详细的输出信息：

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## 贡献

欢迎提交Issue和Pull Request来改进这个项目。

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 参考文献

1. Lo, S. H. (1985). "A new mesh generation scheme for arbitrary planar domains."
2. Peraire, J., Vahdati, M., Morgan, K., & Zienkiewicz, O. C. (1987). "Adaptive remeshing for compressible flow computations."
3. Open CASCADE Technology Documentation: https://dev.opencascade.org/

## 联系方式

如有问题或建议，请通过以下方式联系：

- 提交GitHub Issue
- 发送邮件至项目维护者

---

**注意**: 这是一个教育性质的实现，适用于学习和研究目的。对于生产环境，建议使用成熟的商业或开源网格生成库。