#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

// OCCT 头文件
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Elips.hxx>

// 3D 几何类
#include <Geom_Curve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_BSplineCurve.hxx>

// 2D 几何类
#include <Geom2d_Curve.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Line.hxx>

// 投影和极值计算类
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <Geom2dAPI_ProjectPointOnCurve.hxx>
#include <Extrema_ExtPC.hxx>
#include <Extrema_ExtPC2d.hxx>

// 工具类
#include <GeomAdaptor_Curve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

class PointToCurveDistance {
public:
    
    /**
     * 方法1: 使用 GeomAPI_ProjectPointOnCurve 计算3D点到3D曲线的最短距离
     * 这是最常用和推荐的方法
     */
    static Standard_Real Calculate3DDistance_Method1(const gp_Pnt& point, 
                                                     const Handle(Geom_Curve)& curve,
                                                     gp_Pnt& nearestPoint,
                                                     Standard_Real& parameter) {
        try {
            // 创建投影器
            GeomAPI_ProjectPointOnCurve projector(point, curve);
            
            // 检查是否找到投影点
            if (projector.NbPoints() == 0) {
                std::cerr << "无法找到投影点" << std::endl;
                return -1.0;
            }
            
            // 获取最近的投影点和距离
            nearestPoint = projector.NearestPoint();
            parameter = projector.LowerDistanceParameter();
            Standard_Real distance = projector.LowerDistance();
            
            std::cout << "方法1 - GeomAPI_ProjectPointOnCurve:" << std::endl;
            std::cout << "  最短距离: " << distance << std::endl;
            std::cout << "  投影点: (" << nearestPoint.X() << ", " 
                      << nearestPoint.Y() << ", " << nearestPoint.Z() << ")" << std::endl;
            std::cout << "  参数值: " << parameter << std::endl;
            
            return distance;
            
        } catch (const Standard_Failure& e) {
            std::cerr << "计算3D距离时发生错误: " << e.GetMessageString() << std::endl;
            return -1.0;
        }
    }
    
    /**
     * 方法2: 使用 GeomAPI_ProjectPointOnCurve 在指定参数范围内计算距离
     */
    static Standard_Real Calculate3DDistance_WithRange(const gp_Pnt& point,
                                                       const Handle(Geom_Curve)& curve,
                                                       Standard_Real uMin,
                                                       Standard_Real uMax,
                                                       gp_Pnt& nearestPoint,
                                                       Standard_Real& parameter) {
        try {
            // 在指定参数范围内创建投影器
            GeomAPI_ProjectPointOnCurve projector(point, curve, uMin, uMax);
            
            if (projector.NbPoints() == 0) {
                std::cerr << "在指定范围内无法找到投影点" << std::endl;
                return -1.0;
            }
            
            nearestPoint = projector.NearestPoint();
            parameter = projector.LowerDistanceParameter();
            Standard_Real distance = projector.LowerDistance();
            
            std::cout << "方法2 - 指定范围投影 [" << uMin << ", " << uMax << "]:" << std::endl;
            std::cout << "  最短距离: " << distance << std::endl;
            std::cout << "  投影点: (" << nearestPoint.X() << ", " 
                      << nearestPoint.Y() << ", " << nearestPoint.Z() << ")" << std::endl;
            std::cout << "  参数值: " << parameter << std::endl;
            
            return distance;
            
        } catch (const Standard_Failure& e) {
            std::cerr << "在指定范围内计算距离时发生错误: " << e.GetMessageString() << std::endl;
            return -1.0;
        }
    }
    
