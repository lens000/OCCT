#include <iostream>
#include <vector>

// OCCT includes for geometry
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>

// OCCT includes for topology
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>

// OCCT includes for building
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>

// OCCT includes for utilities
#include <TopTools_ListOfShape.hxx>
#include <BRepBuilderAPI_WireError.hxx>
#include <BRepLib.hxx>
#include <BRep_Tool.hxx>

/**
 * 演示如何使用OCCT将边转换成首尾相连的wire
 * Demonstrates how to convert edges into connected wires using OCCT
 */
class WireFromEdgesExample
{
public:
    /**
     * 方法1: 从单个边创建wire
     * Method 1: Create wire from a single edge
     */
    static TopoDS_Wire CreateWireFromSingleEdge()
    {
        std::cout << "=== Method 1: Creating wire from single edge ===" << std::endl;
        
        // 创建一个圆弧边
        // Create a circular arc edge
        gp_Pnt center(0.0, 0.0, 0.0);
        gp_Dir normal(0.0, 0.0, 1.0);
        gp_Dir xdir(1.0, 0.0, 0.0);
        gp_Ax2 axis(center, normal, xdir);
        gp_Circ circle(axis, 10.0);
        
        TopoDS_Edge circularEdge = BRepBuilderAPI_MakeEdge(circle);
        
        // 使用BRepBuilderAPI_MakeWire从单个边创建wire
        // Use BRepBuilderAPI_MakeWire to create wire from single edge
        BRepBuilderAPI_MakeWire wireBuilder(circularEdge);
        
        if (wireBuilder.IsDone()) {
            std::cout << "Wire created successfully from single circular edge" << std::endl;
            return wireBuilder.Wire();
        } else {
            std::cout << "Failed to create wire from single edge" << std::endl;
            return TopoDS_Wire();
        }
    }
    
    /**
     * 方法2: 从多个连接的边创建wire
     * Method 2: Create wire from multiple connected edges
     */
    static TopoDS_Wire CreateWireFromConnectedEdges()
    {
        std::cout << "\n=== Method 2: Creating wire from connected edges ===" << std::endl;
        
        // 创建一系列首尾相连的边
        // Create a series of connected edges
        gp_Pnt p1(0.0, 0.0, 0.0);
        gp_Pnt p2(10.0, 0.0, 0.0);
        gp_Pnt p3(10.0, 10.0, 0.0);
        gp_Pnt p4(0.0, 10.0, 0.0);
        
        TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
        TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
        TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(p3, p4);
        TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(p4, p1); // 闭合 Close the loop
        
        // 方法2a: 使用构造函数一次性添加多个边
        // Method 2a: Add multiple edges at once using constructor
        BRepBuilderAPI_MakeWire wireBuilder(edge1, edge2, edge3, edge4);
        
        if (wireBuilder.IsDone()) {
            std::cout << "Wire created successfully from 4 connected edges (constructor method)" << std::endl;
            return wireBuilder.Wire();
        } else {
            std::cout << "Failed to create wire from connected edges: " << GetErrorString(wireBuilder.Error()) << std::endl;
            return TopoDS_Wire();
        }
    }
    
    /**
     * 方法3: 逐步添加边创建wire
     * Method 3: Create wire by adding edges step by step
     */
    static TopoDS_Wire CreateWireByAddingEdges()
    {
        std::cout << "\n=== Method 3: Creating wire by adding edges step by step ===" << std::endl;
        
        // 创建边
        // Create edges
        gp_Pnt p1(20.0, 0.0, 0.0);
        gp_Pnt p2(25.0, 5.0, 0.0);
        gp_Pnt p3(20.0, 10.0, 0.0);
        gp_Pnt p4(15.0, 5.0, 0.0);
        
        TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
        TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
        TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(p3, p4);
        TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(p4, p1);
        
        // 创建空的wire构建器
        // Create empty wire builder
        BRepBuilderAPI_MakeWire wireBuilder;
        
        // 逐步添加边
        // Add edges step by step
        wireBuilder.Add(edge1);
        if (!wireBuilder.IsDone()) {
            std::cout << "Failed to add edge1: " << GetErrorString(wireBuilder.Error()) << std::endl;
            return TopoDS_Wire();
        }
        
        wireBuilder.Add(edge2);
        if (!wireBuilder.IsDone()) {
            std::cout << "Failed to add edge2: " << GetErrorString(wireBuilder.Error()) << std::endl;
            return TopoDS_Wire();
        }
        
        wireBuilder.Add(edge3);
        if (!wireBuilder.IsDone()) {
            std::cout << "Failed to add edge3: " << GetErrorString(wireBuilder.Error()) << std::endl;
            return TopoDS_Wire();
        }
        
        wireBuilder.Add(edge4);
        if (!wireBuilder.IsDone()) {
            std::cout << "Failed to add edge4: " << GetErrorString(wireBuilder.Error()) << std::endl;
            return TopoDS_Wire();
        }
        
        std::cout << "Wire created successfully by adding edges step by step" << std::endl;
        return wireBuilder.Wire();
    }
    
