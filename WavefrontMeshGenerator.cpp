#include "WavefrontMeshGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <stack>
#include <algorithm>
#include <numeric>

WavefrontMeshGenerator::WavefrontMeshGenerator() {
    // 构造函数
}

WavefrontMeshGenerator::~WavefrontMeshGenerator() {
    // 析构函数
}

void WavefrontMeshGenerator::setBoundaryPoints(const std::vector<Point2D>& boundaryPoints) {
    // 清空现有数据
    vertices.clear();
    edges.clear();
    triangles.clear();
    nodeToEdges.clear();
    nodeToTriangles.clear();
    boundaryEdges.clear();
    
    // 添加边界点
    for (const auto& point : boundaryPoints) {
        addVertex(point, true);
    }
    
    // 创建边界边
    for (size_t i = 0; i < vertices.size(); ++i) {
        int next = (i + 1) % vertices.size();
        int edgeId = addEdge(i, next, true);
        boundaryEdges.push_back(edgeId);
    }
    
    // 更新连接性
    updateConnectivity();
}

void WavefrontMeshGenerator::addInternalPoints(const std::vector<Point2D>& internalPoints) {
    for (const auto& point : internalPoints) {
        addVertex(point, false);
    }
    updateConnectivity();
}

bool WavefrontMeshGenerator::generateMesh() {
    std::cout << "开始生成网格..." << std::endl;
    
    // 1. 构建初始波前
    if (!buildWavefront()) {
        std::cerr << "构建波前失败" << std::endl;
        return false;
    }
    
    // 2. 推进波前
    if (!advanceFront()) {
        std::cerr << "推进波前失败" << std::endl;
        return false;
    }
    
    // 3. 优化网格
    optimizeMesh();
    
    std::cout << "网格生成完成！" << std::endl;
    return true;
}

bool WavefrontMeshGenerator::buildWavefront() {
    std::cout << "构建初始波前..." << std::endl;
    
    // 检查边界是否有效
    if (boundaryEdges.size() < 3) {
        std::cerr << "边界边数量不足" << std::endl;
        return false;
    }
    
    // 检查边界方向（确保是逆时针）
    std::vector<Point2D> boundaryPoints;
    for (int edgeId : boundaryEdges) {
        const Edge& edge = edges[edgeId];
        boundaryPoints.push_back(vertices[edge.firstNode].coord);
    }
    
    if (isClockwise(boundaryPoints)) {
        std::cout << "检测到顺时针边界，正在反转..." << std::endl;
        std::reverse(boundaryEdges.begin(), boundaryEdges.end());
    }
    
    std::cout << "初始波前构建完成，边界边数量: " << boundaryEdges.size() << std::endl;
    return true;
}

bool WavefrontMeshGenerator::advanceFront() {
    std::cout << "开始推进波前..." << std::endl;
    
    // 使用队列管理波前边
    std::queue<int> frontQueue;
    std::unordered_set<int> processedEdges;
    
    // 初始化波前队列
    for (int edgeId : boundaryEdges) {
        frontQueue.push(edgeId);
        processedEdges.insert(edgeId);
    }
    
    int iteration = 0;
    const int maxIterations = 10000; // 防止无限循环
    
    while (!frontQueue.empty() && iteration < maxIterations) {
        int currentEdgeId = frontQueue.front();
        frontQueue.pop();
        
        if (processedEdges.find(currentEdgeId) != processedEdges.end()) {
            continue;
        }
        
        const Edge& currentEdge = edges[currentEdgeId];
        if (currentEdge.isProcessed) {
            continue;
        }
        
        // 标记边为已处理
        edges[currentEdgeId].isProcessed = true;
        processedEdges.insert(currentEdgeId);
        
        // 尝试在当前边附近创建三角形
        if (tryCreateTriangle(currentEdgeId)) {
            // 成功创建三角形，添加新的边到波前
            addNewEdgesToFront(currentEdgeId, frontQueue);
        }
        
        iteration++;
    }
    
    if (iteration >= maxIterations) {
        std::cerr << "警告：达到最大迭代次数" << std::endl;
    }
    
    std::cout << "波前推进完成，迭代次数: " << iteration << std::endl;
    return true;
}

