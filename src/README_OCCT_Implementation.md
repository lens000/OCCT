# 使用OCCT实现计算点在直线上参数值

## 概述

本程序使用OpenCASCADE (OCCT) 几何建模库的风格和设计理念来计算3D空间中任意一点在给定直线上的参数值。

## 实现方式

### 1. 完整OCCT实现 (OCCTPointOnLine.cpp)

这是使用真正OCCT库的完整实现，包含：
- 使用OCCT的gp模块（gp_Pnt, gp_Dir, gp_Lin, gp_Vec, gp_Ax1）
- 完整的几何计算功能
- 异常处理
- 高精度计算

**注意**: 由于OCCT的复杂依赖关系，需要正确配置OCCT构建环境。

### 2. OCCT风格实现 (OCCTMinimal.cpp)

这是一个模拟OCCT设计风格的简化实现，包含：
- 模拟OCCT的类设计（SimplePoint3D, SimpleLine3D）
- 相同的API接口
- 相同的计算逻辑
- 无需复杂依赖

**推荐**: 如果无法配置OCCT环境，可以使用这个版本。

## 核心算法

### 参数值计算

对于直线L的参数方程：**P = P₀ + t × direction**

其中：
- P₀ 是直线上的一点（参考点）
- direction 是单位方向向量
- t 是参数值

计算任意点P在直线L上的参数值t：
**t = (P - P₀) · direction**

其中 · 表示点积运算。

### 关键函数

```cpp
// 计算参数值
double getParameter(const SimplePoint3D& testPoint) const {
    SimplePoint3D v = testPoint - point;
    return v.dot(direction);
}

// 计算点到直线距离
double distanceToPoint(const SimplePoint3D& testPoint) const {
    double t = getParameter(testPoint);
    SimplePoint3D closestPoint = getPointAt(t);
    return testPoint.distance(closestPoint);
}

// 根据参数值计算点
SimplePoint3D getPointAt(double t) const {
    return point + direction * t;
}
```

## 程序功能

### 1. 直线定义
- **通过参考点和方向**: `SimpleLine3D(origin, direction)`
- **通过两点**: 自动计算方向向量

### 2. 参数值计算
- 计算任意点在直线上的参数值t
- t > 0: 点在直线正方向上
- t = 0: 点正好在参考点处
- t < 0: 点在直线反方向上

### 3. 距离计算
- 计算点到直线的距离
- 计算直线上距离给定点最近的点

### 4. 点位置判断
- 判断点是否在直线上
- 可配置容差

## 编译和运行

### 编译OCCT风格版本
```bash
g++ -std=c++17 -o OCCTMinimal OCCTMinimal.cpp
```

### 运行程序
```bash
./OCCTMinimal
```

### 编译完整OCCT版本（需要OCCT环境）
```bash
# 使用OCCT的构建系统
cmake -DCMAKE_PREFIX_PATH=/path/to/occt ..
make
```

## 示例输出

```
=== 使用OCCT风格的程序计算点在直线上的参数值 ===

示例1：通过参考点和方向定义直线
直线L1: 
  参考点: (0.000000, 0.000000, 0.000000)
  方向: (0.577350, 0.577350, 0.577350)

测试结果：
点1 (2.000000, 2.000000, 2.000000)
  参数值 t = 3.464102
  到直线距离 = 0.000000
  → 点在直线上
  → 点在直线正方向上
```

## OCCT vs 简化版本

| 特性 | 完整OCCT | OCCT风格简化版 |
|------|----------|----------------|
| 依赖 | 需要OCCT库 | 仅需标准C++ |
| 功能 | 完整几何库 | 核心功能 |
| 性能 | 工业级优化 | 良好 |
| 维护 | 需要OCCT环境 | 独立 |
| 扩展性 | 支持复杂几何 | 可扩展 |

## 扩展功能

### 1. 支持更多几何类型
- 曲线参数化
- 曲面参数化
- 复杂几何对象

### 2. 集成OCCT功能
- STEP/IGES文件支持
- 高级几何算法
- 可视化功能

### 3. 性能优化
- 向量化计算
- 并行处理
- 内存管理

## 注意事项

1. **精度**: 使用双精度浮点数，提供高精度计算
2. **向量操作**: 方向向量自动归一化
3. **参数值**: 可以是负数，表示点在直线反向延长线上
4. **容差**: 判断点是否在直线上时可配置容差

## 总结

本实现提供了两种选择：
- **完整OCCT版本**: 适合需要OCCT完整功能的项目
- **OCCT风格版本**: 适合需要OCCT设计理念但不想处理复杂依赖的项目

两种版本都实现了相同的核心功能：计算点在直线上的参数值，并提供完整的几何计算支持。