    /**
     * 方法4: 从边列表创建wire
     * Method 4: Create wire from a list of edges
     */
    static TopoDS_Wire CreateWireFromEdgeList()
    {
        std::cout << "\n=== Method 4: Creating wire from edge list ===" << std::endl;
        
        // 创建边列表
        // Create edge list
        TopTools_ListOfShape edgeList;
        
        // 创建一个椭圆的边，然后分割成多个边
        // Create elliptical edges
        gp_Pnt center(30.0, 0.0, 0.0);
        gp_Dir normal(0.0, 0.0, 1.0);
        gp_Dir xdir(1.0, 0.0, 0.0);
        gp_Ax2 axis(center, normal, xdir);
        gp_Elips ellipse(axis, 8.0, 5.0); // 长轴8, 短轴5
        
        // 创建椭圆的四个象限边
        // Create four quadrant edges of the ellipse
        TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(ellipse, 0.0, M_PI/2);
        TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(ellipse, M_PI/2, M_PI);
        TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(ellipse, M_PI, 3*M_PI/2);
        TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(ellipse, 3*M_PI/2, 2*M_PI);
        
        edgeList.Append(edge1);
        edgeList.Append(edge2);
        edgeList.Append(edge3);
        edgeList.Append(edge4);
        
        // 使用边列表创建wire
        // Create wire from edge list
        BRepBuilderAPI_MakeWire wireBuilder;
        wireBuilder.Add(edgeList);
        
        if (wireBuilder.IsDone()) {
            std::cout << "Wire created successfully from edge list" << std::endl;
            return wireBuilder.Wire();
        } else {
            std::cout << "Failed to create wire from edge list: " << GetErrorString(wireBuilder.Error()) << std::endl;
            return TopoDS_Wire();
        }
    }
    
    /**
     * 方法5: 处理不连接的边
     * Method 5: Handle disconnected edges
     */
    static TopoDS_Wire CreateWireFromDisconnectedEdges()
    {
        std::cout << "\n=== Method 5: Handling disconnected edges ===" << std::endl;
        
        // 创建不连接的边
        // Create disconnected edges
        gp_Pnt p1(40.0, 0.0, 0.0);
        gp_Pnt p2(45.0, 0.0, 0.0);
        gp_Pnt p3(50.0, 5.0, 0.0);  // 注意：这个点不与p2连接
        gp_Pnt p4(45.0, 10.0, 0.0);
        
        TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
        TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p3, p4); // 不连接的边
        
        BRepBuilderAPI_MakeWire wireBuilder;
        wireBuilder.Add(edge1);
        
        std::cout << "Adding first edge... ";
        if (wireBuilder.IsDone()) {
            std::cout << "Success" << std::endl;
        } else {
            std::cout << "Failed: " << GetErrorString(wireBuilder.Error()) << std::endl;
        }
        
        std::cout << "Adding disconnected edge... ";
        wireBuilder.Add(edge2);
        if (wireBuilder.IsDone()) {
            std::cout << "Success (unexpected!)" << std::endl;
            return wireBuilder.Wire();
        } else {
            std::cout << "Failed as expected: " << GetErrorString(wireBuilder.Error()) << std::endl;
            
            // 即使失败，我们仍然可以获得部分结果
            // Even if it fails, we can still get partial result
            try {
                TopoDS_Wire partialWire = wireBuilder.Wire();
                std::cout << "Partial wire with connected edges is still available" << std::endl;
                return partialWire;
            } catch (...) {
                std::cout << "No partial wire available" << std::endl;
                return TopoDS_Wire();
            }
        }
    }
    
    /**
     * 方法6: 合并多个wire
     * Method 6: Merge multiple wires
     */
    static TopoDS_Wire MergeWires()
    {
        std::cout << "\n=== Method 6: Merging multiple wires ===" << std::endl;
        
        // 创建第一个wire
        // Create first wire
        gp_Pnt p1(60.0, 0.0, 0.0);
        gp_Pnt p2(65.0, 0.0, 0.0);
        gp_Pnt p3(65.0, 5.0, 0.0);
        
        TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(p1, p2);
        TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
        TopoDS_Wire wire1 = BRepBuilderAPI_MakeWire(edge1, edge2);
        
        // 创建第二个wire，与第一个wire连接
        // Create second wire, connected to first wire
        gp_Pnt p4(60.0, 5.0, 0.0);
        
        TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(p3, p4);
        TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(p4, p1);
        TopoDS_Wire wire2 = BRepBuilderAPI_MakeWire(edge3, edge4);
        
        // 合并两个wire
        // Merge two wires
        BRepBuilderAPI_MakeWire wireBuilder(wire1);
        wireBuilder.Add(wire2);
        
        if (wireBuilder.IsDone()) {
            std::cout << "Wires merged successfully" << std::endl;
            return wireBuilder.Wire();
        } else {
            std::cout << "Failed to merge wires: " << GetErrorString(wireBuilder.Error()) << std::endl;
            return TopoDS_Wire();
        }
    }
    
    /**
     * 辅助函数：将错误代码转换为字符串
     * Helper function: Convert error code to string
     */
    static std::string GetErrorString(BRepBuilderAPI_WireError error)
    {
        switch (error) {
            case BRepBuilderAPI_WireDone:
                return "WireDone";
            case BRepBuilderAPI_EmptyWire:
                return "EmptyWire";
            case BRepBuilderAPI_DisconnectedWire:
                return "DisconnectedWire";
            case BRepBuilderAPI_NonManifoldWire:
                return "NonManifoldWire";
            default:
                return "Unknown error";
        }
    }
    
    /**
     * 验证wire的有效性
     * Validate wire validity
     */
    static bool ValidateWire(const TopoDS_Wire& wire)
    {
        if (wire.IsNull()) {
            std::cout << "Wire is null" << std::endl;
            return false;
        }
        
        // 构建3D曲线（如果需要）
        // Build 3D curves if needed
        BRepLib::BuildCurves3d(wire);
        
        std::cout << "Wire validation passed" << std::endl;
        return true;
    }
};

