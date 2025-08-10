#include "AdvancingFront.h"
#include <fstream>
#include <cstring>

void AdvancingFront::setBoundary(const std::vector<Point2D>& boundaryPoints) {
    points.clear();
    triangles.clear();
    front.clear();
    holes.clear();
    
    // 设置外边界
    outerBoundary = boundaryPoints;
    numBoundaryPoints = boundaryPoints.size();
    
    // 添加边界点
    for (const auto& point : boundaryPoints) {
        points.push_back(point);
        points.back().id = points.size() - 1;
    }
    
    // 初始化前沿边
    initializeFront();
}

void AdvancingFront::setBoundaryWithHoles(const std::vector<Point2D>& outerBoundaryPoints, 
                                           const std::vector<std::vector<Point2D>>& holesList) {
    points.clear();
    triangles.clear();
    front.clear();
    
    // 设置外边界
    outerBoundary = outerBoundaryPoints;
    holes = holesList;
    
    // 计算总边界点数
    numBoundaryPoints = outerBoundaryPoints.size();
    for (const auto& hole : holes) {
        numBoundaryPoints += hole.size();
    }
    
    // 添加外边界点
    for (const auto& point : outerBoundaryPoints) {
        points.push_back(point);
        points.back().id = points.size() - 1;
    }
    
    // 添加洞的边界点
    for (const auto& hole : holes) {
        for (const auto& point : hole) {
            points.push_back(point);
            points.back().id = points.size() - 1;
        }
    }
    
    // 初始化前沿边
    initializeFront();
}

void AdvancingFront::addHole(const std::vector<Point2D>& hole) {
    if (hole.size() < 3) {
        std::cerr << "Warning: Hole must have at least 3 points" << std::endl;
        return;
    }
    
    holes.push_back(hole);
    numBoundaryPoints += hole.size();
    
    // 添加洞的边界点到点集合
    for (const auto& point : hole) {
        points.push_back(point);
        points.back().id = points.size() - 1;
    }
}

void AdvancingFront::initializeFront() {
    front.clear();
    
    // 创建外边界的前沿边（逆时针）
    for (size_t i = 0; i < outerBoundary.size(); ++i) {
        size_t next = (i + 1) % outerBoundary.size();
        Edge edge(&points[i], &points[next]);
        front.push_back(edge);
    }
    
    // 创建洞边界的前沿边（顺时针，内向外）
    size_t pointOffset = outerBoundary.size();
    for (const auto& hole : holes) {
        for (size_t i = 0; i < hole.size(); ++i) {
            size_t current = pointOffset + i;
            size_t next = pointOffset + (i + 1) % hole.size();
            // 注意：洞的边界应该是顺时针的，这样前沿边指向域内部
            Edge edge(&points[next], &points[current]);  // 反向创建边
            front.push_back(edge);
        }
        pointOffset += hole.size();
    }
}

bool AdvancingFront::generateMesh() {
    if (points.size() < 3) {
        std::cerr << "Error: Need at least 3 boundary points" << std::endl;
        return false;
    }
    
    int iterations = 0;
    
    while (!front.empty() && iterations < maxIterations) {
        iterations++;
        
        // 寻找最短的前沿边进行处理
        auto shortestEdge = std::min_element(front.begin(), front.end(),
            [](const Edge& a, const Edge& b) {
                return a.length() < b.length();
            });
        
        if (shortestEdge == front.end()) break;
        
        // 处理选中的边
        if (!processEdge(shortestEdge)) {
            // 如果处理失败，移除这条边
            front.erase(shortestEdge);
        }
        
        // 定期清理无效的前沿边
        if (iterations % 100 == 0) {
            removeInvalidFrontEdges();
        }
    }
    
    std::cout << "Mesh generation completed after " << iterations << " iterations." << std::endl;
    std::cout << "Generated " << triangles.size() << " triangles." << std::endl;
    
    return true;
}

bool AdvancingFront::processEdge(std::list<Edge>::iterator edgeIt) {
    const Edge& edge = *edgeIt;
    
    if (!edge.p1 || !edge.p2 || !edge.isActive) {
        return false;
    }
    
    // 首先尝试在现有点中找到合适的点
    Point2D* existingPoint = findExistingPoint(edge);
    
    Point2D* newPoint = nullptr;
    
    if (existingPoint) {
        newPoint = existingPoint;
    } else {
        // 寻找最佳的新点位置
        Point2D optimalPoint = findOptimalPoint(edge);
        
        if (!isPointAcceptable(optimalPoint, edge)) {
            return false;
        }
        
        // 添加新点
        int pointIndex = addPoint(optimalPoint);
        newPoint = &points[pointIndex];
    }
    
    // 创建新三角形
    addTriangle(edge.p1, edge.p2, newPoint);
    
    // 更新前沿边
    updateFront(edgeIt, newPoint);
    
    return true;
}

