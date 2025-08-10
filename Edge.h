#ifndef EDGE_H
#define EDGE_H

#include "Point2D.h"
#include <limits>

class Edge {
public:
    Point2D* p1;  // 边的起点
    Point2D* p2;  // 边的终点
    bool isActive; // 是否为活跃前沿边
    
    // 构造函数
    Edge() : p1(nullptr), p2(nullptr), isActive(true) {}
    Edge(Point2D* point1, Point2D* point2) : p1(point1), p2(point2), isActive(true) {}
    
    // 获取边的长度
    double length() const {
        if (p1 && p2) {
            return p1->distance(*p2);
        }
        return 0.0;
    }
    
    // 获取边的中点
    Point2D midpoint() const {
        if (p1 && p2) {
            return Point2D((p1->x + p2->x) / 2.0, (p1->y + p2->y) / 2.0);
        }
        return Point2D();
    }
    
    // 获取边的方向向量
    Point2D direction() const {
        if (p1 && p2) {
            return (*p2 - *p1).normalize();
        }
        return Point2D();
    }
    
    // 获取边的法向量（向左90度旋转）
    Point2D normal() const {
        Point2D dir = direction();
        return Point2D(-dir.y, dir.x);  // 左转90度
    }
    
    // 检查点是否在边的左侧
    bool isPointOnLeft(const Point2D& point) const {
        if (!p1 || !p2) return false;
        
        Point2D edge = *p2 - *p1;
        Point2D toPoint = point - *p1;
        return edge.cross(toPoint) > 0;
    }
    
    // 计算点到边的距离
    double distanceToPoint(const Point2D& point) const {
        if (!p1 || !p2) return std::numeric_limits<double>::max();
        
        Point2D edge = *p2 - *p1;
        Point2D toPoint = point - *p1;
        
        double edgeLength = edge.length();
        if (edgeLength < 1e-10) {
            return p1->distance(point);
        }
        
        double t = toPoint.dot(edge) / (edgeLength * edgeLength);
        t = std::max(0.0, std::min(1.0, t));
        
        Point2D closestPoint = *p1 + edge * t;
        return point.distance(closestPoint);
    }
    
    // 等于操作符
    bool operator==(const Edge& other) const {
        return (p1 == other.p1 && p2 == other.p2) || 
               (p1 == other.p2 && p2 == other.p1);
    }
    
    // 输出操作符
    friend std::ostream& operator<<(std::ostream& os, const Edge& e) {
        if (e.p1 && e.p2) {
            os << "Edge[" << *e.p1 << " -> " << *e.p2 << "]";
        } else {
            os << "Edge[null]";
        }
        return os;
    }
};

#endif // EDGE_H