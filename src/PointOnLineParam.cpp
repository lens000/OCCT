#include <iostream>
#include <cmath>
#include <vector>

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

// 直线结构（参数方程：P = P0 + t * direction）
struct Line3D {
    Point3D point;      // 直线上的一点 P0
    Point3D direction;  // 方向向量（已归一化）
    
    Line3D(const Point3D& p, const Point3D& dir) : point(p) {
        direction = dir.normalize();
    }
    
    // 通过两点构造直线
    static Line3D fromTwoPoints(const Point3D& p1, const Point3D& p2) {
        Point3D dir = p2 - p1;
        return Line3D(p1, dir);
    }
    
    // 计算直线上距离点P最近的点
    Point3D closestPoint(const Point3D& P) const {
        Point3D v = P - point;
        double t = v.dot(direction);
        return point + direction * t;
    }
    
    // 计算点P在直线上的参数值t
    double getParameter(const Point3D& P) const {
        Point3D v = P - point;
        return v.dot(direction);
    }
    
    // 根据参数值t计算直线上对应的点
    Point3D getPointAt(double t) const {
        return point + direction * t;
    }
    
    // 计算点到直线的距离
    double distanceToPoint(const Point3D& P) const {
        Point3D closest = closestPoint(P);
        return (P - closest).length();
    }
};

// 打印点信息
void printPoint(const Point3D& p, const std::string& name = "") {
    if (!name.empty()) {
        std::cout << name << ": ";
    }
    std::cout << "(" << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
}

// 打印直线信息
void printLine(const Line3D& line, const std::string& name = "") {
    if (!name.empty()) {
        std::cout << name << ": " << std::endl;
    }
    std::cout << "  点: ";
    printPoint(line.point, "");
    std::cout << "  方向: ";
    printPoint(line.direction, "");
}

int main() {
    std::cout << "=== 计算点在直线上的参数值 ===" << std::endl;
    
    // 示例1：通过两点定义直线
    Point3D p1(0, 0, 0);
    Point3D p2(1, 1, 1);
    Line3D line1 = Line3D::fromTwoPoints(p1, p2);
    
    std::cout << "\n示例1：通过两点定义直线" << std::endl;
    printPoint(p1, "点P1");
    printPoint(p2, "点P2");
    printLine(line1, "直线L1");
    
    // 测试点
    Point3D testPoint1(2, 2, 2);
    Point3D testPoint2(0.5, 0.5, 0.5);
    Point3D testPoint3(1, 0, 0);
    
    std::cout << "\n测试结果：" << std::endl;
    
    // 计算testPoint1的参数值
    double t1 = line1.getParameter(testPoint1);
    Point3D closest1 = line1.closestPoint(testPoint1);
    double dist1 = line1.distanceToPoint(testPoint1);
    
    printPoint(testPoint1, "测试点1");
    std::cout << "  参数值 t = " << t1 << std::endl;
    std::cout << "  直线上最近点: ";
    printPoint(closest1, "");
    std::cout << "  到直线距离 = " << dist1 << std::endl;
    
    // 计算testPoint2的参数值
    double t2 = line1.getParameter(testPoint2);
    Point3D closest2 = line1.closestPoint(testPoint2);
    double dist2 = line1.distanceToPoint(testPoint2);
    
    printPoint(testPoint2, "测试点2");
    std::cout << "  参数值 t = " << t2 << std::endl;
    std::cout << "  直线上最近点: ";
    printPoint(closest2, "");
    std::cout << "  到直线距离 = " << dist2 << std::endl;
    
    // 计算testPoint3的参数值
    double t3 = line1.getParameter(testPoint3);
    Point3D closest3 = line1.closestPoint(testPoint3);
    double dist3 = line1.distanceToPoint(testPoint3);
    
    printPoint(testPoint3, "测试点3");
    std::cout << "  参数值 t = " << t3 << std::endl;
    std::cout << "  直线上最近点: ";
    printPoint(closest3, "");
    std::cout << "  到直线距离 = " << dist3 << std::endl;
    
    // 示例2：通过点和方向向量定义直线
    std::cout << "\n\n示例2：通过点和方向向量定义直线" << std::endl;
    Point3D origin(0, 0, 0);
    Point3D direction(1, 0, 0);  // X轴方向
    Line3D line2(origin, direction);
    
    printLine(line2, "直线L2 (X轴)");
    
    // 测试点
    Point3D testPoint4(3, 2, 1);
    double t4 = line2.getParameter(testPoint4);
    Point3D closest4 = line2.closestPoint(testPoint4);
    double dist4 = line2.distanceToPoint(testPoint4);
    
    printPoint(testPoint4, "测试点4");
    std::cout << "  参数值 t = " << t4 << std::endl;
    std::cout << "  直线上最近点: ";
    printPoint(closest4, "");
    std::cout << "  到直线距离 = " << dist4 << std::endl;
    
    // 验证：根据参数值计算点
    std::cout << "\n验证：根据参数值计算点" << std::endl;
    double testT = 2.5;
    Point3D computedPoint = line2.getPointAt(testT);
    std::cout << "参数值 t = " << testT << " 对应的点: ";
    printPoint(computedPoint, "");
    
    return 0;
}