bool WavefrontMeshGenerator::tryCreateTriangle(int edgeId) {
    const Edge& edge = edges[edgeId];
    
    // 计算边的方向
    Point2D edgeDir = vertices[edge.lastNode].coord - vertices[edge.firstNode].coord;
    edgeDir = edgeDir.normalized();
    
    // 寻找最佳候选点
    int bestCandidate = -1;
    double bestAngle = -M_PI;
    double bestDistance = std::numeric_limits<double>::max();
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (i == edge.firstNode || i == edge.lastNode) {
            continue;
        }
        
        const Vertex& candidate = vertices[i];
        
        // 计算到边的距离
        Point2D toCandidate = candidate.coord - vertices[edge.firstNode].coord;
        double distance = toCandidate.magnitude();
        
        // 计算角度
        double angle = calculateAngle(vertices[edge.firstNode].coord, 
                                    vertices[edge.lastNode].coord, 
                                    candidate.coord);
        
        // 检查是否形成有效的三角形
        if (angle > MIN_ANGLE && angle < M_PI - MIN_ANGLE) {
            // 检查是否与现有边相交
            if (!checkTriangleIntersections(edge.firstNode, edge.lastNode, i)) {
                // 选择最佳候选（优先考虑角度，其次考虑距离）
                if (angle > bestAngle || (std::abs(angle - bestAngle) < EPSILON && distance < bestDistance)) {
                    bestAngle = angle;
                    bestDistance = distance;
                    bestCandidate = i;
                }
            }
        }
    }
    
    if (bestCandidate != -1) {
        // 创建三角形
        return createTriangle(edgeId, bestCandidate);
    }
    
    return false;
}

bool WavefrontMeshGenerator::createTriangle(int edgeId, int candidateNode) {
    const Edge& edge = edges[edgeId];
    
    // 创建新边
    int newEdge1 = addEdge(edge.firstNode, candidateNode, false);
    int newEdge2 = addEdge(candidateNode, edge.lastNode, false);
    
    // 创建三角形
    int triangleId = addTriangle(edge.firstNode, candidateNode, edge.lastNode);
    
    if (triangleId != -1) {
        // 更新三角形的边信息
        triangles[triangleId].edges[0] = edgeId;
        triangles[triangleId].edges[1] = newEdge1;
        triangles[triangleId].edges[2] = newEdge2;
        
        // 更新连接性
        updateConnectivity();
        
        std::cout << "创建三角形 " << triangleId << " 使用节点: " 
                  << edge.firstNode << ", " << candidateNode << ", " << edge.lastNode << std::endl;
        
        return true;
    }
    
    return false;
}

bool WavefrontMeshGenerator::checkTriangleIntersections(int n1, int n2, int n3) {
    // 检查新三角形的边是否与现有边相交
    std::vector<std::pair<int, int>> newEdges = {
        {n1, n2}, {n2, n3}, {n3, n1}
    };
    
    for (const auto& newEdge : newEdges) {
        for (const auto& existingEdge : edges) {
            if (existingEdge.id == -1) continue;
            
            // 跳过共享节点的边
            if (existingEdge.containsNode(newEdge.first) || 
                existingEdge.containsNode(newEdge.second)) {
                continue;
            }
            
            // 检查相交
            if (checkIntersection(vertices[newEdge.first].coord, vertices[newEdge.second].coord,
                                 vertices[existingEdge.firstNode].coord, vertices[existingEdge.lastNode].coord)) {
                return true; // 发现相交
            }
        }
    }
    
    return false;
}

void WavefrontMeshGenerator::addNewEdgesToFront(int edgeId, std::queue<int>& frontQueue) {
    const Edge& edge = edges[edgeId];
    
    // 找到包含这条边的三角形
    for (const auto& triangle : triangles) {
        if (triangle.id == -1) continue;
        
        // 检查三角形是否包含这条边
        for (int i = 0; i < 3; ++i) {
            if (triangle.edges[i] == edgeId) {
                // 添加其他两条边到波前
                for (int j = 0; j < 3; ++j) {
                    if (j != i && triangle.edges[j] != -1) {
                        frontQueue.push(triangle.edges[j]);
                    }
                }
                break;
            }
        }
    }
}

bool WavefrontMeshGenerator::checkIntersection(const Edge& edge1, const Edge& edge2) {
    return checkIntersection(vertices[edge1.firstNode].coord, vertices[edge1.lastNode].coord,
                            vertices[edge2.firstNode].coord, vertices[edge2.lastNode].coord);
}