Point2D AdvancingFront::findOptimalPoint(const Edge& edge) const {
    if (!edge.p1 || !edge.p2) return Point2D();
    
    // 计算边的中点和法向量
    Point2D midpoint = edge.midpoint();
    Point2D normal = edge.normal();
    
    // 计算理想的三角形高度
    double edgeLength = edge.length();
    double idealHeight = edgeLength * std::sqrt(3.0) / 2.0;  // 等边三角形的高
    
    // 根据网格尺寸调整高度
    double actualHeight = std::min(idealHeight, meshSize);
    
    // 候选点位置
    Point2D candidate = midpoint + normal * actualHeight;
    
    return candidate;
}

bool AdvancingFront::isPointAcceptable(const Point2D& point, const Edge& edge) const {
    // 检查点是否在域内
    if (!isPointInDomain(point)) {
        return false;
    }
    
    // 检查点是否与现有点过近
    if (isPointTooClose(point)) {
        return false;
    }
    
    // 检查新三角形的质量
    Triangle testTriangle(edge.p1, edge.p2, const_cast<Point2D*>(&point));
    
    if (!testTriangle.isValid()) {
        return false;
    }
    
    // 检查角度约束
    double minAngle = testTriangle.minAngle();
    if (minAngle < minAngleThreshold) {
        return false;
    }
    
    // 检查是否与现有三角形相交
    if (isTriangleIntersecting(testTriangle)) {
        return false;
    }
    
    return true;
}

bool AdvancingFront::isPointInDomain(const Point2D& point) const {
    // 检查点是否在外边界内
    bool inOuter = isPointInPolygon(point, outerBoundary);
    if (!inOuter) {
        return false;  // 不在外边界内
    }
    
    // 检查点是否在任何洞内
    for (const auto& hole : holes) {
        if (isPointInPolygon(point, hole)) {
            return false;  // 在洞内，不在有效域内
        }
    }
    
    return true;  // 在外边界内且不在任何洞内
}

bool AdvancingFront::isPointInPolygon(const Point2D& point, const std::vector<Point2D>& polygon) const {
    if (polygon.size() < 3) return false;
    
    // 使用射线法检查点是否在多边形内
    int intersections = 0;
    Point2D rayEnd(point.x + 1000.0, point.y);  // 水平射线
    
    for (size_t i = 0; i < polygon.size(); ++i) {
        size_t next = (i + 1) % polygon.size();
        
        const Point2D& p1 = polygon[i];
        const Point2D& p2 = polygon[next];
        
        // 检查射线与边的交点
        if (((p1.y > point.y) != (p2.y > point.y)) &&
            (point.x < (p2.x - p1.x) * (point.y - p1.y) / (p2.y - p1.y) + p1.x)) {
            intersections++;
        }
    }
    
    return (intersections % 2) == 1;  // 奇数个交点表示在内部
}

bool AdvancingFront::isPointTooClose(const Point2D& point) const {
    double minDistance = meshSize * 0.5;  // 最小距离阈值
    
    for (const auto& existingPoint : points) {
        if (point.distance(existingPoint) < minDistance) {
            return true;
        }
    }
    
    return false;
}

bool AdvancingFront::isTriangleIntersecting(const Triangle& newTriangle) const {
    for (const auto& triangle : triangles) {
        // 简化的相交检测：检查三角形是否有重叠的顶点
        if ((newTriangle.p1 == triangle.p1 || newTriangle.p1 == triangle.p2 || newTriangle.p1 == triangle.p3) &&
            (newTriangle.p2 == triangle.p1 || newTriangle.p2 == triangle.p2 || newTriangle.p2 == triangle.p3)) {
            continue;  // 共享边的三角形是允许的
        }
        
        // 检查新三角形的重心是否在现有三角形内
        Point2D centroid = newTriangle.centroid();
        if (triangle.contains(centroid)) {
            return true;
        }
    }
    
    return false;
}

