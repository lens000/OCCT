#include "AdvancingFront.h"
#include <iostream>
#include <vector>
#include <cmath>

// 创建矩形边界
std::vector<Point2D> createRectangleBoundary(double width, double height, double centerX = 0.0, double centerY = 0.0) {
    std::vector<Point2D> boundary;
    
    double halfWidth = width / 2.0;
    double halfHeight = height / 2.0;
    
    boundary.push_back(Point2D(centerX - halfWidth, centerY - halfHeight));  // 左下
    boundary.push_back(Point2D(centerX + halfWidth, centerY - halfHeight));  // 右下
    boundary.push_back(Point2D(centerX + halfWidth, centerY + halfHeight));  // 右上
    boundary.push_back(Point2D(centerX - halfWidth, centerY + halfHeight));  // 左上
    
    return boundary;
}

// 创建圆形边界
std::vector<Point2D> createCircleBoundary(double radius, int numPoints = 32, double centerX = 0.0, double centerY = 0.0) {
    std::vector<Point2D> boundary;
    
    for (int i = 0; i < numPoints; ++i) {
        double angle = 2.0 * M_PI * i / numPoints;
        double x = centerX + radius * std::cos(angle);
        double y = centerY + radius * std::sin(angle);
        boundary.push_back(Point2D(x, y));
    }
    
    return boundary;
}

// 创建L形边界
std::vector<Point2D> createLShapeBoundary(double width, double height, double thickness) {
    std::vector<Point2D> boundary;
    
    // L形的外轮廓（逆时针）
    boundary.push_back(Point2D(0, 0));                    // 起点
    boundary.push_back(Point2D(width, 0));                // 右下
    boundary.push_back(Point2D(width, thickness));        // 右中
    boundary.push_back(Point2D(thickness, thickness));    // 中中
    boundary.push_back(Point2D(thickness, height));       // 中上
    boundary.push_back(Point2D(0, height));               // 左上
    
    return boundary;
}

// 创建带圆形洞的矩形
std::pair<std::vector<Point2D>, std::vector<Point2D>> createRectangleWithCircularHole(
    double rectWidth, double rectHeight, double holeRadius, double holeCenterX = 0.0, double holeCenterY = 0.0) {
    
    // 外边界（矩形，逆时针）
    std::vector<Point2D> outerBoundary = createRectangleBoundary(rectWidth, rectHeight);
    
    // 内边界（圆形洞，顺时针）
    std::vector<Point2D> hole;
    int numPoints = 16;
    for (int i = 0; i < numPoints; ++i) {
        double angle = -2.0 * M_PI * i / numPoints;  // 负角度表示顺时针
        double x = holeCenterX + holeRadius * std::cos(angle);
        double y = holeCenterY + holeRadius * std::sin(angle);
        hole.push_back(Point2D(x, y));
    }
    
    return std::make_pair(outerBoundary, hole);
}

// 创建带矩形洞的矩形
std::pair<std::vector<Point2D>, std::vector<Point2D>> createRectangleWithRectangularHole(
    double outerWidth, double outerHeight, double innerWidth, double innerHeight) {
    
    // 外边界（大矩形，逆时针）
    std::vector<Point2D> outerBoundary = createRectangleBoundary(outerWidth, outerHeight);
    
    // 内边界（小矩形洞，顺时针）
    std::vector<Point2D> hole;
    double halfInnerWidth = innerWidth / 2.0;
    double halfInnerHeight = innerHeight / 2.0;
    
    hole.push_back(Point2D(halfInnerWidth, halfInnerHeight));    // 右上
    hole.push_back(Point2D(halfInnerWidth, -halfInnerHeight));   // 右下
    hole.push_back(Point2D(-halfInnerWidth, -halfInnerHeight));  // 左下
    hole.push_back(Point2D(-halfInnerWidth, halfInnerHeight));   // 左上
    
    return std::make_pair(outerBoundary, hole);
}

// 创建带多个洞的复杂域
std::pair<std::vector<Point2D>, std::vector<std::vector<Point2D>>> createComplexDomainWithMultipleHoles() {
    // 外边界（大矩形）
    std::vector<Point2D> outerBoundary = createRectangleBoundary(6.0, 4.0);
    
    std::vector<std::vector<Point2D>> holes;
    
    // 第一个洞（圆形）
    std::vector<Point2D> hole1;
    int numPoints = 12;
    for (int i = 0; i < numPoints; ++i) {
        double angle = -2.0 * M_PI * i / numPoints;  // 顺时针
        double x = -1.5 + 0.8 * std::cos(angle);
        double y = 0.5 + 0.8 * std::sin(angle);
        hole1.push_back(Point2D(x, y));
    }
    holes.push_back(hole1);
    
    // 第二个洞（小矩形）
    std::vector<Point2D> hole2;
    hole2.push_back(Point2D(1.5, 0.5));
    hole2.push_back(Point2D(1.5, -0.5));
    hole2.push_back(Point2D(0.5, -0.5));
    hole2.push_back(Point2D(0.5, 0.5));
    holes.push_back(hole2);
    
    return std::make_pair(outerBoundary, holes);
}

