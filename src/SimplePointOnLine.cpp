#include <iostream>
#include <cmath>
#include <vector> // Added missing include for std::vector

// 3D点结构
struct Point3D {
    double x, y, z;
    Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    
    // 向量减法
    Point3D operator-(const Point3D& other) const {
        return Point3D(x - other.x, y - other.y, z - other.z);
    }
    
    // 向量加法
    Point3D operator+(const Point3D& other) const {
        return Point3D(x + other.x, y + other.y, z + other.z);
    }
    
    // 标量乘法
    Point3D operator*(double scalar) const {
        return Point3D(x * scalar, y * scalar, z * scalar);
    }
    
    // 点积
    double dot(const Point3D& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    // 向量长度
    double length() const {
        return sqrt(x * x + y * y + z * z);
    }
    
    // 归一化
    Point3D normalize() const {
        double len = length();
        if (len == 0) return Point3D();
        return Point3D(x / len, y / len, z / len);
    }
};

// 计算点在直线上的参数值
double getParameterOnLine(const Point3D& linePoint, const Point3D& lineDirection, const Point3D& testPoint) {
    Point3D normalizedDir = lineDirection.normalize();
    Point3D v = testPoint - linePoint;
    return v.dot(normalizedDir);
}

// 计算点到直线的距离
double distanceToLine(const Point3D& linePoint, const Point3D& lineDirection, const Point3D& testPoint) {
    Point3D normalizedDir = lineDirection.normalize();
    Point3D v = testPoint - linePoint;
    double t = v.dot(normalizedDir);
    Point3D closestPoint = linePoint + normalizedDir * t;
    return (testPoint - closestPoint).length();
}

int main() {
    std::cout << "=== 简化版：计算点在直线上的参数值 ===" << std::endl;
    
    // 定义直线：通过点(0,0,0)和方向(1,1,1)
    Point3D linePoint(0, 0, 0);
    Point3D lineDirection(1, 1, 1);
    
    std::cout << "直线定义：" << std::endl;
    std::cout << "  点: (" << linePoint.x << ", " << linePoint.y << ", " << linePoint.z << ")" << std::endl;
    std::cout << "  方向: (" << lineDirection.x << ", " << lineDirection.y << ", " << lineDirection.z << ")" << std::endl;
    
    // 测试点
    std::vector<Point3D> testPoints = {
        Point3D(2, 2, 2),      // 在直线上
        Point3D(0.5, 0.5, 0.5), // 在直线上
        Point3D(1, 0, 0),      // 不在直线上
        Point3D(-1, -1, -1),   // 在直线反向延长线上
        Point3D(3, 3, 3)       // 在直线延长线上
    };
    
    std::cout << "\n测试结果：" << std::endl;
    for (size_t i = 0; i < testPoints.size(); ++i) {
        const Point3D& p = testPoints[i];
        double t = getParameterOnLine(linePoint, lineDirection, p);
        double dist = distanceToLine(linePoint, lineDirection, p);
        
        std::cout << "点" << (i+1) << " (" << p.x << ", " << p.y << ", " << p.z << "):" << std::endl;
        std::cout << "  参数值 t = " << t << std::endl;
        std::cout << "  到直线距离 = " << dist << std::endl;
        
        if (dist < 1e-10) {
            std::cout << "  → 点在直线上" << std::endl;
        } else {
            std::cout << "  → 点不在直线上" << std::endl;
        }
        std::cout << std::endl;
    }
    
    return 0;
}