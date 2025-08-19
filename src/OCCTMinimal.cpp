#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector> // Added for testPoints1

// 简化的3D点和向量类，模拟OCCT的功能
class SimplePoint3D {
public:
    double x, y, z;
    
    SimplePoint3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    
    // 向量减法
    SimplePoint3D operator-(const SimplePoint3D& other) const {
        return SimplePoint3D(x - other.x, y - other.y, z - other.z);
    }
    
    // 向量加法
    SimplePoint3D operator+(const SimplePoint3D& other) const {
        return SimplePoint3D(x + other.x, y + other.y, z + other.z);
    }
    
    // 标量乘法
    SimplePoint3D operator*(double scalar) const {
        return SimplePoint3D(x * scalar, y * scalar, z * scalar);
    }
    
    // 点积
    double dot(const SimplePoint3D& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    // 向量长度
    double length() const {
        return sqrt(x * x + y * y + z * z);
    }
    
    // 归一化
    SimplePoint3D normalize() const {
        double len = length();
        if (len == 0) return SimplePoint3D();
        return SimplePoint3D(x / len, y / len, z / len);
    }
    
    // 距离计算
    double distance(const SimplePoint3D& other) const {
        return (*this - other).length();
    }
};

// 简化的直线类，模拟OCCT的gp_Lin
class SimpleLine3D {
private:
    SimplePoint3D point;      // 直线上的一点
    SimplePoint3D direction;  // 方向向量（已归一化）
    
public:
    SimpleLine3D(const SimplePoint3D& p, const SimplePoint3D& dir) : point(p) {
        direction = dir.normalize();
    }
    
    // 获取参考点
    SimplePoint3D getLocation() const { return point; }
    
    // 获取方向
    SimplePoint3D getDirection() const { return direction; }
    
    // 计算点在直线上的参数值
    double getParameter(const SimplePoint3D& testPoint) const {
        SimplePoint3D v = testPoint - point;
        return v.dot(direction);
    }
    
    // 根据参数值计算直线上对应的点
    SimplePoint3D getPointAt(double t) const {
        return point + direction * t;
    }
    
    // 计算点到直线的距离
    double distanceToPoint(const SimplePoint3D& testPoint) const {
        double t = getParameter(testPoint);
        SimplePoint3D closestPoint = getPointAt(t);
        return testPoint.distance(closestPoint);
    }
    
    // 判断点是否在直线上
    bool isPointOnLine(const SimplePoint3D& testPoint, double tolerance = 1e-6) const {
        return distanceToPoint(testPoint) < tolerance;
    }
};

// 打印点信息
void printPoint(const SimplePoint3D& p, const std::string& name = "") {
    if (!name.empty()) {
        std::cout << name << ": ";
    }
    std::cout << "(" << std::fixed << std::setprecision(6) 
              << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
}

// 打印直线信息
void printLine(const SimpleLine3D& line, const std::string& name = "") {
    if (!name.empty()) {
        std::cout << name << ": " << std::endl;
    }
    std::cout << "  参考点: ";
    printPoint(line.getLocation(), "");
    std::cout << "  方向: ";
    printPoint(line.getDirection(), "");
}

int main() {
    std::cout << "=== 使用OCCT风格的程序计算点在直线上的参数值 ===" << std::endl;
    
    try {
        // 示例1：通过参考点和方向定义直线
        std::cout << "\n示例1：通过参考点和方向定义直线" << std::endl;
        
        SimplePoint3D origin(0.0, 0.0, 0.0);
        SimplePoint3D direction(1.0, 1.0, 1.0);
        SimpleLine3D line1(origin, direction);
        
        printLine(line1, "直线L1");
        
        // 测试点
        std::vector<SimplePoint3D> testPoints1 = {
            SimplePoint3D(2.0, 2.0, 2.0),      // 在直线上
            SimplePoint3D(0.5, 0.5, 0.5),      // 在直线上
            SimplePoint3D(1.0, 0.0, 0.0),      // 不在直线上
            SimplePoint3D(-1.0, -1.0, -1.0),   // 在直线反向延长线上
            SimplePoint3D(3.0, 3.0, 3.0)       // 在直线延长线上
        };
        
        std::cout << "\n测试结果：" << std::endl;
        for (size_t i = 0; i < testPoints1.size(); ++i) {
            const SimplePoint3D& p = testPoints1[i];
            double t = line1.getParameter(p);
            double dist = line1.distanceToPoint(p);
            bool onLine = line1.isPointOnLine(p);
            
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
        
        SimplePoint3D p1(0.0, 0.0, 0.0);
        SimplePoint3D p2(1.0, 0.0, 0.0);  // X轴方向
        
        // 计算方向向量
        SimplePoint3D dirVec = p2 - p1;
        SimpleLine3D line2(p1, dirVec);
        
        printLine(line2, "直线L2 (X轴)");
        
        // 测试点
        SimplePoint3D testPoint2(3.0, 2.0, 1.0);
        double t2 = line2.getParameter(testPoint2);
        double dist2 = line2.distanceToPoint(testPoint2);
        bool onLine2 = line2.isPointOnLine(testPoint2);
        
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
        SimplePoint3D computedPoint = line2.getPointAt(testT);
        std::cout << "参数值 t = " << testT << " 对应的点: ";
        printPoint(computedPoint, "");
        
        // 验证计算是否正确
        double actualT = line2.getParameter(computedPoint);
        std::cout << "验证：该点的参数值 t = " << std::fixed << std::setprecision(6) << actualT << std::endl;
        
        // 示例3：计算点到直线的投影点
        std::cout << "\n示例3：计算点到直线的投影点" << std::endl;
        
        SimplePoint3D testPoint3(2.0, 1.0, 0.0);  // 不在X轴上的点
        double t3 = line2.getParameter(testPoint3);
        SimplePoint3D projectionPoint = line2.getPointAt(t3);
        
        printPoint(testPoint3, "测试点");
        std::cout << "  参数值 t = " << std::fixed << std::setprecision(6) << t3 << std::endl;
        printPoint(projectionPoint, "投影点");
        std::cout << "  到直线距离 = " << std::fixed << std::setprecision(6) 
                  << testPoint3.distance(projectionPoint) << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "未知错误发生" << std::endl;
        return 1;
    }
    
    return 0;
}