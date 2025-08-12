# Geom2dAdaptor_Curve 参数值获取方法总结

## 概述
`Geom2dAdaptor_Curve` 是 OpenCASCADE (OCCT) 中用于适配 2D 几何曲线的类，它提供了统一的接口来访问各种 2D 曲线的参数和几何信息。

## 主要参数获取方法

### 1. 基本参数信息
```cpp
Geom2dAdaptor_Curve adaptorCurve(curve);

// 获取参数范围
Standard_Real firstParam = adaptorCurve.FirstParameter();  // 首参数
Standard_Real lastParam = adaptorCurve.LastParameter();    // 末参数

// 获取曲线类型
GeomAbs_CurveType curveType = adaptorCurve.GetType();

// 获取连续性
GeomAbs_Shape continuity = adaptorCurve.Continuity();

// 检查曲线特性
Standard_Boolean isClosed = adaptorCurve.IsClosed();      // 是否闭合
Standard_Boolean isPeriodic = adaptorCurve.IsPeriodic();  // 是否周期
Standard_Real period = adaptorCurve.Period();             // 周期值（如果周期）
```

### 2. 几何点值和导数
```cpp
Standard_Real param = 1.5; // 参数值

// 获取点坐标
gp_Pnt2d point = adaptorCurve.Value(param);

// 获取一阶导数（切向量）
gp_Pnt2d P;
gp_Vec2d V;
adaptorCurve.D1(param, P, V);

// 获取二阶导数
gp_Vec2d V2;
adaptorCurve.D2(param, P, V, V2);

// 获取N阶导数
gp_Vec2d VN = adaptorCurve.DN(param, N);
```

### 3. 特定曲线类型的几何参数

#### 圆 (Circle)
```cpp
if (adaptorCurve.GetType() == GeomAbs_Circle) {
    gp_Circ2d circle2d = adaptorCurve.Circle();
    gp_Pnt2d center = circle2d.Location();        // 圆心
    Standard_Real radius = circle2d.Radius();     // 半径
}
```

#### 直线 (Line)
```cpp
if (adaptorCurve.GetType() == GeomAbs_Line) {
    gp_Lin2d line2d = adaptorCurve.Line();
    gp_Pnt2d point = line2d.Location();           // 直线上一点
    gp_Dir2d direction = line2d.Direction();      // 方向向量
}
```

#### 椭圆 (Ellipse)
```cpp
if (adaptorCurve.GetType() == GeomAbs_Ellipse) {
    gp_Elips2d ellipse2d = adaptorCurve.Ellipse();
    gp_Pnt2d center = ellipse2d.Location();       // 中心
    Standard_Real majorRadius = ellipse2d.MajorRadius();  // 长半轴
    Standard_Real minorRadius = ellipse2d.MinorRadius();  // 短半轴
}
```

#### B样条曲线 (BSpline)
```cpp
if (adaptorCurve.GetType() == GeomAbs_BSplineCurve) {
    Standard_Integer degree = adaptorCurve.Degree();           // 度数
    Standard_Boolean isRational = adaptorCurve.IsRational();   // 是否有理
    Standard_Integer nbPoles = adaptorCurve.NbPoles();        // 控制点数量
    Standard_Integer nbKnots = adaptorCurve.NbKnots();        // 节点数量
    
    // 获取B样条曲线对象
    Handle(Geom2d_BSplineCurve) bspline = adaptorCurve.BSpline();
}
```

### 4. 连续性分析
```cpp
// 获取指定连续性的区间数量
Standard_Integer numIntervals = adaptorCurve.NbIntervals(GeomAbs_C2);

// 获取区间边界参数
TColStd_Array1OfReal intervals(1, numIntervals + 1);
adaptorCurve.Intervals(intervals, GeomAbs_C2);
```

### 5. 边界框和分辨率
```cpp
// 获取参数分辨率
Standard_Real tolerance = 0.001;
Standard_Real resolution = adaptorCurve.Resolution(tolerance);

// 获取边界框（需要额外的库支持）
Bnd_Box2d boundingBox;
BndLib_Add2dCurve::Add(adaptorCurve, Precision::Approximation(), boundingBox);
```

### 6. 原始几何对象
```cpp
// 获取原始的几何曲线对象
const Handle(Geom2d_Curve)& originalCurve = adaptorCurve.Curve();
```

### 7. 参数化采样
```cpp
// 在参数范围内均匀采样
Standard_Integer numSamples = 10;
for (Standard_Integer i = 0; i <= numSamples; i++) {
    Standard_Real t = adaptorCurve.FirstParameter() + 
                     (adaptorCurve.LastParameter() - adaptorCurve.FirstParameter()) * i / numSamples;
    gp_Pnt2d samplePoint = adaptorCurve.Value(t);
    // 处理采样点...
}
```

## 使用注意事项

1. **参数范围**: 始终检查 `FirstParameter()` 和 `LastParameter()` 确保参数值在有效范围内
2. **连续性检查**: 在调用导数方法前检查连续性要求
3. **类型检查**: 在调用特定曲线类型的方法前检查 `GetType()` 返回值
4. **异常处理**: 使用 try-catch 块处理可能的异常
5. **内存管理**: 使用 `Handle` 智能指针管理几何对象

## 常见应用场景

- **曲线分析**: 获取曲线的几何特性和参数信息
- **路径规划**: 在曲线上进行参数化采样
- **几何计算**: 计算曲线的导数、曲率等
- **CAD/CAM**: 在工程应用中处理2D几何曲线
- **数值计算**: 为数值算法提供统一的曲线接口

## 相关类

- `Geom2d_Curve`: 基础2D几何曲线类
- `Adaptor2d_Curve2d`: 适配器基类
- `gp_Pnt2d`, `gp_Vec2d`: 2D点和向量类
- `GeomAbs_CurveType`: 曲线类型枚举
- `GeomAbs_Shape`: 连续性类型枚举