/**
 * 主函数演示所有方法
 * Main function demonstrating all methods
 */
int main()
{
    std::cout << "OCCT Wire Creation Examples" << std::endl;
    std::cout << "===========================" << std::endl;
    
    try {
        // 方法1: 单个边
        TopoDS_Wire wire1 = WireFromEdgesExample::CreateWireFromSingleEdge();
        WireFromEdgesExample::ValidateWire(wire1);
        
        // 方法2: 多个连接的边
        TopoDS_Wire wire2 = WireFromEdgesExample::CreateWireFromConnectedEdges();
        WireFromEdgesExample::ValidateWire(wire2);
        
        // 方法3: 逐步添加边
        TopoDS_Wire wire3 = WireFromEdgesExample::CreateWireByAddingEdges();
        WireFromEdgesExample::ValidateWire(wire3);
        
        // 方法4: 从边列表创建
        TopoDS_Wire wire4 = WireFromEdgesExample::CreateWireFromEdgeList();
        WireFromEdgesExample::ValidateWire(wire4);
        
        // 方法5: 处理不连接的边
        TopoDS_Wire wire5 = WireFromEdgesExample::CreateWireFromDisconnectedEdges();
        WireFromEdgesExample::ValidateWire(wire5);
        
        // 方法6: 合并wire
        TopoDS_Wire wire6 = WireFromEdgesExample::MergeWires();
        WireFromEdgesExample::ValidateWire(wire6);
        
        std::cout << "\n=== Summary ===" << std::endl;
        std::cout << "All wire creation methods demonstrated successfully!" << std::endl;
        std::cout << "Key points:" << std::endl;
        std::cout << "1. Edges must be geometrically connected (shared vertices)" << std::endl;
        std::cout << "2. Use BRepBuilderAPI_MakeWire for wire creation" << std::endl;
        std::cout << "3. Always check IsDone() before using the result" << std::endl;
        std::cout << "4. Handle errors appropriately" << std::endl;
        std::cout << "5. Call BRepLib::BuildCurves3d() if needed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    return 0;
}

/*
编译说明 (Compilation Instructions):
=================================

1. 确保已安装OCCT库
   Make sure OCCT library is installed

2. 使用CMake编译:
   Compile using CMake:

cmake_minimum_required(VERSION 3.10)
project(WireFromEdgesExample)

find_package(OpenCASCADE REQUIRED)

add_executable(wire_example wire_from_edges_example.cpp)
target_link_libraries(wire_example ${OpenCASCADE_LIBRARIES})
target_include_directories(wire_example PRIVATE ${OpenCASCADE_INCLUDE_DIR})

3. 或者直接使用g++编译:
   Or compile directly with g++:

g++ -std=c++11 wire_from_edges_example.cpp -lTKernel -lTKMath -lTKBRep -lTKTopAlgo -o wire_example

使用要点 (Key Usage Points):
===========================

1. BRepBuilderAPI_MakeWire是创建wire的主要工具
   BRepBuilderAPI_MakeWire is the main tool for creating wires

2. 边必须首尾相连（共享顶点）
   Edges must be connected end-to-end (share vertices)

3. 可以从以下创建wire:
   You can create wires from:
   - 单个边 (single edge)
   - 多个连接的边 (multiple connected edges)
   - 现有的wire加新边 (existing wire + new edges)
   - 边的列表 (list of edges)

4. 始终检查IsDone()状态
   Always check IsDone() status

5. 处理错误情况
   Handle error cases appropriately
*/