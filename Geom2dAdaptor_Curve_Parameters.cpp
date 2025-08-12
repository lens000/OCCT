#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <gp_Ax22d.hxx>
#include <gp.hxx>
#include <iostream>
#include <iomanip>

// 演示如何获取Geom2dAdaptor_Curve的各种参数值
void DemonstrateGeom2dAdaptorCurveParameters()
{
    std::cout << "=== Geom2dAdaptor_Curve 参数值获取示例 ===\n\n";
    
    // 1. 创建基础几何曲线
    Standard_Real radius = 5.0;
    Handle(Geom2d_Circle) circle = new Geom2d_Circle(gp::OX2d(), radius);
    
    // 2. 创建Geom2dAdaptor_Curve对象
    Geom2dAdaptor_Curve adaptorCurve(circle);
    
    std::cout << "1. 基本参数信息:\n";
    std::cout << "   曲线类型: " << adaptorCurve.GetType() << "\n";
    std::cout << "   首参数: " << adaptorCurve.FirstParameter() << "\n";
    std::cout << "   末参数: " << adaptorCurve.LastParameter() << "\n";
    std::cout << "   连续性: " << adaptorCurve.Continuity() << "\n";
    std::cout << "   是否闭合: " << (adaptorCurve.IsClosed() ? "是" : "否") << "\n";
    std::cout << "   是否周期: " << (adaptorCurve.IsPeriodic() ? "是" : "否") << "\n";
    if (adaptorCurve.IsPeriodic()) {
        std::cout << "   周期: " << adaptorCurve.Period() << "\n";
    }
    
    // 3. 获取特定参数值处的几何信息
    std::cout << "\n2. 特定参数值处的几何信息:\n";
    Standard_Real param = M_PI / 4.0; // 45度
    
    // 获取点坐标
    gp_Pnt2d point = adaptorCurve.Value(param);
    std::cout << "   参数 " << param << " 处的点: (" 
              << point.X() << ", " << point.Y() << ")\n";
    
    // 获取一阶导数（切向量）
    gp_Pnt2d P;
    gp_Vec2d V;
    adaptorCurve.D1(param, P, V);
    std::cout << "   参数 " << param << " 处的切向量: (" 
              << V.X() << ", " << V.Y() << ")\n";
    
    // 获取二阶导数
    gp_Vec2d V2;
    adaptorCurve.D2(param, P, V, V2);
    std::cout << "   参数 " << param << " 处的二阶导数: (" 
              << V2.X() << ", " << V2.Y() << ")\n";
    
    // 4. 获取曲线的几何特性
    std::cout << "\n3. 曲线几何特性:\n";
    
    // 对于圆，获取圆心和半径
    if (adaptorCurve.GetType() == GeomAbs_Circle) {
        gp_Circ2d circle2d = adaptorCurve.Circle();
        gp_Pnt2d center = circle2d.Location();
        Standard_Real circleRadius = circle2d.Radius();
        std::cout << "   圆心: (" << center.X() << ", " << center.Y() << ")\n";
        std::cout << "   半径: " << circleRadius << "\n";
    }
    
    // 5. 创建B样条曲线示例
    std::cout << "\n4. B样条曲线参数:\n";
    
    // 创建控制点数组
    TColgp_Array1OfPnt2d controlPoints(1, 4);
    controlPoints.SetValue(1, gp_Pnt2d(0, 0));
    controlPoints.SetValue(2, gp_Pnt2d(2, 3));
    controlPoints.SetValue(3, gp_Pnt2d(4, 1));
    controlPoints.SetValue(4, gp_Pnt2d(6, 4));
    
    // 创建B样条曲线
    Handle(Geom2d_BSplineCurve) bspline = 
        Geom2dAPI_PointsToBSpline(controlPoints);
    
    // 创建B样条曲线的适配器
    Geom2dAdaptor_Curve bsplineAdaptor(bspline);
    
    std::cout << "   B样条曲线类型: " << bsplineAdaptor.GetType() << "\n";
    std::cout << "   首参数: " << bsplineAdaptor.FirstParameter() << "\n";
    std::cout << "   末参数: " << bsplineAdaptor.LastParameter() << "\n";
    std::cout << "   度数: " << bsplineAdaptor.Degree() << "\n";
    std::cout << "   是否有理: " << (bsplineAdaptor.IsRational() ? "是" : "否") << "\n";
    std::cout << "   控制点数量: " << bsplineAdaptor.NbPoles() << "\n";
    std::cout << "   节点数量: " << bsplineAdaptor.NbKnots() << "\n";
    
    // 6. 获取曲线的边界框
    std::cout << "\n5. 曲线边界框:\n";
    
    // 使用BRepLib_Add2dCurve获取边界框
    Bnd_Box2d boundingBox;
    BndLib_Add2dCurve::Add(bsplineAdaptor, Precision::Approximation(), boundingBox);
    
    Standard_Real xMin, yMin, xMax, yMax;
    boundingBox.Get(xMin, yMin, xMax, yMax);
    
    std::cout << "   边界框: X[" << xMin << ", " << xMax 
              << "], Y[" << yMin << ", " << yMax << "]\n";
    
    // 7. 获取曲线的连续性区间
    std::cout << "\n6. 连续性区间:\n";
    
    Standard_Integer numIntervals = bsplineAdaptor.NbIntervals(GeomAbs_C2);
    std::cout << "   C2连续性区间数量: " << numIntervals << "\n";
    
    if (numIntervals > 0) {
        TColStd_Array1OfReal intervals(1, numIntervals + 1);
        bsplineAdaptor.Intervals(intervals, GeomAbs_C2);
        
        std::cout << "   区间边界参数: ";
        for (Standard_Integer i = 1; i <= intervals.Length(); i++) {
            std::cout << intervals(i);
            if (i < intervals.Length()) std::cout << ", ";
        }
        std::cout << "\n";
    }
    
    // 8. 参数分辨率
    std::cout << "\n7. 参数分辨率:\n";
    Standard_Real tolerance = 0.001;
    Standard_Real resolution = bsplineAdaptor.Resolution(tolerance);
    std::cout << "   容差 " << tolerance << " 对应的参数分辨率: " << resolution << "\n";
    
    // 9. 获取曲线的原始几何对象
    std::cout << "\n8. 原始几何对象:\n";
    const Handle(Geom2d_Curve)& originalCurve = adaptorCurve.Curve();
    std::cout << "   原始曲线类型: " << originalCurve->DynamicType()->Name() << "\n";
    
    // 10. 演示参数化采样
    std::cout << "\n9. 参数化采样:\n";
    Standard_Integer numSamples = 10;
    for (Standard_Integer i = 0; i <= numSamples; i++) {
        Standard_Real t = adaptorCurve.FirstParameter() + 
                         (adaptorCurve.LastParameter() - adaptorCurve.FirstParameter()) * i / numSamples;
        gp_Pnt2d samplePoint = adaptorCurve.Value(t);
        std::cout << "   参数 " << std::fixed << std::setprecision(3) << t 
                  << " -> 点(" << samplePoint.X() << ", " << samplePoint.Y() << ")\n";
    }
}

int main()
{
    try {
        DemonstrateGeom2dAdaptorCurveParameters();
        std::cout << "\n=== 示例执行完成 ===\n";
    }
    catch (const Standard_Failure& e) {
        std::cerr << "错误: " << e.GetMessageString() << "\n";
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "标准异常: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}