Point2D* AdvancingFront::findExistingPoint(const Edge& edge) const {
    if (!edge.p1 || !edge.p2) return nullptr;
    
    double bestScore = std::numeric_limits<double>::max();
    Point2D* bestPoint = nullptr;
    
    for (auto& point : const_cast<std::vector<Point2D>&>(points)) {
        // 跳过边的端点
        if (&point == edge.p1 || &point == edge.p2) continue;
        
        // 创建测试三角形
        Triangle testTriangle(edge.p1, edge.p2, &point);
        
        if (!testTriangle.isValid()) continue;
        
        // 检查角度质量
        double minAngle = testTriangle.minAngle();
        if (minAngle < minAngleThreshold) continue;
        
        // 检查点是否在合理的位置
        if (!edge.isPointOnLeft(point)) continue;
        
        // 计算评分（基于角度质量和距离）
        double score = 1.0 / minAngle + point.distance(edge.midpoint()) / meshSize;
        
        if (score < bestScore) {
            bestScore = score;
            bestPoint = &point;
        }
    }
    
    return bestPoint;
}

int AdvancingFront::addPoint(const Point2D& point) {
    points.push_back(point);
    points.back().id = points.size() - 1;
    return points.size() - 1;
}

void AdvancingFront::addTriangle(Point2D* p1, Point2D* p2, Point2D* p3) {
    Triangle triangle(p1, p2, p3);
    triangle.ensureCounterClockwise();
    
    if (triangle.isValid()) {
        triangles.push_back(triangle);
    }
}

void AdvancingFront::updateFront(std::list<Edge>::iterator processedEdge, Point2D* newPoint) {
    Edge& edge = *processedEdge;
    
    // 创建两条新的前沿边
    Edge newEdge1(edge.p1, newPoint);
    Edge newEdge2(newPoint, edge.p2);
    
    // 检查新边是否与现有前沿边重合
    bool addEdge1 = true, addEdge2 = true;
    
    for (auto it = front.begin(); it != front.end(); ++it) {
        if (it == processedEdge) continue;
        
        if ((it->p1 == newEdge1.p1 && it->p2 == newEdge1.p2) ||
            (it->p1 == newEdge1.p2 && it->p2 == newEdge1.p1)) {
            it->isActive = false;
            addEdge1 = false;
        }
        
        if ((it->p1 == newEdge2.p1 && it->p2 == newEdge2.p2) ||
            (it->p1 == newEdge2.p2 && it->p2 == newEdge2.p1)) {
            it->isActive = false;
            addEdge2 = false;
        }
    }
    
    // 移除处理过的边
    front.erase(processedEdge);
    
    // 添加新的前沿边
    if (addEdge1) {
        front.push_back(newEdge1);
    }
    if (addEdge2) {
        front.push_back(newEdge2);
    }
}

void AdvancingFront::removeInvalidFrontEdges() {
    front.remove_if([](const Edge& edge) {
        return !edge.isActive || !edge.p1 || !edge.p2;
    });
}

double AdvancingFront::getIdealDistance(const Point2D& p1, const Point2D& p2) const {
    // 基于局部密度函数计算理想距离
    Point2D midpoint = Point2D((p1.x + p2.x) / 2.0, (p1.y + p2.y) / 2.0);
    return meshSize * getDensityFunction(midpoint);
}

double AdvancingFront::getDensityFunction(const Point2D& point) const {
    // 简单的均匀密度函数
    return 1.0;
}

void AdvancingFront::printMeshInfo() const {
    std::cout << "\n=== Mesh Information ===" << std::endl;
    std::cout << "Number of points: " << points.size() << std::endl;
    std::cout << "Number of triangles: " << triangles.size() << std::endl;
    std::cout << "Number of active front edges: " << front.size() << std::endl;
    
    if (!triangles.empty()) {
        double minAngle = std::numeric_limits<double>::max();
        double maxAngle = 0.0;
        double avgAngle = 0.0;
        
        for (const auto& triangle : triangles) {
            double angle = triangle.minAngle();
            minAngle = std::min(minAngle, angle);
            maxAngle = std::max(maxAngle, angle);
            avgAngle += angle;
        }
        avgAngle /= triangles.size();
        
        std::cout << "Triangle quality:" << std::endl;
        std::cout << "  Min angle: " << minAngle * 180.0 / M_PI << " degrees" << std::endl;
        std::cout << "  Max angle: " << maxAngle * 180.0 / M_PI << " degrees" << std::endl;
        std::cout << "  Avg angle: " << avgAngle * 180.0 / M_PI << " degrees" << std::endl;
    }
}

void AdvancingFront::exportToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }
    
    // 写入点
    file << "POINTS " << points.size() << std::endl;
    for (const auto& point : points) {
        file << point.x << " " << point.y << std::endl;
    }
    
    // 写入三角形
    file << "TRIANGLES " << triangles.size() << std::endl;
    for (const auto& triangle : triangles) {
        file << triangle.p1->id << " " << triangle.p2->id << " " << triangle.p3->id << std::endl;
    }
    
    file.close();
    std::cout << "Mesh exported to " << filename << std::endl;
}