void testRectangleMesh() {
    std::cout << "\n=== Testing Rectangle Mesh ===" << std::endl;
    
    AdvancingFront af(0.5);  // 网格尺寸 0.5
    
    // 创建 4x3 的矩形
    std::vector<Point2D> boundary = createRectangleBoundary(4.0, 3.0);
    
    af.setBoundary(boundary);
    
    std::cout << "Generating mesh for rectangle (4x3)..." << std::endl;
    if (af.generateMesh()) {
        af.printMeshInfo();
        af.exportToFile("rectangle_mesh.txt");
    } else {
        std::cout << "Failed to generate rectangle mesh!" << std::endl;
    }
}

void testCircleMesh() {
    std::cout << "\n=== Testing Circle Mesh ===" << std::endl;
    
    AdvancingFront af(0.3);  // 更精细的网格
    
    // 创建半径为 2 的圆
    std::vector<Point2D> boundary = createCircleBoundary(2.0, 24);
    
    af.setBoundary(boundary);
    
    std::cout << "Generating mesh for circle (radius=2)..." << std::endl;
    if (af.generateMesh()) {
        af.printMeshInfo();
        af.exportToFile("circle_mesh.txt");
    } else {
        std::cout << "Failed to generate circle mesh!" << std::endl;
    }
}

void testLShapeMesh() {
    std::cout << "\n=== Testing L-Shape Mesh ===" << std::endl;
    
    AdvancingFront af(0.4);
    
    // 创建 L 形
    std::vector<Point2D> boundary = createLShapeBoundary(3.0, 3.0, 1.0);
    
    af.setBoundary(boundary);
    
    std::cout << "Generating mesh for L-shape..." << std::endl;
    if (af.generateMesh()) {
        af.printMeshInfo();
        af.exportToFile("lshape_mesh.txt");
    } else {
        std::cout << "Failed to generate L-shape mesh!" << std::endl;
    }
}

void testCustomPolygon() {
    std::cout << "\n=== Testing Custom Polygon Mesh ===" << std::endl;
    
    AdvancingFront af(0.25);
    
    // 创建一个不规则五边形
    std::vector<Point2D> boundary = {
        Point2D(0, 0),
        Point2D(3, 0),
        Point2D(4, 2),
        Point2D(2, 3),
        Point2D(-1, 2)
    };
    
    af.setBoundary(boundary);
    
    std::cout << "Generating mesh for custom polygon..." << std::endl;
    if (af.generateMesh()) {
        af.printMeshInfo();
        af.exportToFile("polygon_mesh.txt");
    } else {
        std::cout << "Failed to generate polygon mesh!" << std::endl;
    }
}

void testRectangleWithCircularHole() {
    std::cout << "\n=== Testing Rectangle with Circular Hole ===" << std::endl;
    
    AdvancingFront af(0.4);
    
    // 创建4x3的矩形，中间有半径为0.8的圆形洞
    auto boundaryAndHole = createRectangleWithCircularHole(4.0, 3.0, 0.8);
    std::vector<Point2D> outerBoundary = boundaryAndHole.first;
    std::vector<Point2D> hole = boundaryAndHole.second;
    
    std::vector<std::vector<Point2D>> holes = {hole};
    af.setBoundaryWithHoles(outerBoundary, holes);
    
    std::cout << "Generating mesh for rectangle with circular hole..." << std::endl;
    if (af.generateMesh()) {
        af.printMeshInfo();
        af.exportToFile("rectangle_circular_hole_mesh.txt");
    } else {
        std::cout << "Failed to generate mesh with circular hole!" << std::endl;
    }
}

void testRectangleWithRectangularHole() {
    std::cout << "\n=== Testing Rectangle with Rectangular Hole ===" << std::endl;
    
    AdvancingFront af(0.3);
    
    // 创建4x3的矩形，中间有2x1.5的矩形洞
    auto boundaryAndHole = createRectangleWithRectangularHole(4.0, 3.0, 2.0, 1.5);
    std::vector<Point2D> outerBoundary = boundaryAndHole.first;
    std::vector<Point2D> hole = boundaryAndHole.second;
    
    std::vector<std::vector<Point2D>> holes = {hole};
    af.setBoundaryWithHoles(outerBoundary, holes);
    
    std::cout << "Generating mesh for rectangle with rectangular hole..." << std::endl;
    if (af.generateMesh()) {
        af.printMeshInfo();
        af.exportToFile("rectangle_rectangular_hole_mesh.txt");
    } else {
        std::cout << "Failed to generate mesh with rectangular hole!" << std::endl;
    }
}

