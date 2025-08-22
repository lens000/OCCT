# OCCT边转换为Wire示例 / OCCT Edge to Wire Examples

本示例演示如何使用Open CASCADE Technology (OCCT)将边(Edge)转换成首尾相连的线框(Wire)。

This example demonstrates how to convert edges into connected wires using Open CASCADE Technology (OCCT).

## 文件说明 / File Description

1. **`simple_wire_example.cpp`** - 简单示例，演示基本的边到Wire转换
2. **`wire_from_edges_example.cpp`** - 完整示例，演示多种Wire创建方法
3. **`CMakeLists_wire_example.txt`** - CMake构建配置文件

## 核心概念 / Core Concepts

### Wire是什么？/ What is a Wire?
在OCCT中，Wire是一个由一个或多个首尾相连的边组成的拓扑实体。Wire可以是开放的或闭合的。

In OCCT, a Wire is a topological entity composed of one or more connected edges. A wire can be open or closed.

### 关键要求 / Key Requirements
- **连接性**: 边必须首尾相连，即相邻边必须共享顶点
- **Connectivity**: Edges must be connected end-to-end, sharing vertices between adjacent edges

## 主要方法 / Main Methods

### 1. 使用构造函数 / Using Constructor
```cpp
TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge1, edge2);
```

### 2. 逐步添加边 / Adding Edges Step by Step
```cpp
BRepBuilderAPI_MakeWire wireBuilder;
wireBuilder.Add(edge1);
wireBuilder.Add(edge2);
if (wireBuilder.IsDone()) {
    TopoDS_Wire wire = wireBuilder.Wire();
}
```

### 3. 从边列表创建 / Creating from Edge List
```cpp
TopTools_ListOfShape edgeList;
edgeList.Append(edge1);
edgeList.Append(edge2);

BRepBuilderAPI_MakeWire wireBuilder;
wireBuilder.Add(edgeList);
```

## 错误处理 / Error Handling

检查`wireBuilder.IsDone()`状态，如果失败则通过`wireBuilder.Error()`获取错误类型：

Check `wireBuilder.IsDone()` status, if failed get error type via `wireBuilder.Error()`:

- `BRepBuilderAPI_EmptyWire` - 空Wire
- `BRepBuilderAPI_DisconnectedWire` - 边未连接
- `BRepBuilderAPI_NonManifoldWire` - 非流形Wire

## 编译方法 / Compilation Methods

### 方法1: 使用CMake / Method 1: Using CMake
```bash
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/occt
make
```

### 方法2: 直接编译 / Method 2: Direct Compilation
```bash
g++ -std=c++11 simple_wire_example.cpp \
    -lTKernel -lTKMath -lTKBRep -lTKTopAlgo -lTKG3d -lTKGeomBase \
    -o simple_wire_example
```

## 运行示例 / Running Examples

```bash
./simple_wire_example
```

## 最佳实践 / Best Practices

1. **检查连接性** - 确保边的顶点正确连接
2. **错误处理** - 始终检查`IsDone()`状态
3. **构建3D曲线** - 调用`BRepLib::BuildCurves3d(wire)`
4. **验证结果** - 检查Wire是否为空(`wire.IsNull()`)

1. **Check connectivity** - Ensure edge vertices are properly connected
2. **Error handling** - Always check `IsDone()` status  
3. **Build 3D curves** - Call `BRepLib::BuildCurves3d(wire)`
4. **Validate result** - Check if wire is null (`wire.IsNull()`)

## 常见问题 / Common Issues

### Q: 边无法连接成Wire？
**A**: 检查相邻边是否共享顶点。顶点必须几何重合（在容差范围内）。

### Q: Edges cannot be connected into Wire?
**A**: Check if adjacent edges share vertices. Vertices must be geometrically coincident (within tolerance).

### Q: Wire创建成功但显示异常？
**A**: 调用`BRepLib::BuildCurves3d(wire)`构建3D曲线表示。

### Q: Wire created successfully but displays abnormally?
**A**: Call `BRepLib::BuildCurves3d(wire)` to build 3D curve representation.