bool WavefrontMeshGenerator::checkIntersection(const Point2D& p1, const Point2D& p2, 
                                              const Point2D& p3, const Point2D& p4) {
    // 使用叉积检查线段相交
    double d1 = (p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x);
    double d2 = (p4.x - p3.x) * (p2.y - p3.y) - (p4.y - p3.y) * (p2.x - p3.x);
    double d3 = (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
    double d4 = (p2.x - p1.x) * (p4.y - p1.y) - (p2.y - p1.y) * (p4.x - p1.x);
    
    // 检查是否相交
    if (d1 * d2 < 0 && d3 * d4 < 0) {
        return true;
    }
    
    return false;
}

double WavefrontMeshGenerator::calculateAngle(const Point2D& center, const Point2D& p1, const Point2D& p2) {
    Point2D v1 = p1 - center;
    Point2D v2 = p2 - center;
    
    double dot = v1.dot(v2);
    double cross = v1.cross(v2);
    
    return std::atan2(cross, dot);
}

double WavefrontMeshGenerator::calculateTriangleArea(const Point2D& p1, const Point2D& p2, const Point2D& p3) {
    return std::abs((p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y)) / 2.0;
}

bool WavefrontMeshGenerator::isPointInTriangle(const Point2D& point, const Point2D& p1, 
                                               const Point2D& p2, const Point2D& p3) {
    double area = calculateTriangleArea(p1, p2, p3);
    double area1 = calculateTriangleArea(point, p2, p3);
    double area2 = calculateTriangleArea(p1, point, p3);
    double area3 = calculateTriangleArea(p1, p2, point);
    
    return std::abs(area - (area1 + area2 + area3)) < EPSILON;
}

bool WavefrontMeshGenerator::isClockwise(const std::vector<Point2D>& polygon) {
    if (polygon.size() < 3) return false;
    
    double sum = 0.0;
    for (size_t i = 0; i < polygon.size(); ++i) {
        const Point2D& current = polygon[i];
        const Point2D& next = polygon[(i + 1) % polygon.size()];
        sum += (next.x - current.x) * (next.y + current.y);
    }
    
    return sum > 0;
}

void WavefrontMeshGenerator::optimizeMesh() {
    std::cout << "开始优化网格..." << std::endl;
    
    removeSliverTriangles();
    smoothMesh();
    
    std::cout << "网格优化完成" << std::endl;
}

void WavefrontMeshGenerator::removeSliverTriangles() {
    std::vector<int> trianglesToRemove;
    
    for (size_t i = 0; i < triangles.size(); ++i) {
        const Triangle& triangle = triangles[i];
        if (triangle.id == -1) continue;
        
        // 计算三角形面积
        const Point2D& p1 = vertices[triangle.nodes[0]].coord;
        const Point2D& p2 = vertices[triangle.nodes[1]].coord;
        const Point2D& p3 = vertices[triangle.nodes[2]].coord;
        
        double area = calculateTriangleArea(p1, p2, p3);
        
        // 检查最小角度
        double angle1 = std::abs(calculateAngle(p1, p2, p3));
        double angle2 = std::abs(calculateAngle(p2, p3, p1));
        double angle3 = std::abs(calculateAngle(p3, p1, p2));
        double minAngle = std::min({angle1, angle2, angle3});
        
        if (area < MIN_AREA || minAngle < MIN_ANGLE) {
            trianglesToRemove.push_back(i);
        }
    }
    
    // 移除低质量三角形
    for (int index : trianglesToRemove) {
        triangles[index].id = -1; // 标记为无效
    }
    
    std::cout << "移除了 " << trianglesToRemove.size() << " 个低质量三角形" << std::endl;
}

void WavefrontMeshGenerator::smoothMesh() {
    // 简单的Laplacian平滑
    std::vector<Point2D> newPositions = vertices.size();
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (vertices[i].isBoundary) {
            newPositions[i] = vertices[i].coord; // 边界点不移动
            continue;
        }
        
        // 计算相邻点的平均位置
        Point2D sum(0, 0);
        int count = 0;
        
        for (int edgeId : nodeToEdges[i]) {
            const Edge& edge = edges[edgeId];
            int neighbor = edge.getOtherNode(i);
            if (neighbor != -1) {
                sum = sum + vertices[neighbor].coord;
                count++;
            }
        }
        
        if (count > 0) {
            newPositions[i] = sum * (1.0 / count);
        } else {
            newPositions[i] = vertices[i].coord;
        }
    }
    
    // 应用新位置
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (!vertices[i].isBoundary) {
            vertices[i].coord = newPositions[i];
        }
    }
}

int WavefrontMeshGenerator::addVertex(const Point2D& coord, bool isBoundary) {
    int id = vertices.size();
    vertices.emplace_back(coord, id, isBoundary);
    return id;
}

int WavefrontMeshGenerator::addEdge(int node1, int node2, bool isBoundary) {
    int id = edges.size();
    edges.emplace_back(node1, node2, id, isBoundary);
    return id;
}

int WavefrontMeshGenerator::addTriangle(int n1, int n2, int n3) {
    int id = triangles.size();
    triangles.emplace_back(n1, n2, n3, id);
    return id;
}

