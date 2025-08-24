#ifndef WAVEFRONT_MESH_GENERATOR_H
#define WAVEFRONT_MESH_GENERATOR_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>

// 前向声明
struct Point2D;
struct Edge;
struct Triangle;
struct Vertex;

// 2D点结构
struct Point2D {
    double x, y;
    
    Point2D(double x = 0.0, double y = 0.0) : x(x), y(y) {}
    
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
    
    // 叉积（2D情况下返回标量）
    double cross(const Point2D& other) const {
        return x * other.y - y * other.x;
    }
    
    // 模长
    double magnitude() const {
        return std::sqrt(x * x + y * y);
    }
    
    // 平方模长
    double squareMagnitude() const {
        return x * x + y * y;
    }
    
    // 单位化
    Point2D normalized() const {
        double mag = magnitude();
        if (mag < 1e-10) return Point2D(0, 0);
        return *this * (1.0 / mag);
    }
    
    // 距离
    double distance(const Point2D& other) const {
        return (*this - other).magnitude();
    }
    
    // 平方距离
    double squareDistance(const Point2D& other) const {
        return (*this - other).squareMagnitude();
    }
};

// 顶点结构
struct Vertex {
    Point2D coord;
    int id;
    bool isBoundary;
    
    Vertex(const Point2D& coord = Point2D(), int id = -1, bool isBoundary = false)
        : coord(coord), id(id), isBoundary(isBoundary) {}
};

// 边结构
struct Edge {
    int firstNode;
    int lastNode;
    int id;
    bool isBoundary;
    bool isProcessed;
    
    Edge(int first = -1, int last = -1, int id = -1, bool isBoundary = false)
        : firstNode(first), lastNode(last), id(id), isBoundary(isBoundary), isProcessed(false) {}
    
    // 获取另一个节点
    int getOtherNode(int nodeId) const {
        if (nodeId == firstNode) return lastNode;
        if (nodeId == lastNode) return firstNode;
        return -1;
    }
    
    // 检查是否包含指定节点
    bool containsNode(int nodeId) const {
        return nodeId == firstNode || nodeId == lastNode;
    }
};

// 三角形结构
struct Triangle {
    int nodes[3];
    int edges[3];
    int id;
    
    Triangle(int n1 = -1, int n2 = -1, int n3 = -1, int id = -1) : id(id) {
        nodes[0] = n1;
        nodes[1] = n2;
        nodes[2] = n3;
        edges[0] = edges[1] = edges[2] = -1;
    }
    
    // 检查是否包含指定节点
    bool containsNode(int nodeId) const {
        return nodes[0] == nodeId || nodes[1] == nodeId || nodes[2] == nodeId;
    }
    
    // 获取节点索引
    int getNodeIndex(int nodeId) const {
        for (int i = 0; i < 3; ++i) {
            if (nodes[i] == nodeId) return i;
        }
        return -1;
    }
};

// 边界框结构
struct BoundingBox {
    Point2D minPoint, maxPoint;
    
    BoundingBox() : minPoint(Point2D(1e10, 1e10)), maxPoint(Point2D(-1e10, -1e10)) {}
    
    BoundingBox(const Point2D& min, const Point2D& max) : minPoint(min), maxPoint(max) {}
    
    // 添加点
    void addPoint(const Point2D& point) {
        minPoint.x = std::min(minPoint.x, point.x);
        minPoint.y = std::min(minPoint.y, point.y);
        maxPoint.x = std::max(maxPoint.x, point.x);
        maxPoint.y = std::max(maxPoint.y, point.y);
    }
    
    // 添加边
    void addEdge(const Point2D& p1, const Point2D& p2) {
        addPoint(p1);
        addPoint(p2);
    }
    
    // 检查是否与另一个边界框重叠
    bool overlaps(const BoundingBox& other) const {
        return !(maxPoint.x < other.minPoint.x || minPoint.x > other.maxPoint.x ||
                 maxPoint.y < other.minPoint.y || minPoint.y > other.maxPoint.y);
    }
    
    // 检查点是否在边界框内
    bool contains(const Point2D& point) const {
        return point.x >= minPoint.x && point.x <= maxPoint.x &&
               point.y >= minPoint.y && point.y <= maxPoint.y;
    }
};

// 波前法网格生成器
class WavefrontMeshGenerator {
public:
    WavefrontMeshGenerator();
    ~WavefrontMeshGenerator();
    
    // 设置边界点
    void setBoundaryPoints(const std::vector<Point2D>& boundaryPoints);
    
    // 添加内部点
    void addInternalPoints(const std::vector<Point2D>& internalPoints);
    
    // 生成网格
    bool generateMesh();
    
    // 获取生成的网格
    const std::vector<Triangle>& getTriangles() const { return triangles; }
    const std::vector<Edge>& getEdges() const { return edges; }
    const std::vector<Vertex>& getVertices() const { return vertices; }
    
    // 网格质量统计
    void printMeshStatistics() const;
    
    // 导出为PLY格式
    bool exportToPLY(const std::string& filename) const;

private:
    // 数据结构
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::vector<Triangle> triangles;
    
    // 辅助数据结构
    std::unordered_map<int, std::vector<int>> nodeToEdges;  // 节点到边的映射
    std::unordered_map<int, std::vector<int>> nodeToTriangles;  // 节点到三角形的映射
    std::vector<int> boundaryEdges;  // 边界边列表
    
    // 波前法核心函数
    bool buildWavefront();
    bool advanceFront();
    int findNextEdge(int currentNode, int prevEdge, const Point2D& direction);
    bool createTriangle(int edge1, int edge2, int edge3);
    bool checkIntersection(const Edge& edge1, const Edge& edge2);
    bool checkIntersection(const Point2D& p1, const Point2D& p2, const Point2D& p3, const Point2D& p4);
    
    // 几何计算函数
    double calculateAngle(const Point2D& center, const Point2D& p1, const Point2D& p2);
    double calculateTriangleArea(const Point2D& p1, const Point2D& p2, const Point2D& p3);
    bool isPointInTriangle(const Point2D& point, const Point2D& p1, const Point2D& p2, const Point2D& p3);
    bool isClockwise(const std::vector<Point2D>& polygon);
    
    // 网格优化函数
    void optimizeMesh();
    void removeSliverTriangles();
    void smoothMesh();
    
    // 辅助函数
    int addVertex(const Point2D& coord, bool isBoundary = false);
    int addEdge(int node1, int node2, bool isBoundary = false);
    int addTriangle(int n1, int n2, int n3);
    void updateConnectivity();
    bool isValidTriangle(const Triangle& triangle);
    
    // 常量
    static constexpr double EPSILON = 1e-10;
    static constexpr double MIN_ANGLE = 0.1;  // 最小角度（弧度）
    static constexpr double MIN_AREA = 1e-8;  // 最小面积
};

#endif // WAVEFRONT_MESH_GENERATOR_H