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