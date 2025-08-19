#include <iostream>
#include <iomanip>

// 使用OCCT的gp模块
#include "FoundationClasses/TKMath/gp/gp_Pnt.hxx"
#include "FoundationClasses/TKMath/gp/gp_Dir.hxx"
#include "FoundationClasses/TKMath/gp/gp_Lin.hxx"
#include "FoundationClasses/TKMath/gp/gp_Vec.hxx"

// 计算点在直线上的参数值
double getParameterOnLine(const gp_Lin& line, const gp_Pnt& testPoint) {
    // 获取直线的参考点和方向
    gp_Pnt linePoint = line.Location();
    gp_Dir lineDir = line.Direction();
    
    // 计算从直线参考点到测试点的向量
    gp_Vec vec(testPoint, linePoint);
    
    // 计算参数值：t = -vec · lineDir
    double t = -vec.Dot(gp_Vec(lineDir));
    
    return t;
}

int main() {
    std::cout << "=== 简化版OCCT程序：计算点在直线上的参数值 ===" << std::endl;
    
    try {
        // 创建直线：通过点(0,0,0)和方向(1,1,1)
        gp_Pnt origin(0.0, 0.0, 0.0);
        gp_Dir direction(1.0, 1.0, 1.0);
        gp_Lin line(origin, direction);
        
        std::cout << "直线定义：" << std::endl;
        std::cout << "  参考点: (" << origin.X() << ", " << origin.Y() << ", " << origin.Z() << ")" << std::endl;
        std::cout << "  方向: (" << direction.X() << ", " << direction.Y() << ", " << direction.Z() << ")" << std::endl;
        
        // 测试点
        gp_Pnt testPoint(2.0, 2.0, 2.0);
        double t = getParameterOnLine(line, testPoint);
        
        std::cout << "\n测试结果：" << std::endl;
        std::cout << "测试点: (" << testPoint.X() << ", " << testPoint.Y() << ", " << testPoint.Z() << ")" << std::endl;
        std::cout << "参数值 t = " << std::fixed << std::setprecision(6) << t << std::endl;
        
        if (std::abs(t) < 1e-10) {
            std::cout << "→ 点正好在参考点处 (t ≈ 0)" << std::endl;
        } else if (t > 0) {
            std::cout << "→ 点在直线正方向上" << std::endl;
        } else {
            std::cout << "→ 点在直线反方向上" << std::endl;
        }
        
        // 验证：根据参数值计算点
        std::cout << "\n验证：根据参数值计算点" << std::endl;
        double testT = 2.5;
        gp_Pnt computedPoint = origin.Translated(gp_Vec(direction) * testT);
        std::cout << "参数值 t = " << testT << " 对应的点: (" 
                  << computedPoint.X() << ", " << computedPoint.Y() << ", " << computedPoint.Z() << ")" << std::endl;
        
        // 验证计算是否正确
        double actualT = getParameterOnLine(line, computedPoint);
        std::cout << "验证：该点的参数值 t = " << std::fixed << std::setprecision(6) << actualT << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "未知错误发生" << std::endl;
        return 1;
    }
    
    return 0;
}