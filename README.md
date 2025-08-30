# OpenCascade曲线偏移延伸示例

这个项目演示了如何在OpenCascade中实现曲线偏移时的延伸功能。

## 功能特性

本示例提供了多种方法来实现曲线偏移时的延伸：

### 1. 使用GeomLib::ExtendCurveToPoint延伸曲线
- 支持G1、G2、G3连续性
- 可以向前或向后延伸
- 适用于有界曲线

### 2. 创建偏移曲线并延伸
- 先创建偏移曲线
- 然后使用切线方向计算延伸点
- 自动处理延伸逻辑

### 3. 拓扑偏移（Wire）处理
- 使用BRepOffsetAPI_MakeOffset进行拓扑偏移
- 支持复杂的Wire结构
- 为后续延伸操作提供基础

### 4. 自定义偏移延伸算法
- 结合偏移和延伸的完整流程
- 转换为B样条曲线以获得更好的控制
- 支持参数化延伸

## 编译要求

- OpenCascade 7.0.0 或更高版本
- C++17 兼容的编译器
- CMake 3.16 或更高版本

## 编译步骤

### Linux/macOS
```bash
mkdir build
cd build
cmake ..
make
```

### Windows (Visual Studio)
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

## 使用方法

编译完成后，运行可执行文件：

```bash
./offset_curve_extend_example
```

程序将演示：
1. 创建基础圆曲线
2. 使用GeomLib::ExtendCurveToPoint延伸曲线
3. 创建偏移曲线并延伸
4. 创建自定义偏移延伸
5. 创建拓扑偏移

## 关键API说明

### GeomLib::ExtendCurveToPoint
```cpp
void GeomLib::ExtendCurveToPoint(
    Handle(Geom_BoundedCurve)& Curve,
    const gp_Pnt& Point,
    const Standard_Integer Continuity,  // 1=G1, 2=G2, 3=G3
    const Standard_Boolean After       // true=向后延伸, false=向前延伸
);
```

### Geom_OffsetCurve
```cpp
Handle(Geom_OffsetCurve) anOffsetCurve = new Geom_OffsetCurve(
    theBaseCurve,    // 基础曲线
    theOffset,       // 偏移距离
    theDirection     // 偏移方向
);
```

### BRepOffsetAPI_MakeOffset
```cpp
BRepOffsetAPI_MakeOffset anAlgo(theWire);
anAlgo.Perform(offsetDistance, altitude);
TopoDS_Shape result = anAlgo.Shape();
```

## 注意事项

1. **曲线类型限制**：GeomLib::ExtendCurveToPoint只适用于有界曲线
2. **连续性要求**：延伸操作需要曲线在端点处有足够的导数信息
3. **偏移方向**：偏移曲线的方向必须与基础曲线的切线不平行
4. **拓扑复杂性**：对于复杂的Wire结构，延伸操作可能需要额外的处理逻辑

## 扩展应用

这个示例可以扩展用于：
- 机械设计中的轮廓偏移
- 建筑CAD中的墙体偏移
- 路径规划中的安全边界计算
- 3D建模中的表面偏移

## 故障排除

### 常见问题

1. **编译错误：找不到OpenCascade头文件**
   - 确保OpenCascade已正确安装
   - 设置OpenCascade_ROOT环境变量
   - 检查CMakeLists.txt中的路径设置

2. **运行时错误：曲线延伸失败**
   - 检查曲线是否为有界曲线
   - 验证目标点是否合理
   - 确保连续性要求满足

3. **偏移操作失败**
   - 检查偏移距离是否合理
   - 验证偏移方向是否有效
   - 确保基础曲线几何有效

## 许可证

本项目遵循OpenCascade的LGPL 2.1许可证。

## 贡献

欢迎提交Issue和Pull Request来改进这个示例。