    /**
     * 方法3: 使用 Extrema_ExtPC 计算所有极值点
     * 可以找到所有局部最小值和最大值
     */
    static Standard_Real Calculate3DDistance_Method3(const gp_Pnt& point,
                                                     const Handle(Geom_Curve)& curve,
                                                     std::vector<gp_Pnt>& extremaPoints,
                                                     std::vector<Standard_Real>& parameters,
                                                     std::vector<Standard_Real>& distances) {
        try {
            // 创建曲线适配器
            GeomAdaptor_Curve adapter(curve);
            
            // 创建极值计算器
            Extrema_ExtPC extrema(point, adapter);
            
            if (!extrema.IsDone()) {
                std::cerr << "极值计算失败" << std::endl;
                return -1.0;
            }
            
            Standard_Integer nbExt = extrema.NbExt();
            if (nbExt == 0) {
                std::cerr << "未找到极值点" << std::endl;
                return -1.0;
            }
            
            // 清空输出容器
            extremaPoints.clear();
            parameters.clear();
            distances.clear();
            
            Standard_Real minDistance = std::numeric_limits<Standard_Real>::max();
            
            std::cout << "方法3 - Extrema_ExtPC (找到 " << nbExt << " 个极值点):" << std::endl;
            
            for (Standard_Integer i = 1; i <= nbExt; ++i) {
                Standard_Real sqDist = extrema.SquareDistance(i);
                Standard_Real dist = std::sqrt(sqDist);
                
                // 获取极值点的参数
                Standard_Real u;
                extrema.Point(i).Parameter(u);
                
                // 计算曲线上的点
                gp_Pnt curvePoint = curve->Value(u);
                
                extremaPoints.push_back(curvePoint);
                parameters.push_back(u);
                distances.push_back(dist);
                
                if (dist < minDistance) {
                    minDistance = dist;
                }
                
                std::cout << "  极值点 " << i << ": 距离=" << dist 
                          << ", 参数=" << u 
                          << ", 点=(" << curvePoint.X() << ", " 
                          << curvePoint.Y() << ", " << curvePoint.Z() << ")" << std::endl;
            }
            
            std::cout << "  最短距离: " << minDistance << std::endl;
            
            return minDistance;
            
        } catch (const Standard_Failure& e) {
            std::cerr << "使用Extrema计算距离时发生错误: " << e.GetMessageString() << std::endl;
            return -1.0;
        }
    }
    
    /**
     * 方法4: 使用 Geom2dAPI_ProjectPointOnCurve 计算2D点到2D曲线的距离
     */
    static Standard_Real Calculate2DDistance(const gp_Pnt2d& point,
                                            const Handle(Geom2d_Curve)& curve,
                                            gp_Pnt2d& nearestPoint,
                                            Standard_Real& parameter) {
        try {
            // 创建2D投影器
            Geom2dAPI_ProjectPointOnCurve projector(point, curve);
            
            if (projector.NbPoints() == 0) {
                std::cerr << "无法找到2D投影点" << std::endl;
                return -1.0;
            }
            
            nearestPoint = projector.NearestPoint();
            parameter = projector.LowerDistanceParameter();
            Standard_Real distance = projector.LowerDistance();
            
            std::cout << "方法4 - 2D曲线投影:" << std::endl;
            std::cout << "  最短距离: " << distance << std::endl;
            std::cout << "  投影点: (" << nearestPoint.X() << ", " 
                      << nearestPoint.Y() << ")" << std::endl;
            std::cout << "  参数值: " << parameter << std::endl;
            
            return distance;
            
        } catch (const Standard_Failure& e) {
            std::cerr << "计算2D距离时发生错误: " << e.GetMessageString() << std::endl;
            return -1.0;
        }
    }
    
    /**
     * 辅助方法: 考虑曲线端点的距离计算
     * 对于有限曲线，有时最短距离可能在端点处
     */
    static Standard_Real CalculateDistanceWithEndpoints(const gp_Pnt& point,
                                                        const Handle(Geom_Curve)& curve,
                                                        gp_Pnt& nearestPoint,
                                                        Standard_Real& parameter,
                                                        bool& isEndpoint) {
        // 首先使用标准投影方法
        Standard_Real projectionDistance = Calculate3DDistance_Method1(point, curve, nearestPoint, parameter);
        
        // 获取曲线的参数范围
        Standard_Real uFirst = curve->FirstParameter();
        Standard_Real uLast = curve->LastParameter();
        
        // 计算到端点的距离
        gp_Pnt startPoint = curve->Value(uFirst);
        gp_Pnt endPoint = curve->Value(uLast);
        
        Standard_Real startDistance = point.Distance(startPoint);
        Standard_Real endDistance = point.Distance(endPoint);
        
        Standard_Real minDistance = projectionDistance;
        isEndpoint = false;
        
        std::cout << "端点距离检查:" << std::endl;
        std::cout << "  起点距离: " << startDistance << std::endl;
        std::cout << "  终点距离: " << endDistance << std::endl;
        std::cout << "  投影距离: " << projectionDistance << std::endl;
        
        if (startDistance < minDistance) {
            minDistance = startDistance;
            nearestPoint = startPoint;
            parameter = uFirst;
            isEndpoint = true;
            std::cout << "  最短距离在起点处" << std::endl;
        }
        
        if (endDistance < minDistance) {
            minDistance = endDistance;
            nearestPoint = endPoint;
            parameter = uLast;
            isEndpoint = true;
            std::cout << "  最短距离在终点处" << std::endl;
        }
        
        if (!isEndpoint) {
            std::cout << "  最短距离在曲线内部" << std::endl;
        }
        
        return minDistance;
    }
};

