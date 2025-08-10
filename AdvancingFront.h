#ifndef ADVANCING_FRONT_H
#define ADVANCING_FRONT_H

#include "Point2D.h"
#include "Edge.h"
#include "Triangle.h"
#include <vector>
#include <list>
#include <algorithm>
#include <memory>
#include <iostream>
#include <cmath>

class AdvancingFront {
private:
    std::vector<Point2D> points;           // 所有点的存储
    std::vector<Triangle> triangles;       // 生成的三角形
    std::list<Edge> front;                 // 当前前沿边列表
    
    double meshSize;                       // 期望的网格尺寸
    double minAngleThreshold;              // 最小角度阈值
    double maxAngleThreshold;              // 最大角度阈值
    int maxIterations;                     // 最大迭代次数
    
public:
    // 构造函数
    AdvancingFront(double meshSize = 1.0, double minAngle = 0.3, double maxAngle = 2.8) 
        : meshSize(meshSize), minAngleThreshold(minAngle), maxAngleThreshold(maxAngle), maxIterations(10000) {}
    
    // 设置边界点（按逆时针顺序）
    void setBoundary(const std::vector<Point2D>& boundaryPoints);
    
    // 生成网格
    bool generateMesh();
    
    // 获取结果
    const std::vector<Point2D>& getPoints() const { return points; }
    const std::vector<Triangle>& getTriangles() const { return triangles; }
    
    // 设置参数
    void setMeshSize(double size) { meshSize = size; }
    void setAngleThresholds(double minAngle, double maxAngle) {
        minAngleThreshold = minAngle;
        maxAngleThreshold = maxAngle;
    }
    void setMaxIterations(int iterations) { maxIterations = iterations; }
    
    // 输出网格信息
    void printMeshInfo() const;
    
    // 导出为简单格式
    void exportToFile(const std::string& filename) const;
    
private:
    // 初始化前沿边
    void initializeFront();
    
    // 处理单个前沿边
    bool processEdge(std::list<Edge>::iterator edgeIt);
    
    // 寻找最佳的新点位置
    Point2D findOptimalPoint(const Edge& edge) const;
    
    // 检查点是否可接受
    bool isPointAcceptable(const Point2D& point, const Edge& edge) const;
    
    // 检查点是否在域内
    bool isPointInDomain(const Point2D& point) const;
    
    // 检查点是否与现有点过近
    bool isPointTooClose(const Point2D& point) const;
    
    // 检查新三角形是否与现有三角形相交
    bool isTriangleIntersecting(const Triangle& newTriangle) const;
    
    // 在现有点中寻找合适的点
    Point2D* findExistingPoint(const Edge& edge) const;
    
    // 添加新点
    int addPoint(const Point2D& point);
    
    // 添加新三角形
    void addTriangle(Point2D* p1, Point2D* p2, Point2D* p3);
    
    // 更新前沿边
    void updateFront(std::list<Edge>::iterator processedEdge, Point2D* newPoint);
    
    // 移除无效的前沿边
    void removeInvalidFrontEdges();
    
    // 计算两点之间的理想距离
    double getIdealDistance(const Point2D& p1, const Point2D& p2) const;
    
    // 计算点的密度函数值
    double getDensityFunction(const Point2D& point) const;
    
    // 检查线段是否与边界相交
    bool isLineIntersectingBoundary(const Point2D& p1, const Point2D& p2) const;
    
    // 计算两条线段的交点
    bool getLineIntersection(const Point2D& p1, const Point2D& p2, 
                           const Point2D& p3, const Point2D& p4, 
                           Point2D& intersection) const;
};

#endif // ADVANCING_FRONT_H