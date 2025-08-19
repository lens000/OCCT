#include <iostream>
#include <iomanip>
#include <vector>

// OCCT头文件 - 使用相对路径
#include "FoundationClasses/TKMath/gp/gp_Pnt.hxx"
#include "FoundationClasses/TKMath/gp/gp_Dir.hxx"
#include "FoundationClasses/TKMath/gp/gp_Lin.hxx"
#include "FoundationClasses/TKMath/gp/gp_Vec.hxx"
#include "FoundationClasses/TKMath/gp/gp_Ax1.hxx"

// 计算点在直线上的参数值
double getParameterOnLine(const gp_Lin& line, const gp_Pnt& testPoint) {
    // 获取直线的参考点和方向
    gp_Pnt linePoint = line.Location();
    gp_Dir lineDir = line.Direction();
    
    // 计算从直线参考点到测试点的向量
    gp_Vec vec(testPoint, linePoint);
    
    // 计算参数值：t = -vec · lineDir
    // 注意：gp_Vec的构造函数是(终点, 起点)，所以这里用负号
    double t = -vec.Dot(gp_Vec(lineDir));
    
    return t;
}

// 计算点到直线的距离
double distanceToLine(const gp_Lin& line, const gp_Pnt& testPoint) {
    // 获取直线的参考点和方向
    gp_Pnt linePoint = line.Location();
    gp_Dir lineDir = line.Direction();
    
    // 计算从直线参考点到测试点的向量
    gp_Vec vec(testPoint, linePoint);
    
    // 计算参数值
    double t = -vec.Dot(gp_Vec(lineDir));
    
    // 计算直线上距离测试点最近的点
    gp_Pnt closestPoint = linePoint.Translated(gp_Vec(lineDir) * t);
    
    // 计算距离
    return testPoint.Distance(closestPoint);
}

// 判断点是否在直线上
bool isPointOnLine(const gp_Lin& line, const gp_Pnt& testPoint, double tolerance = 1e-6) {
    return distanceToLine(line, testPoint) < tolerance;
}

// 根据参数值计算直线上对应的点
gp_Pnt getPointAtParameter(const gp_Lin& line, double t) {
    gp_Pnt linePoint = line.Location();
    gp_Dir lineDir = line.Direction();
    
    return linePoint.Translated(gp_Vec(lineDir) * t);
}

// 打印点信息
void printPoint(const gp_Pnt& p, const std::string& name = "") {
    if (!name.empty()) {
        std::cout << name << ": ";
    }
    std::cout << "(" << std::fixed << std::setprecision(6) 
              << p.X() << ", " << p.Y() << ", " << p.Z() << ")" << std::endl;
}

// 打印直线信息
void printLine(const gp_Lin& line, const std::string& name = "") {
    if (!name.empty()) {
        std::cout << name << ": " << std::endl;
    }
    std::cout << "  参考点: ";
    printPoint(line.Location(), "");
    std::cout << "  方向: (" << std::fixed << std::setprecision(6)
              << line.Direction().X() << ", " 
              << line.Direction().Y() << ", " 
              << line.Direction().Z() << ")" << std::endl;
}