// 示例函数
void demonstratePointToCurveDistance() {
    std::cout << "=== OCCT 点到曲线最短距离计算示例 ===" << std::endl << std::endl;
    
    // 创建一个测试点
    gp_Pnt testPoint(5.0, 3.0, 2.0);
    std::cout << "测试点: (" << testPoint.X() << ", " << testPoint.Y() << ", " << testPoint.Z() << ")" << std::endl << std::endl;
    
    // 示例1: 圆形曲线
    std::cout << "--- 示例1: 3D圆形曲线 ---" << std::endl;
    gp_Ax2 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    gp_Circ circle(axis, 3.0);
    Handle(Geom_Circle) circleGeom = new Geom_Circle(circle);
    
    gp_Pnt nearestPoint;
    Standard_Real parameter;
    
    // 方法1: 标准投影
    PointToCurveDistance::Calculate3DDistance_Method1(testPoint, circleGeom, nearestPoint, parameter);
    std::cout << std::endl;
    
    // 方法3: 极值计算
    std::vector<gp_Pnt> extremaPoints;
    std::vector<Standard_Real> parameters;
    std::vector<Standard_Real> distances;
    PointToCurveDistance::Calculate3DDistance_Method3(testPoint, circleGeom, extremaPoints, parameters, distances);
    std::cout << std::endl;
    
    // 示例2: 直线
    std::cout << "--- 示例2: 3D直线 ---" << std::endl;
    gp_Pnt lineStart(0, 0, 0);
    gp_Dir lineDir(1, 1, 0);
    Handle(Geom_Line) lineGeom = new Geom_Line(lineStart, lineDir);
    
    // 在有限范围内计算
    PointToCurveDistance::Calculate3DDistance_WithRange(testPoint, lineGeom, 0.0, 10.0, nearestPoint, parameter);
    std::cout << std::endl;
    
    // 示例3: 椭圆
    std::cout << "--- 示例3: 3D椭圆 ---" << std::endl;
    gp_Elips ellipse(axis, 4.0, 2.0);  // 长轴4.0, 短轴2.0
    Handle(Geom_Ellipse) ellipseGeom = new Geom_Ellipse(ellipse);
    
    bool isEndpoint;
    PointToCurveDistance::CalculateDistanceWithEndpoints(testPoint, ellipseGeom, nearestPoint, parameter, isEndpoint);
    std::cout << std::endl;
    
    // 示例4: 2D曲线
    std::cout << "--- 示例4: 2D圆形曲线 ---" << std::endl;
    gp_Pnt2d testPoint2D(3.0, 2.0);
    gp_Circ2d circle2D(gp_Ax2d(gp_Pnt2d(0, 0), gp_Dir2d(1, 0)), 2.0);
    Handle(Geom2d_Circle) circle2DGeom = new Geom2d_Circle(circle2D);
    
    gp_Pnt2d nearestPoint2D;
    PointToCurveDistance::Calculate2DDistance(testPoint2D, circle2DGeom, nearestPoint2D, parameter);
    std::cout << std::endl;
    
    std::cout << "=== 示例完成 ===" << std::endl;
}

int main() {
    try {
        demonstratePointToCurveDistance();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "程序执行出错: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "未知错误" << std::endl;
        return 1;
    }
}