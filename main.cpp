#include "WavefrontMeshGenerator.h"
#include <iostream>
#include <vector>
#include <cmath>

// 生成圆形边界点
std::vector<Point2D> generateCircleBoundary(double centerX, double centerY, double radius, int numPoints) {
    std::vector<Point2D> boundaryPoints;
    boundaryPoints.reserve(numPoints);
    
    for (int i = 0; i < numPoints; ++i) {
        double angle = 2.0 * M_PI * i / numPoints;
        double x = centerX + radius * cos(angle);
        double y = centerY + radius * sin(angle);
        boundaryPoints.emplace_back(x, y);
    }
    
    return boundaryPoints;
}

// 生成矩形边界点
std::vector<Point2D> generateRectangleBoundary(double minX, double minY, double maxX, double maxY, int pointsPerSide) {
    std::vector<Point2D> boundaryPoints;
    boundaryPoints.reserve(pointsPerSide * 4);
    
    // 底边
    for (int i = 0; i < pointsPerSide; ++i) {
        double x = minX + (maxX - minX) * i / (pointsPerSide - 1);
        boundaryPoints.emplace_back(x, minY);
    }
    
    // 右边
    for (int i = 1; i < pointsPerSide; ++i) {
        double y = minY + (maxY - minY) * i / (pointsPerSide - 1);
        boundaryPoints.emplace_back(maxX, y);
    }
    
    // 顶边
    for (int i = 1; i < pointsPerSide; ++i) {
        double x = maxX - (maxX - minX) * i / (pointsPerSide - 1);
        boundaryPoints.emplace_back(x, maxY);
    }
    
    // 左边
    for (int i = 1; i < pointsPerSide - 1; ++i) {
        double y = maxY - (maxY - minY) * i / (pointsPerSide - 1);
        boundaryPoints.emplace_back(minX, y);
    }
    
    return boundaryPoints;
}

// 生成L形边界点
std::vector<Point2D> generateLShapeBoundary(double width, double height, double thickness, int pointsPerSide) {
    std::vector<Point2D> boundaryPoints;
    
    // 外边界
    std::vector<Point2D> outerBoundary = generateRectangleBoundary(0, 0, width, height, pointsPerSide);
    
    // 内边界（挖空部分）
    std::vector<Point2D> innerBoundary = generateRectangleBoundary(
        thickness, thickness, width - thickness, height - thickness, pointsPerSide);
    
    // 合并边界（外边界顺时针，内边界逆时针）
    boundaryPoints.insert(boundaryPoints.end(), outerBoundary.begin(), outerBoundary.end());
    
    // 反转内边界并添加
    std::reverse(innerBoundary.begin(), innerBoundary.end());
    boundaryPoints.insert(boundaryPoints.end(), innerBoundary.begin(), innerBoundary.end());
    
    return boundaryPoints;
}

// 生成内部点
std::vector<Point2D> generateInternalPoints(const std::vector<Point2D>& boundaryPoints, int numInternalPoints) {
    std::vector<Point2D> internalPoints;
    internalPoints.reserve(numInternalPoints);
    
    // 计算边界框
    double minX = boundaryPoints[0].x, maxX = boundaryPoints[0].x;
    double minY = boundaryPoints[0].y, maxY = boundaryPoints[0].y;
    
    for (const auto& point : boundaryPoints) {
        minX = std::min(minX, point.x);
        maxX = std::max(maxX, point.x);
        minY = std::min(minY, point.y);
        maxY = std::max(maxY, point.y);
    }
    
    // 生成随机内部点
    std::srand(42); // 固定种子以便重现结果
    
    for (int i = 0; i < numInternalPoints; ++i) {
        Point2D point;
        bool validPoint = false;
        int attempts = 0;
        
        while (!validPoint && attempts < 100) {
            point.x = minX + (maxX - minX) * (std::rand() % 1000) / 1000.0;
            point.y = minY + (maxY - minY) * (std::rand() % 1000) / 1000.0;
            
            // 简单的边界检查（这里可以改进为更复杂的点在多边形内的检查）
            validPoint = true;
            attempts++;
        }
        
        if (validPoint) {
            internalPoints.push_back(point);
        }
    }
    
    return internalPoints;
}

int main() {
    std::cout << "=== 波前法网格生成器演示 ===" << std::endl;
    
    // 创建网格生成器
    WavefrontMeshGenerator meshGenerator;
    
    // 示例1：圆形网格
    std::cout << "\n--- 示例1：圆形网格 ---" << std::endl;
    {
        std::vector<Point2D> circleBoundary = generateCircleBoundary(0, 0, 5.0, 20);
        std::vector<Point2D> internalPoints = generateInternalPoints(circleBoundary, 30);
        
        meshGenerator.setBoundaryPoints(circleBoundary);
        meshGenerator.addInternalPoints(internalPoints);
        
        if (meshGenerator.generateMesh()) {
            meshGenerator.printMeshStatistics();
            meshGenerator.exportToPLY("circle_mesh.ply");
        }
    }
    
    // 示例2：矩形网格
    std::cout << "\n--- 示例2：矩形网格 ---" << std::endl;
    {
        std::vector<Point2D> rectBoundary = generateRectangleBoundary(-3, -2, 3, 2, 8);
        std::vector<Point2D> internalPoints = generateInternalPoints(rectBoundary, 25);
        
        meshGenerator.setBoundaryPoints(rectBoundary);
        meshGenerator.addInternalPoints(internalPoints);
        
        if (meshGenerator.generateMesh()) {
            meshGenerator.printMeshStatistics();
            meshGenerator.exportToPLY("rectangle_mesh.ply");
        }
    }
    
    // 示例3：L形网格
    std::cout << "\n--- 示例3：L形网格 ---" << std::endl;
    {
        std::vector<Point2D> lShapeBoundary = generateLShapeBoundary(6, 4, 1, 6);
        std::vector<Point2D> internalPoints = generateInternalPoints(lShapeBoundary, 40);
        
        meshGenerator.setBoundaryPoints(lShapeBoundary);
        meshGenerator.addInternalPoints(internalPoints);
        
        if (meshGenerator.generateMesh()) {
            meshGenerator.printMeshStatistics();
            meshGenerator.exportToPLY("lshape_mesh.ply");
        }
    }
    
    // 示例4：自定义复杂形状
    std::cout << "\n--- 示例4：自定义复杂形状 ---" << std::endl;
    {
        std::vector<Point2D> customBoundary = {
            Point2D(0, 0),      // 左下角
            Point2D(2, 0),      // 右下角
            Point2D(2, 1),      // 右下内角
            Point2D(1, 1),      // 内角
            Point2D(1, 2),      // 内角
            Point2D(0, 2),      // 左上角
        };
        
        std::vector<Point2D> internalPoints = generateInternalPoints(customBoundary, 15);
        
        meshGenerator.setBoundaryPoints(customBoundary);
        meshGenerator.addInternalPoints(internalPoints);
        
        if (meshGenerator.generateMesh()) {
            meshGenerator.printMeshStatistics();
            meshGenerator.exportToPLY("custom_mesh.ply");
        }
    }
    
    std::cout << "\n=== 演示完成 ===" << std::endl;
    std::cout << "生成的PLY文件可以用MeshLab、Blender等软件查看" << std::endl;
    
    return 0;
}