#include <iostream>

// OCCT核心几何类
// OCCT core geometry classes
#include <gp_Pnt.hxx>
#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>

// OCCT拓扑类
// OCCT topology classes
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>

// OCCT构建API
// OCCT builder API
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>

// OCCT工具类
// OCCT utility classes
#include <TopTools_ListOfShape.hxx>
#include <BRepBuilderAPI_WireError.hxx>
#include <BRepLib.hxx>

/**
 * 简单示例：将边转换成首尾相连的wire
 * Simple example: Convert edges into connected wire
 */
TopoDS_Wire CreateConnectedWire()
{
    // 1. 创建一系列首尾相连的边
    // Create a series of connected edges (forming a rectangle)
    gp_Pnt p1(0.0, 0.0, 0.0);   // 起点
    gp_Pnt p2(10.0, 0.0, 0.0);  // 第一条边的终点，第二条边的起点
    gp_Pnt p3(10.0, 10.0, 0.0); // 第二条边的终点，第三条边的起点
    gp_Pnt p4(0.0, 10.0, 0.0);  // 第三条边的终点，第四条边的起点
    // 第四条边从p4回到p1，形成闭合
    
    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
    TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(p3, p4);
    TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(p4, p1);
    
    // 2. 使用BRepBuilderAPI_MakeWire将边转换成wire
    // Use BRepBuilderAPI_MakeWire to convert edges to wire
    BRepBuilderAPI_MakeWire wireBuilder;
    
    // 逐个添加边
    // Add edges one by one
    wireBuilder.Add(edge1);
    wireBuilder.Add(edge2);
    wireBuilder.Add(edge3);
    wireBuilder.Add(edge4);
    
    // 3. 检查是否成功创建wire
    // Check if wire creation was successful
    if (wireBuilder.IsDone()) {
        std::cout << "✓ Wire created successfully!" << std::endl;
        
        // 获取创建的wire
        // Get the created wire
        TopoDS_Wire wire = wireBuilder.Wire();
        
        // 构建3D曲线（推荐步骤）
        // Build 3D curves (recommended step)
        BRepLib::BuildCurves3d(wire);
        
        return wire;
    } else {
        // 处理错误
        // Handle errors
        BRepBuilderAPI_WireError error = wireBuilder.Error();
        std::cout << "✗ Failed to create wire. Error: ";
        
        switch (error) {
            case BRepBuilderAPI_EmptyWire:
                std::cout << "Empty wire" << std::endl;
                break;
            case BRepBuilderAPI_DisconnectedWire:
                std::cout << "Disconnected wire - edges are not connected" << std::endl;
                break;
            case BRepBuilderAPI_NonManifoldWire:
                std::cout << "Non-manifold wire" << std::endl;
                break;
            default:
                std::cout << "Unknown error" << std::endl;
                break;
        }
        
        return TopoDS_Wire(); // 返回空wire
    }
}

/**
 * 更简单的方法：一次性创建wire
 * Simpler method: Create wire in one step
 */
TopoDS_Wire CreateWireDirectly()
{
    // 创建边
    // Create edges
    gp_Pnt p1(20.0, 0.0, 0.0);
    gp_Pnt p2(30.0, 0.0, 0.0);
    gp_Pnt p3(30.0, 10.0, 0.0);
    
    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
    
    // 直接使用构造函数创建wire
    // Create wire directly using constructor
    BRepBuilderAPI_MakeWire wireBuilder(edge1, edge2);
    
    if (wireBuilder.IsDone()) {
        std::cout << "✓ Wire created directly from two edges!" << std::endl;
        return wireBuilder.Wire();
    } else {
        std::cout << "✗ Failed to create wire directly" << std::endl;
        return TopoDS_Wire();
    }
}

int main()
{
    std::cout << "OCCT边转换为Wire示例 / OCCT Edge to Wire Example" << std::endl;
    std::cout << "================================================" << std::endl;
    
    // 示例1：逐步添加边
    // Example 1: Add edges step by step
    std::cout << "\n1. 逐步添加边创建wire:" << std::endl;
    TopoDS_Wire wire1 = CreateConnectedWire();
    
    // 示例2：直接创建wire
    // Example 2: Create wire directly
    std::cout << "\n2. 直接创建wire:" << std::endl;
    TopoDS_Wire wire2 = CreateWireDirectly();
    
    std::cout << "\n=== 关键要点 / Key Points ===" << std::endl;
    std::cout << "• 边必须首尾相连（共享顶点）" << std::endl;
    std::cout << "• Edges must be connected end-to-end (share vertices)" << std::endl;
    std::cout << "• 使用BRepBuilderAPI_MakeWire类" << std::endl;
    std::cout << "• Use BRepBuilderAPI_MakeWire class" << std::endl;
    std::cout << "• 检查IsDone()状态" << std::endl;
    std::cout << "• Check IsDone() status" << std::endl;
    std::cout << "• 调用BRepLib::BuildCurves3d()构建3D曲线" << std::endl;
    std::cout << "• Call BRepLib::BuildCurves3d() to build 3D curves" << std::endl;
    
    return 0;
}

/*
快速编译命令 (Quick compile command):
==================================

g++ -std=c++11 simple_wire_example.cpp -lTKernel -lTKMath -lTKBRep -lTKTopAlgo -lTKG3d -lTKGeomBase -o simple_wire_example

核心API总结 (Core API Summary):
===============================

1. BRepBuilderAPI_MakeWire wireBuilder;           // 创建构建器
2. wireBuilder.Add(edge);                         // 添加边
3. if (wireBuilder.IsDone()) { ... }              // 检查状态
4. TopoDS_Wire wire = wireBuilder.Wire();         // 获取结果
5. BRepLib::BuildCurves3d(wire);                  // 构建3D曲线

*/