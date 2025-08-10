# 二维前沿推进网格生成算法 (2D Advancing Front Mesh Generation)

本项目实现了一个用于二维平面的前沿推进（Advancing Front Approach）网格生成算法。该算法是一种广泛应用于有限元分析中的三角网格生成技术。

## 算法原理

前沿推进算法是一种增量式网格生成方法，其基本思想是：

1. **初始化**：从给定的边界创建初始前沿边（front edges）
2. **迭代处理**：选择前沿边并在其上生成新的三角形
3. **前沿更新**：更新前沿边列表，移除已处理的边，添加新生成的边
4. **终止条件**：当所有前沿边都被处理完毕时，算法结束

## 项目结构

```
.
├── Point2D.h           # 二维点类
├── Edge.h              # 前沿边类
├── Triangle.h          # 三角形类
├── AdvancingFront.h    # 核心算法类头文件
├── AdvancingFront.cpp  # 核心算法实现
├── main.cpp           # 演示程序
├── CMakeLists.txt     # 构建配置
└── README.md          # 项目说明
```

## 核心类说明

### Point2D
- 表示二维平面上的点
- 提供基本的向量运算（加法、减法、点积、叉积）
- 包含距离计算和单位化操作

### Edge
- 表示前沿边
- 包含两个端点和活跃状态
- 提供边的几何属性计算（长度、中点、方向、法向量）

### Triangle
- 表示生成的三角形
- 提供面积、外心、外接圆等几何计算
- 包含质量评估方法（最小角度等）

### AdvancingFront
- 核心算法实现类
- 管理点集、三角形集合和前沿边队列
- 提供网格生成的主要逻辑
- 支持带洞域的复杂边界管理

## 主要特性

- **自适应网格**：根据设定的网格尺寸生成合适的三角形
- **质量控制**：通过角度阈值控制三角形质量
- **边界适应**：支持任意多边形边界
- **带洞域支持**：支持包含洞的复杂域网格生成
- **几何形状支持**：
  - 矩形
  - 圆形
  - L形
  - 自定义多边形
  - 带洞的复杂域（单个洞或多个洞）

## 编译和运行

### 系统要求
- C++11 或更高版本
- CMake 3.10 或更高版本
- 支持 GCC、Clang 或 MSVC 编译器

### 使用 CMake 编译（推荐）

```bash
mkdir build
cd build
cmake ..
make
```

### 直接使用编译器

```bash
g++ -std=c++11 -O2 -Wall -o advancing_front_demo main.cpp AdvancingFront.cpp -lm
```

### 运行演示

```bash
./bin/advancing_front_demo  # CMake 编译方式
# 或
./advancing_front_demo      # 直接编译方式
```

## 使用示例

### 基本用法（无洞域）

```cpp
#include "AdvancingFront.h"

// 创建算法实例
AdvancingFront af(0.5);  // 网格尺寸为 0.5

// 定义矩形边界
std::vector<Point2D> boundary = {
    Point2D(-2, -1.5),  // 左下
    Point2D(2, -1.5),   // 右下
    Point2D(2, 1.5),    // 右上
    Point2D(-2, 1.5)    // 左上
};

// 设置边界
af.setBoundary(boundary);

// 生成网格
if (af.generateMesh()) {
    // 获取结果
    const auto& points = af.getPoints();
    const auto& triangles = af.getTriangles();
    
    // 输出网格信息
    af.printMeshInfo();
    
    // 导出到文件
    af.exportToFile("mesh.txt");
}
```

### 带洞域的用法

```cpp
#include "AdvancingFront.h"

// 创建算法实例
AdvancingFront af(0.4);

// 定义外边界（逆时针）
std::vector<Point2D> outerBoundary = {
    Point2D(-2, -1.5),  // 左下
    Point2D(2, -1.5),   // 右下
    Point2D(2, 1.5),    // 右上
    Point2D(-2, 1.5)    // 左上
};

// 定义洞（顺时针）
std::vector<Point2D> hole = {
    Point2D(0.5, 0.5),   // 右上
    Point2D(0.5, -0.5),  // 右下
    Point2D(-0.5, -0.5), // 左下
    Point2D(-0.5, 0.5)   // 左上
};

// 设置带洞的域
std::vector<std::vector<Point2D>> holes = {hole};
af.setBoundaryWithHoles(outerBoundary, holes);

// 生成网格
if (af.generateMesh()) {
    af.printMeshInfo();
    af.exportToFile("mesh_with_hole.txt");
}
```

### 动态添加洞

```cpp
AdvancingFront af(0.4);

// 设置外边界
af.setBoundary(outerBoundary);

// 动态添加洞
af.addHole(hole1);
af.addHole(hole2);

// 重新初始化（使用所有边界）
af.setBoundaryWithHoles(outerBoundary, {hole1, hole2});

// 生成网格
af.generateMesh();
```

## 参数调节

### 网格尺寸
```cpp
af.setMeshSize(0.3);  // 更精细的网格
```

### 角度阈值
```cpp
af.setAngleThresholds(0.2, 3.0);  // 最小角度和最大角度
```

### 最大迭代次数
```cpp
af.setMaxIterations(5000);
```

## API 参考

### 边界设置方法

```cpp
// 设置简单边界（无洞）
void setBoundary(const std::vector<Point2D>& boundaryPoints);

// 设置带洞的复杂域
void setBoundaryWithHoles(const std::vector<Point2D>& outerBoundary, 
                          const std::vector<std::vector<Point2D>>& holes);

// 添加单个洞
void addHole(const std::vector<Point2D>& hole);
```

### 重要注意事项

1. **边界方向**：
   - 外边界：必须按逆时针方向排列
   - 洞：必须按顺时针方向排列

2. **点的顺序**：
   - 边界点应该形成封闭的多边形
   - 避免自相交或重复点

3. **洞的位置**：
   - 洞必须完全位于外边界内部
   - 洞之间不能相交或重叠

## 输出格式

生成的网格文件格式：

```
POINTS <点的数量>
<x1> <y1>
<x2> <y2>
...

TRIANGLES <三角形数量>
<点索引1> <点索引2> <点索引3>
<点索引1> <点索引2> <点索引3>
...
```

## 算法性能

- **时间复杂度**：O(n log n)，其中 n 是生成的三角形数量
- **空间复杂度**：O(n)
- **网格质量**：通过角度约束保证三角形质量

## 应用场景

- 有限元分析前处理
- 计算流体力学网格生成
- 计算几何研究
- 图形学三角化应用

## 算法优势

1. **渐进式生成**：逐步生成网格，便于控制和调试
2. **边界适应性强**：能够很好地适应复杂边界形状
3. **质量可控**：通过参数调节控制网格质量
4. **实现相对简单**：相比 Delaunay 三角化更容易理解和实现

## 限制和改进方向

### 当前限制
- 网格密度控制相对简单
- 对于非常尖锐的角度处理可能不够理想
- 洞的数量过多时性能可能下降

### 改进方向
- 更精细的密度函数控制
- 更好的质量优化算法
- 并行化实现
- 自适应网格细化
- 支持约束边

## 参考文献

1. Löhner, R. (1996). *Progress in grid generation via the advancing front technique*
2. Peraire, J., Vahdati, M., Morgan, K., & Zienkiewicz, O. C. (1987). *Adaptive remeshing for compressible flow computations*
3. Lo, S. H. (1985). *A new mesh generation scheme for arbitrary planar domains*

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件。

## 作者

实现了完整的二维前沿推进网格生成算法，包括核心数据结构、算法逻辑和演示程序。

---

如果您有任何问题或建议，欢迎提出 Issue 或 Pull Request！