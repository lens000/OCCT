#ifndef POINT2D_H
#define POINT2D_H

#include <cmath>
#include <iostream>

class Point2D {
public:
    double x, y;
    int id;
    
    // 构造函数
    Point2D() : x(0.0), y(0.0), id(-1) {}
    Point2D(double x, double y, int id = -1) : x(x), y(y), id(id) {}
    
    // 计算两点之间的距离
    double distance(const Point2D& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    // 计算两点之间的距离的平方
    double distanceSquared(const Point2D& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return dx * dx + dy * dy;
    }
    
    // 向量运算
    Point2D operator+(const Point2D& other) const {
        return Point2D(x + other.x, y + other.y);
    }
    
    Point2D operator-(const Point2D& other) const {
        return Point2D(x - other.x, y - other.y);
    }
    
    Point2D operator*(double scalar) const {
        return Point2D(x * scalar, y * scalar);
    }
    
    // 点积
    double dot(const Point2D& other) const {
        return x * other.x + y * other.y;
    }
    
    // 叉积（返回标量，表示z分量）
    double cross(const Point2D& other) const {
        return x * other.y - y * other.x;
    }
    
    // 向量长度
    double length() const {
        return std::sqrt(x * x + y * y);
    }
    
    // 单位化
    Point2D normalize() const {
        double len = length();
        if (len > 1e-10) {
            return Point2D(x / len, y / len);
        }
        return Point2D(0, 0);
    }
    
    // 等于操作符
    bool operator==(const Point2D& other) const {
        const double eps = 1e-10;
        return std::abs(x - other.x) < eps && std::abs(y - other.y) < eps;
    }
    
    // 输出操作符
    friend std::ostream& operator<<(std::ostream& os, const Point2D& p) {
        os << "(" << p.x << ", " << p.y << ")";
        return os;
    }
};

#endif // POINT2D_H