int main() {
    std::cout << "=== 使用OCCT计算点在直线上的参数值 ===" << std::endl;
    
    try {
        // 示例1：通过参考点和方向定义直线
        std::cout << "\n示例1：通过参考点和方向定义直线" << std::endl;
        
        gp_Pnt origin(0.0, 0.0, 0.0);
        gp_Dir direction(1.0, 1.0, 1.0);
        gp_Lin line1(origin, direction);
        
        printLine(line1, "直线L1");
        
        // 测试点
        std::vector<gp_Pnt> testPoints1 = {
            gp_Pnt(2.0, 2.0, 2.0),      // 在直线上
            gp_Pnt(0.5, 0.5, 0.5),      // 在直线上
            gp_Pnt(1.0, 0.0, 0.0),      // 不在直线上
            gp_Pnt(-1.0, -1.0, -1.0),   // 在直线反向延长线上
            gp_Pnt(3.0, 3.0, 3.0)       // 在直线延长线上
        };
        
        std::cout << "\n测试结果：" << std::endl;
        for (size_t i = 0; i < testPoints1.size(); ++i) {
            const gp_Pnt& p = testPoints1[i];
            double t = getParameterOnLine(line1, p);
            double dist = distanceToLine(line1, p);
            bool onLine = isPointOnLine(line1, p);
            
            std::cout << "点" << (i+1) << " ";
            printPoint(p, "");
            std::cout << "  参数值 t = " << std::fixed << std::setprecision(6) << t << std::endl;
            std::cout << "  到直线距离 = " << std::fixed << std::setprecision(6) << dist << std::endl;
            
            if (onLine) {
                std::cout << "  → 点在直线上" << std::endl;
                if (std::abs(t) < 1e-10) {
                    std::cout << "  → 点正好在参考点处 (t ≈ 0)" << std::endl;
                } else if (t > 0) {
                    std::cout << "  → 点在直线正方向上" << std::endl;
                } else {
                    std::cout << "  → 点在直线反方向上" << std::endl;
                }
            } else {
                std::cout << "  → 点不在直线上" << std::endl;
            }
            std::cout << std::endl;
        }
        
        // 示例2：通过两点定义直线
        std::cout << "\n示例2：通过两点定义直线" << std::endl;
        
        gp_Pnt p1(0.0, 0.0, 0.0);
        gp_Pnt p2(1.0, 0.0, 0.0);  // X轴方向
        
        // 计算方向向量
        gp_Vec dirVec(p1, p2);
        gp_Dir dir(dirVec);
        gp_Lin line2(p1, dir);
        
        printLine(line2, "直线L2 (X轴)");
        
        // 测试点
        gp_Pnt testPoint2(3.0, 2.0, 1.0);
        double t2 = getParameterOnLine(line2, testPoint2);
        double dist2 = distanceToLine(line2, testPoint2);
        bool onLine2 = isPointOnLine(line2, testPoint2);
        
        std::cout << "\n测试结果：" << std::endl;
        printPoint(testPoint2, "测试点");
        std::cout << "  参数值 t = " << std::fixed << std::setprecision(6) << t2 << std::endl;
        std::cout << "  到直线距离 = " << std::fixed << std::setprecision(6) << dist2 << std::endl;
        
        if (onLine2) {
            std::cout << "  → 点在直线上" << std::endl;
        } else {
            std::cout << "  → 点不在直线上" << std::endl;
        }
        
        // 验证：根据参数值计算点
        std::cout << "\n验证：根据参数值计算点" << std::endl;
        double testT = 2.5;
        gp_Pnt computedPoint = getPointAtParameter(line2, testT);
        std::cout << "参数值 t = " << testT << " 对应的点: ";
        printPoint(computedPoint, "");
        
        // 验证计算是否正确
        double actualT = getParameterOnLine(line2, computedPoint);
        std::cout << "验证：该点的参数值 t = " << std::fixed << std::setprecision(6) << actualT << std::endl;
        
        // 示例3：使用gp_Ax1构造直线
        std::cout << "\n示例3：使用gp_Ax1构造直线" << std::endl;
        
        gp_Ax1 axis(origin, direction);
        gp_Lin line3(axis);
        
        printLine(line3, "直线L3");
        
        // 测试点
        gp_Pnt testPoint3(1.5, 1.5, 1.5);
        double t3 = getParameterOnLine(line3, testPoint3);
        double dist3 = distanceToLine(line3, testPoint3);
        
        std::cout << "\n测试结果：" << std::endl;
        printPoint(testPoint3, "测试点");
        std::cout << "  参数值 t = " << std::fixed << std::setprecision(6) << t3 << std::endl;
        std::cout << "  到直线距离 = " << std::fixed << std::setprecision(6) << dist3 << std::endl;
        
        // 示例4：计算点到直线的投影点
        std::cout << "\n示例4：计算点到直线的投影点" << std::endl;
        
        gp_Pnt testPoint4(2.0, 1.0, 0.0);  // 不在X轴上的点
        double t4 = getParameterOnLine(line2, testPoint4);
        gp_Pnt projectionPoint = getPointAtParameter(line2, t4);
        
        printPoint(testPoint4, "测试点");
        std::cout << "  参数值 t = " << std::fixed << std::setprecision(6) << t4 << std::endl;
        printPoint(projectionPoint, "投影点");
        std::cout << "  到直线距离 = " << std::fixed << std::setprecision(6) 
                  << testPoint4.Distance(projectionPoint) << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "未知错误发生" << std::endl;
        return 1;
    }
    
    return 0;
}