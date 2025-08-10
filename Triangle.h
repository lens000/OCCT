#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Point2D.h"
#include <algorithm>
#include <limits>

class Triangle {
public:
    Point2D* p1, * p2, * p3;  // 三角形的三个顶点
    
    // 构造函数
    Triangle() : p1(nullptr), p2(nullptr), p3(nullptr) {}
    Triangle(Point2D* point1, Point2D* point2, Point2D* point3) 
        : p1(point1), p2(point2), p3(point3) {}
    
    // 计算三角形面积
    double area() const {
        if (!p1 || !p2 || !p3) return 0.0;
        
        Point2D v1 = *p2 - *p1;
        Point2D v2 = *p3 - *p1;
        return std::abs(v1.cross(v2)) / 2.0;
    }
    
    // 计算三角形的外心
    Point2D circumcenter() const {
        if (!p1 || !p2 || !p3) return Point2D();
        
        double ax = p1->x, ay = p1->y;
        double bx = p2->x, by = p2->y;
        double cx = p3->x, cy = p3->y;
        
        double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
        
        if (std::abs(d) < 1e-10) {
            // 三点共线或近似共线
            return Point2D();
        }
        
        double ux = ((ax * ax + ay * ay) * (by - cy) + 
                     (bx * bx + by * by) * (cy - ay) + 
                     (cx * cx + cy * cy) * (ay - by)) / d;
        double uy = ((ax * ax + ay * ay) * (cx - bx) + 
                     (bx * bx + by * by) * (ax - cx) + 
                     (cx * cx + cy * cy) * (bx - ax)) / d;
        
        return Point2D(ux, uy);
    }
    
    // 计算外接圆半径
    double circumradius() const {
        if (!p1 || !p2 || !p3) return std::numeric_limits<double>::max();
        
        Point2D center = circumcenter();
        return p1->distance(center);
    }
    
    // 检查点是否在三角形内（包括边界）
    bool contains(const Point2D& point) const {
        if (!p1 || !p2 || !p3) return false;
        
        // 使用重心坐标
        Point2D v0 = *p3 - *p1;
        Point2D v1 = *p2 - *p1;
        Point2D v2 = point - *p1;
        
        double dot00 = v0.dot(v0);
        double dot01 = v0.dot(v1);
        double dot02 = v0.dot(v2);
        double dot11 = v1.dot(v1);
        double dot12 = v1.dot(v2);
        
        double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
        double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
        
        return (u >= 0) && (v >= 0) && (u + v <= 1);
    }
    
    // 检查点是否在外接圆内
    bool inCircumcircle(const Point2D& point) const {
        if (!p1 || !p2 || !p3) return false;
        
        Point2D center = circumcenter();
        double radius = circumradius();
        return point.distance(center) < radius;
    }
    
    // 检查三角形的方向（顺时针或逆时针）
    bool isCounterClockwise() const {
        if (!p1 || !p2 || !p3) return false;
        
        Point2D v1 = *p2 - *p1;
        Point2D v2 = *p3 - *p1;
        return v1.cross(v2) > 0;
    }
    
    // 确保三角形为逆时针方向
    void ensureCounterClockwise() {
        if (!isCounterClockwise()) {
            std::swap(p2, p3);
        }
    }
    
    // 获取三角形的质心
    Point2D centroid() const {
        if (!p1 || !p2 || !p3) return Point2D();
        
        return Point2D((p1->x + p2->x + p3->x) / 3.0, 
                       (p1->y + p2->y + p3->y) / 3.0);
    }
    
    // 计算三角形的最小角度
    double minAngle() const {
        if (!p1 || !p2 || !p3) return 0.0;
        
        double a = p2->distance(*p3);  // 边a (对应角A)
        double b = p1->distance(*p3);  // 边b (对应角B)
        double c = p1->distance(*p2);  // 边c (对应角C)
        
        // 使用余弦定理计算角度
        double cosA = (b * b + c * c - a * a) / (2.0 * b * c);
        double cosB = (a * a + c * c - b * b) / (2.0 * a * c);
        double cosC = (a * a + b * b - c * c) / (2.0 * a * b);
        
        double angleA = std::acos(std::max(-1.0, std::min(1.0, cosA)));
        double angleB = std::acos(std::max(-1.0, std::min(1.0, cosB)));
        double angleC = std::acos(std::max(-1.0, std::min(1.0, cosC)));
        
        return std::min({angleA, angleB, angleC});
    }
    
    // 检查三角形是否有效（非退化）
    bool isValid() const {
        if (!p1 || !p2 || !p3) return false;
        
        double area_val = area();
        return area_val > 1e-10;  // 面积足够大
    }
    
    // 输出操作符
    friend std::ostream& operator<<(std::ostream& os, const Triangle& t) {
        if (t.p1 && t.p2 && t.p3) {
            os << "Triangle[" << *t.p1 << ", " << *t.p2 << ", " << *t.p3 << "]";
        } else {
            os << "Triangle[null]";
        }
        return os;
    }
};

#endif // TRIANGLE_H