void WavefrontMeshGenerator::updateConnectivity() {
    // 清空连接性映射
    nodeToEdges.clear();
    nodeToTriangles.clear();
    
    // 重建节点到边的映射
    for (const auto& edge : edges) {
        if (edge.id == -1) continue;
        
        nodeToEdges[edge.firstNode].push_back(edge.id);
        nodeToEdges[edge.lastNode].push_back(edge.id);
    }
    
    // 重建节点到三角形的映射
    for (const auto& triangle : triangles) {
        if (triangle.id == -1) continue;
        
        for (int i = 0; i < 3; ++i) {
            nodeToTriangles[triangle.nodes[i]].push_back(triangle.id);
        }
    }
}

bool WavefrontMeshGenerator::isValidTriangle(const Triangle& triangle) {
    if (triangle.id == -1) return false;
    
    // 检查节点是否有效
    for (int i = 0; i < 3; ++i) {
        if (triangle.nodes[i] < 0 || triangle.nodes[i] >= static_cast<int>(vertices.size())) {
            return false;
        }
    }
    
    // 检查面积
    const Point2D& p1 = vertices[triangle.nodes[0]].coord;
    const Point2D& p2 = vertices[triangle.nodes[1]].coord;
    const Point2D& p3 = vertices[triangle.nodes[2]].coord;
    
    double area = calculateTriangleArea(p1, p2, p3);
    if (area < MIN_AREA) return false;
    
    return true;
}

void WavefrontMeshGenerator::printMeshStatistics() const {
    std::cout << "\n=== 网格统计信息 ===" << std::endl;
    std::cout << "顶点数量: " << vertices.size() << std::endl;
    std::cout << "边数量: " << edges.size() << std::endl;
    std::cout << "三角形数量: " << triangles.size() << std::endl;
    
    // 统计边界点
    int boundaryVertices = 0;
    for (const auto& vertex : vertices) {
        if (vertex.isBoundary) boundaryVertices++;
    }
    std::cout << "边界顶点数量: " << boundaryVertices << std::endl;
    
    // 统计有效三角形
    int validTriangles = 0;
    double totalArea = 0.0;
    double minArea = std::numeric_limits<double>::max();
    double maxArea = 0.0;
    
    for (const auto& triangle : triangles) {
        if (triangle.id != -1) {
            validTriangles++;
            
            const Point2D& p1 = vertices[triangle.nodes[0]].coord;
            const Point2D& p2 = vertices[triangle.nodes[1]].coord;
            const Point2D& p3 = vertices[triangle.nodes[2]].coord;
            
            double area = calculateTriangleArea(p1, p2, p3);
            totalArea += area;
            minArea = std::min(minArea, area);
            maxArea = std::max(maxArea, area);
        }
    }
    
    std::cout << "有效三角形数量: " << validTriangles << std::endl;
    std::cout << "总面积: " << totalArea << std::endl;
    std::cout << "最小三角形面积: " << minArea << std::endl;
    std::cout << "最大三角形面积: " << maxArea << std::endl;
    std::cout << "平均三角形面积: " << (validTriangles > 0 ? totalArea / validTriangles : 0) << std::endl;
}

bool WavefrontMeshGenerator::exportToPLY(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }
    
    // 统计有效顶点和三角形
    std::vector<int> validVertexMap(vertices.size(), -1);
    int validVertexCount = 0;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (vertices[i].id != -1) {
            validVertexMap[i] = validVertexCount++;
        }
    }
    
    int validTriangleCount = 0;
    for (const auto& triangle : triangles) {
        if (triangle.id != -1) validTriangleCount++;
    }
    
    // 写入PLY头
    file << "ply" << std::endl;
    file << "format ascii 1.0" << std::endl;
    file << "element vertex " << validVertexCount << std::endl;
    file << "property float x" << std::endl;
    file << "property float y" << std::endl;
    file << "property float z" << std::endl;
    file << "element face " << validTriangleCount << std::endl;
    file << "property list uchar int vertex_indices" << std::endl;
    file << "end_header" << std::endl;
    
    // 写入顶点
    for (const auto& vertex : vertices) {
        if (vertex.id != -1) {
            file << vertex.coord.x << " " << vertex.coord.y << " 0.0" << std::endl;
        }
    }
    
    // 写入三角形
    for (const auto& triangle : triangles) {
        if (triangle.id != -1) {
            file << "3";
            for (int i = 0; i < 3; ++i) {
                file << " " << validVertexMap[triangle.nodes[i]];
            }
            file << std::endl;
        }
    }
    
    file.close();
    std::cout << "网格已导出到: " << filename << std::endl;
    return true;
}