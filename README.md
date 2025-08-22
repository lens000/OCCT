# OCCT Edge to Wire 转换示例

这个项目展示了如何使用OpenCascade Community Technology (OCCT) 将离散的edge转换为首尾相连的wires。

## 概述

在CAD/CAM系统中，经常需要将一系列离散的edge（边）组合成连续的wire（线框）。这个过程包括：

1. 检测edge之间的连接性
2. 按照拓扑关系组织edge
3. 创建有效的wire结构
4. 验证和优化结果

## 文件说明

### 1. `edge_to_wire_example.cpp` - 基础版本
- 简单的edge连接算法
- 基本的wire创建和验证
- 适合学习和理解基本概念

### 2. `advanced_edge_to_wire.cpp` - 高级版本
- 使用深度优先搜索的智能连接算法
- 顶点映射和拓扑分析
- Wire优化和错误修复
- 适合生产环境使用

### 3. `CMakeLists_examples.txt` - 编译配置
- 用于编译示例程序的CMake配置
- 自动查找和链接OCCT库

## 核心功能

### Edge连接检测
```cpp
bool CanEdgesConnect(const TopoDS_Edge& edge1, const TopoDS_Edge& edge2, double tolerance = 1e-6)
```
检查两个edge是否可以通过顶点重合来连接。

### Wire创建
```cpp
std::vector<TopoDS_Wire> ConvertEdgesToWires(const std::vector<TopoDS_Edge>& edges, double tolerance = 1e-6)
```
将离散的edge集合转换为连接的wire集合。

### 拓扑验证
```cpp
bool IsWireValid(const TopoDS_Wire& wire)
```
使用OCCT的拓扑分析器验证wire的有效性。

## 编译方法

### 方法1：使用CMake（推荐）
```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake -f ../CMakeLists_examples.txt ..

# 编译
make

# 运行示例
./edge_to_wire_example
./advanced_edge_to_wire
```

### 方法2：直接编译
```bash
# 基础版本
g++ -std=c++17 -I/path/to/occt/include edge_to_wire_example.cpp -L/path/to/occt/lib -lTKernel -lTKMath -lTKGeomBase -lTKGeomAlgo -lTKTopAlgo -lTKBRep -lTKTopTest -lTKMathTest -o edge_to_wire_example

# 高级版本
g++ -std=c++17 -I/path/to/occt/include advanced_edge_to_wire.cpp -L/path/to/occt/lib -lTKKernel -lTKMath -lTKGeomBase -lTKGeomAlgo -lTKTopAlgo -lTKBRep -lTKTopTest -lTKMathTest -o advanced_edge_to_wire
```

## 使用示例

### 基本用法
```cpp
#include <BRepBuilderAPI_MakeWire.hxx>
#include <TopoDS_Edge.hxx>

// 创建edge集合
std::vector<TopoDS_Edge> edges = CreateSampleEdges();

// 转换为wires
std::vector<TopoDS_Wire> wires = ConvertEdgesToWires(edges);

// 验证结果
for (const auto& wire : wires) {
    if (IsWireValid(wire)) {
        std::cout << "Valid wire created" << std::endl;
    }
}
```

### 高级用法
```cpp
// 使用高级转换器
AdvancedEdgeToWireConverter converter(1e-6);

// 添加edges
for (const auto& edge : edges) {
    converter.AddEdge(edge);
}

// 转换并优化
std::vector<TopoDS_Wire> wires = converter.ConvertToWires();
std::vector<TopoDS_Wire> optimizedWires = converter.OptimizeWires(wires);
```

## 算法原理

### 1. 顶点重合检测
使用几何距离计算来检测两个顶点是否重合：
```cpp
bool AreVerticesCoincident(const TopoDS_Vertex& v1, const TopoDS_Vertex& v2, double tolerance = 1e-6)
{
    gp_Pnt p1 = BRep_Tool::Pnt(v1);
    gp_Pnt p2 = BRep_Tool::Pnt(v2);
    return p1.Distance(p2) < tolerance;
}
```

### 2. Edge连接性检查
检查两个edge是否可以通过顶点重合来连接：
```cpp
bool CanEdgesConnect(const TopoDS_Edge& edge1, const TopoDS_Edge& edge2, double tolerance = 1e-6)
{
    // 检查所有可能的顶点组合
    // 返回true如果找到重合的顶点
}
```

### 3. Wire构建
使用OCCT的`BRepBuilderAPI_MakeWire`类来构建wire：
```cpp
BRepBuilderAPI_MakeWire wireMaker;
wireMaker.Add(edge1);
wireMaker.Add(edge2);
// ... 添加更多edges

if (wireMaker.IsDone()) {
    TopoDS_Wire wire = wireMaker.Wire();
}
```

## 注意事项

1. **容差设置**：根据模型的精度要求设置合适的容差值
2. **拓扑验证**：始终验证生成的wire的有效性
3. **错误处理**：处理OCCT可能抛出的异常
4. **内存管理**：OCCT使用智能指针，注意避免循环引用

## 扩展功能

### 1. 添加更多几何类型
- 支持样条曲线
- 支持椭圆和双曲线
- 支持3D空间曲线

### 2. 改进连接算法
- 使用图论算法优化连接顺序
- 支持分支和循环结构
- 添加方向性检查

### 3. 增强验证功能
- 检查wire的几何连续性
- 验证法向量一致性
- 检测自相交

## 故障排除

### 常见问题

1. **编译错误**：确保OCCT库路径正确设置
2. **运行时错误**：检查edge的几何有效性
3. **连接失败**：调整容差值或检查几何精度

### 调试技巧

1. 使用OCCT的调试工具
2. 检查顶点坐标和edge方向
3. 验证几何曲线的参数范围

## 参考资料

- [OCCT官方文档](https://dev.opencascade.org/)
- [OCCT几何建模指南](https://dev.opencascade.org/doc/overview/html/occt_user_guides__modeling_algos.html)
- [OCCT拓扑数据结构](https://dev.opencascade.org/doc/overview/html/occt_user_guides__foundation_classes.html)

## 许可证

本项目遵循OCCT的LGPL许可证。