void testComplexDomainWithMultipleHoles() {
    std::cout << "\n=== Testing Complex Domain with Multiple Holes ===" << std::endl;
    
    AdvancingFront af(0.35);
    
    // 创建复杂域，包含多个洞
    auto boundaryAndHoles = createComplexDomainWithMultipleHoles();
    std::vector<Point2D> outerBoundary = boundaryAndHoles.first;
    std::vector<std::vector<Point2D>> holes = boundaryAndHoles.second;
    
    af.setBoundaryWithHoles(outerBoundary, holes);
    
    std::cout << "Generating mesh for complex domain with multiple holes..." << std::endl;
    if (af.generateMesh()) {
        af.printMeshInfo();
        af.exportToFile("complex_multiple_holes_mesh.txt");
    } else {
        std::cout << "Failed to generate mesh for complex domain!" << std::endl;
    }
}

void testAddHoleFeature() {
    std::cout << "\n=== Testing Add Hole Feature ===" << std::endl;
    
    AdvancingFront af(0.4);
    
    // 首先设置外边界
    std::vector<Point2D> outerBoundary = createRectangleBoundary(5.0, 3.0);
    af.setBoundary(outerBoundary);
    
    // 然后添加洞
    auto boundaryAndHole1 = createRectangleWithCircularHole(1.0, 1.0, 0.4);  // 圆形洞
    std::vector<Point2D> hole1 = boundaryAndHole1.second;
    af.addHole(hole1);
    
    // 添加另一个洞
    std::vector<Point2D> hole2;
    hole2.push_back(Point2D(1.5, 0.5));
    hole2.push_back(Point2D(1.5, -0.5));
    hole2.push_back(Point2D(0.8, -0.5));
    hole2.push_back(Point2D(0.8, 0.5));
    af.addHole(hole2);
    
    // 重新初始化前沿边
    af.setBoundaryWithHoles(outerBoundary, {hole1, hole2});
    
    std::cout << "Generating mesh using addHole feature..." << std::endl;
    if (af.generateMesh()) {
        af.printMeshInfo();
        af.exportToFile("add_hole_feature_mesh.txt");
    } else {
        std::cout << "Failed to generate mesh using addHole feature!" << std::endl;
    }
}

void demonstrateParameters() {
    std::cout << "\n=== Demonstrating Parameter Effects ===" << std::endl;
    
    std::vector<Point2D> boundary = createRectangleBoundary(2.0, 2.0);
    
    // 测试不同的网格尺寸
    std::vector<double> meshSizes = {0.2, 0.5, 1.0};
    
    for (double meshSize : meshSizes) {
        std::cout << "\nTesting with mesh size: " << meshSize << std::endl;
        
        AdvancingFront af(meshSize);
        af.setBoundary(boundary);
        
        if (af.generateMesh()) {
            std::cout << "Results: " << af.getTriangles().size() << " triangles generated" << std::endl;
            
            std::string filename = "test_meshsize_" + std::to_string(meshSize) + ".txt";
            af.exportToFile(filename);
        }
    }
}

int main() {
    std::cout << "二维前沿推进网格生成算法演示" << std::endl;
    std::cout << "==============================" << std::endl;
    
    try {
        // 测试矩形网格
        testRectangleMesh();
        
        // 测试圆形网格
        testCircleMesh();
        
        // 测试 L 形网格
        testLShapeMesh();
        
        // 测试自定义多边形
        testCustomPolygon();
        
        // 测试带洞的域
        testRectangleWithCircularHole();
        testRectangleWithRectangularHole();
        testComplexDomainWithMultipleHoles();
        testAddHoleFeature();
        
        // 演示参数效果
        demonstrateParameters();
        
        std::cout << "\n=== 演示完成 ===" << std::endl;
        std::cout << "所有网格文件已导出到当前目录" << std::endl;
        std::cout << "文件格式：" << std::endl;
        std::cout << "POINTS <数量>" << std::endl;
        std::cout << "<x1> <y1>" << std::endl;
        std::cout << "..." << std::endl;
        std::cout << "TRIANGLES <数量>" << std::endl;
        std::cout << "<点索引1> <点索引2> <点索引3>" << std::endl;
        std::cout << "..." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}