#include "MultiLoopWireBuilder.h"
#include <iostream>
#include <algorithm>

// OCCT includes
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRep_Builder.hxx>
#include <BRepLib.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>

// EdgeInfo implementation
MultiLoopWireBuilder::EdgeInfo::EdgeInfo(const TopoDS_Edge& e) 
    : edge(e), groupId(-1), processed(false) 
{
    TopoDS_Vertex v1, v2;
    TopExp::Vertices(e, v1, v2);
    startPoint = BRep_Tool::Pnt(v1);
    endPoint = BRep_Tool::Pnt(v2);
}

// WireGroup implementation
MultiLoopWireBuilder::WireGroup::WireGroup() : isClosed(false) {}

// MultiLoopWireBuilder implementation
MultiLoopWireBuilder::MultiLoopWireBuilder(Standard_Real tolerance) 
    : m_tolerance(tolerance) {}

void MultiLoopWireBuilder::AddEdge(const TopoDS_Edge& edge) 
{
    m_edges.emplace_back(edge);
}

void MultiLoopWireBuilder::AddEdges(const TopTools_ListOfShape& edgeList) 
{
    for (TopTools_ListIteratorOfListOfShape it(edgeList); it.More(); it.Next()) {
        if (it.Value().ShapeType() == TopAbs_EDGE) {
            AddEdge(TopoDS::Edge(it.Value()));
        }
    }
}

void MultiLoopWireBuilder::AddEdges(const std::vector<TopoDS_Edge>& edges) 
{
    for (const auto& edge : edges) {
        AddEdge(edge);
    }
}

bool MultiLoopWireBuilder::ArePointsCoincident(const gp_Pnt& p1, const gp_Pnt& p2) const 
{
    return p1.Distance(p2) <= m_tolerance;
}

std::vector<int> MultiLoopWireBuilder::FindConnectedEdges(const gp_Pnt& point, int excludeIndex) 
{
    std::vector<int> connectedEdges;
    
    for (size_t i = 0; i < m_edges.size(); ++i) {
        if ((int)i == excludeIndex || m_edges[i].processed) continue;
        
        if (ArePointsCoincident(point, m_edges[i].startPoint) ||
            ArePointsCoincident(point, m_edges[i].endPoint)) {
            connectedEdges.push_back((int)i);
        }
    }
    
    return connectedEdges;
}

bool MultiLoopWireBuilder::AnalyzeConnectivity() 
{
    m_wireGroups.clear();
    
    // 重置处理状态
    for (auto& edge : m_edges) {
        edge.processed = false;
        edge.groupId = -1;
    }
    
    int currentGroupId = 0;
    
    for (size_t i = 0; i < m_edges.size(); ++i) {
        if (m_edges[i].processed) continue;
        
        // 开始新的连通组
        WireGroup newGroup;
        std::queue<int> edgeQueue;
        edgeQueue.push((int)i);
        
        while (!edgeQueue.empty()) {
            int currentEdgeIndex = edgeQueue.front();
            edgeQueue.pop();
            
            if (m_edges[currentEdgeIndex].processed) continue;
            
            // 标记为已处理
            m_edges[currentEdgeIndex].processed = true;
            m_edges[currentEdgeIndex].groupId = currentGroupId;
            newGroup.edges.push_back(m_edges[currentEdgeIndex].edge);
            
            // 查找连接的边
            std::vector<int> connectedToStart = FindConnectedEdges(m_edges[currentEdgeIndex].startPoint, currentEdgeIndex);
            std::vector<int> connectedToEnd = FindConnectedEdges(m_edges[currentEdgeIndex].endPoint, currentEdgeIndex);
            
            // 添加连接的边到队列
            for (int idx : connectedToStart) {
                if (!m_edges[idx].processed) {
                    edgeQueue.push(idx);
                }
            }
            for (int idx : connectedToEnd) {
                if (!m_edges[idx].processed) {
                    edgeQueue.push(idx);
                }
            }
        }
        
        m_wireGroups.push_back(newGroup);
        currentGroupId++;
    }
    
    std::cout << "Connectivity analysis found " << m_wireGroups.size() << " connected groups" << std::endl;
    return !m_wireGroups.empty();
}

bool MultiLoopWireBuilder::SortEdgesInGroups() 
{
    for (auto& group : m_wireGroups) {
        if (group.edges.size() <= 1) {
            if (group.edges.size() == 1) {
                // 单边情况
                TopoDS_Vertex v1, v2;
                TopExp::Vertices(group.edges[0], v1, v2);
                group.startPoint = BRep_Tool::Pnt(v1);
                group.endPoint = BRep_Tool::Pnt(v2);
                group.isClosed = false;
            }
            continue;
        }
        
        std::vector<TopoDS_Edge> sortedEdges;
        std::vector<bool> used(group.edges.size(), false);
        
        // 选择起始边
        sortedEdges.push_back(group.edges[0]);
        used[0] = true;
        
        // 获取起始边的端点
        TopoDS_Vertex startVertex, endVertex;
        TopExp::Vertices(group.edges[0], startVertex, endVertex);
        gp_Pnt currentPoint = BRep_Tool::Pnt(endVertex);
        group.startPoint = BRep_Tool::Pnt(startVertex);
        
        // 按连接顺序排序边
        while (sortedEdges.size() < group.edges.size()) {
            bool foundNext = false;
            
            for (size_t i = 0; i < group.edges.size(); ++i) {
                if (used[i]) continue;
                
                TopoDS_Vertex v1, v2;
                TopExp::Vertices(group.edges[i], v1, v2);
                gp_Pnt p1 = BRep_Tool::Pnt(v1);
                gp_Pnt p2 = BRep_Tool::Pnt(v2);
                
                if (ArePointsCoincident(currentPoint, p1)) {
                    sortedEdges.push_back(group.edges[i]);
                    used[i] = true;
                    currentPoint = p2;
                    foundNext = true;
                    break;
                } else if (ArePointsCoincident(currentPoint, p2)) {
                    // 需要反向边
                    TopoDS_Edge reversedEdge = group.edges[i].Reversed();
                    sortedEdges.push_back(reversedEdge);
                    used[i] = true;
                    currentPoint = p1;
                    foundNext = true;
                    break;
                }
            }
            
            if (!foundNext) {
                std::cout << "Warning: Could not find next connected edge in group" << std::endl;
                break;
            }
        }
        
        group.edges = sortedEdges;
        group.endPoint = currentPoint;
        group.isClosed = ArePointsCoincident(group.startPoint, group.endPoint);
        
        std::cout << "Group sorted: " << group.edges.size() << " edges, " 
                 << (group.isClosed ? "closed loop" : "open path") << std::endl;
    }
    
    return true;
}

std::vector<TopoDS_Wire> MultiLoopWireBuilder::BuildAllWires() 
{
    std::vector<TopoDS_Wire> resultWires;
    
    // 分析连通性
    if (!AnalyzeConnectivity()) {
        std::cout << "Failed to analyze connectivity" << std::endl;
        return resultWires;
    }
    
    // 排序边
    if (!SortEdgesInGroups()) {
        std::cout << "Failed to sort edges in groups" << std::endl;
        return resultWires;
    }
    
    // 为每个组创建Wire
    for (size_t groupIndex = 0; groupIndex < m_wireGroups.size(); ++groupIndex) {
        const auto& group = m_wireGroups[groupIndex];
        
        if (group.edges.empty()) continue;
        
        BRepBuilderAPI_MakeWire wireBuilder;
        
        for (const auto& edge : group.edges) {
            wireBuilder.Add(edge);
            if (!wireBuilder.IsDone()) {
                std::cout << "Failed to add edge to wire in group " << groupIndex + 1 << std::endl;
                break;
            }
        }
        
        if (wireBuilder.IsDone()) {
            TopoDS_Wire wire = wireBuilder.Wire();
            BRepLib::BuildCurves3d(wire);
            resultWires.push_back(wire);
            
            std::cout << "✓ Created " << (group.isClosed ? "closed" : "open") 
                     << " wire " << resultWires.size() 
                     << " with " << group.edges.size() << " edges" << std::endl;
        } else {
            std::cout << "✗ Failed to create wire for group " << groupIndex + 1 << std::endl;
        }
    }
    
    return resultWires;
}

std::vector<TopoDS_Wire> MultiLoopWireBuilder::BuildWiresUsingFreeBounds() 
{
    std::vector<TopoDS_Wire> resultWires;
    
    if (m_edges.empty()) {
        std::cout << "No edges to process" << std::endl;
        return resultWires;
    }
    
    // 创建包含所有边的复合体
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    
    for (const auto& edgeInfo : m_edges) {
        builder.Add(compound, edgeInfo.edge);
    }
    
    try {
        // 使用ShapeAnalysis_FreeBounds分析自由边界
        ShapeAnalysis_FreeBounds analyzer(compound, m_tolerance, Standard_False, Standard_True);
        
        TopoDS_Compound closedWires = analyzer.GetClosedWires();
        TopoDS_Compound openWires = analyzer.GetOpenWires();
        
        // 提取闭合Wire
        TopExp_Explorer exp1(closedWires, TopAbs_WIRE);
        int closedCount = 0;
        for (; exp1.More(); exp1.Next()) {
            TopoDS_Wire wire = TopoDS::Wire(exp1.Current());
            BRepLib::BuildCurves3d(wire);
            resultWires.push_back(wire);
            closedCount++;
        }
        
        // 提取开放Wire
        TopExp_Explorer exp2(openWires, TopAbs_WIRE);
        int openCount = 0;
        for (; exp2.More(); exp2.Next()) {
            TopoDS_Wire wire = TopoDS::Wire(exp2.Current());
            BRepLib::BuildCurves3d(wire);
            resultWires.push_back(wire);
            openCount++;
        }
        
        std::cout << "FreeBounds analysis found " << closedCount << " closed and " 
                 << openCount << " open wires" << std::endl;
                 
    } catch (const std::exception& e) {
        std::cout << "Exception in FreeBounds analysis: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown exception in FreeBounds analysis" << std::endl;
    }
    
    return resultWires;
}

void MultiLoopWireBuilder::Reset() 
{
    m_edges.clear();
    m_wireGroups.clear();
}

void MultiLoopWireBuilder::PrintAnalysisResults() const 
{
    std::cout << "\n=== 详细分析结果 / Detailed Analysis Results ===" << std::endl;
    std::cout << "Total edges: " << m_edges.size() << std::endl;
    std::cout << "Connected groups: " << m_wireGroups.size() << std::endl;
    std::cout << "Geometric tolerance: " << m_tolerance << std::endl;
    
    for (size_t i = 0; i < m_wireGroups.size(); ++i) {
        const auto& group = m_wireGroups[i];
        std::cout << "\nGroup " << i + 1 << ":" << std::endl;
        std::cout << "  - Edges: " << group.edges.size() << std::endl;
        std::cout << "  - Type: " << (group.isClosed ? "Closed loop" : "Open path") << std::endl;
        
        if (!group.isClosed) {
            std::cout << "  - Start: (" << group.startPoint.X() << ", " 
                     << group.startPoint.Y() << ", " << group.startPoint.Z() << ")" << std::endl;
            std::cout << "  - End: (" << group.endPoint.X() << ", " 
                     << group.endPoint.Y() << ", " << group.endPoint.Z() << ")" << std::endl;
        }
    }
}

bool MultiLoopWireBuilder::ValidateWire(const TopoDS_Wire& wire) 
{
    if (wire.IsNull()) {
        std::cout << "Wire is null" << std::endl;
        return false;
    }
    
    try {
        // 构建3D曲线
        BRepLib::BuildCurves3d(wire);
        
        // 计算边数
        int edgeCount = 0;
        TopExp_Explorer exp(wire, TopAbs_EDGE);
        for (; exp.More(); exp.Next()) {
            edgeCount++;
        }
        
        std::cout << "Wire validation passed: " << edgeCount << " edges" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "Wire validation failed: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cout << "Wire validation failed: unknown error" << std::endl;
        return false;
    }
}

// Convenience functions implementation
std::vector<TopoDS_Wire> CreateMultiLoopWires(const TopTools_ListOfShape& edges, 
                                              Standard_Real tolerance) 
{
    MultiLoopWireBuilder builder(tolerance);
    builder.AddEdges(edges);
    std::vector<TopoDS_Wire> wires = builder.BuildAllWires();
    builder.PrintAnalysisResults();
    return wires;
}

std::vector<TopoDS_Wire> CreateMultiLoopWires(const std::vector<TopoDS_Edge>& edges,
                                              Standard_Real tolerance) 
{
    MultiLoopWireBuilder builder(tolerance);
    builder.AddEdges(edges);
    std::vector<TopoDS_Wire> wires = builder.BuildAllWires();
    builder.PrintAnalysisResults();
    return wires;
}

/**
 * 演示复杂多环情况的测试类
 * Test class for demonstrating complex multi-loop cases
 */
class ComplexMultiLoopTest
{
public:
    /**
     * 创建复杂的嵌套环结构
     * Create complex nested loop structure
     */
    static TopTools_ListOfShape CreateComplexNestedStructure() 
    {
        TopTools_ListOfShape edgeList;
        
        // 外层大矩形 (0,0) to (30,20)
        gp_Pnt outer1(0, 0, 0), outer2(30, 0, 0), outer3(30, 20, 0), outer4(0, 20, 0);
        edgeList.Append(BRepBuilderAPI_MakeEdge(outer1, outer2));
        edgeList.Append(BRepBuilderAPI_MakeEdge(outer2, outer3));
        edgeList.Append(BRepBuilderAPI_MakeEdge(outer3, outer4));
        edgeList.Append(BRepBuilderAPI_MakeEdge(outer4, outer1));
        
        // 内层左矩形 (5,5) to (12,15)
        gp_Pnt inner1_1(5, 5, 0), inner1_2(12, 5, 0), inner1_3(12, 15, 0), inner1_4(5, 15, 0);
        edgeList.Append(BRepBuilderAPI_MakeEdge(inner1_1, inner1_2));
        edgeList.Append(BRepBuilderAPI_MakeEdge(inner1_2, inner1_3));
        edgeList.Append(BRepBuilderAPI_MakeEdge(inner1_3, inner1_4));
        edgeList.Append(BRepBuilderAPI_MakeEdge(inner1_4, inner1_1));
        
        // 内层右矩形 (18,5) to (25,15)
        gp_Pnt inner2_1(18, 5, 0), inner2_2(25, 5, 0), inner2_3(25, 15, 0), inner2_4(18, 15, 0);
        edgeList.Append(BRepBuilderAPI_MakeEdge(inner2_1, inner2_2));
        edgeList.Append(BRepBuilderAPI_MakeEdge(inner2_2, inner2_3));
        edgeList.Append(BRepBuilderAPI_MakeEdge(inner2_3, inner2_4));
        edgeList.Append(BRepBuilderAPI_MakeEdge(inner2_4, inner2_1));
        
        // 独立的三角形 (35,0) to (45,10)
        gp_Pnt tri1(35, 0, 0), tri2(45, 0, 0), tri3(40, 10, 0);
        edgeList.Append(BRepBuilderAPI_MakeEdge(tri1, tri2));
        edgeList.Append(BRepBuilderAPI_MakeEdge(tri2, tri3));
        edgeList.Append(BRepBuilderAPI_MakeEdge(tri3, tri1));
        
        // 开放的Z字形路径
        gp_Pnt z1(50, 0, 0), z2(55, 0, 0), z3(50, 5, 0), z4(55, 5, 0);
        edgeList.Append(BRepBuilderAPI_MakeEdge(z1, z2));
        edgeList.Append(BRepBuilderAPI_MakeEdge(z2, z3));
        edgeList.Append(BRepBuilderAPI_MakeEdge(z3, z4));
        
        return edgeList;
    }
    
    /**
     * 创建打乱顺序的多环边
     * Create multi-loop edges in random order
     */
    static TopTools_ListOfShape CreateShuffledMultiLoopEdges() 
    {
        TopTools_ListOfShape edgeList;
        
        // 创建两个矩形的边，但完全打乱顺序
        std::vector<TopoDS_Edge> allEdges;
        
        // 矩形1的边
        gp_Pnt r1p1(0, 0, 0), r1p2(8, 0, 0), r1p3(8, 6, 0), r1p4(0, 6, 0);
        allEdges.push_back(BRepBuilderAPI_MakeEdge(r1p1, r1p2));
        allEdges.push_back(BRepBuilderAPI_MakeEdge(r1p2, r1p3));
        allEdges.push_back(BRepBuilderAPI_MakeEdge(r1p3, r1p4));
        allEdges.push_back(BRepBuilderAPI_MakeEdge(r1p4, r1p1));
        
        // 矩形2的边
        gp_Pnt r2p1(15, 0, 0), r2p2(23, 0, 0), r2p3(23, 6, 0), r2p4(15, 6, 0);
        allEdges.push_back(BRepBuilderAPI_MakeEdge(r2p1, r2p2));
        allEdges.push_back(BRepBuilderAPI_MakeEdge(r2p2, r2p3));
        allEdges.push_back(BRepBuilderAPI_MakeEdge(r2p3, r2p4));
        allEdges.push_back(BRepBuilderAPI_MakeEdge(r2p4, r2p1));
        
        // 打乱顺序
        std::vector<int> indices = {7, 2, 5, 0, 3, 6, 1, 4}; // 随机顺序
        
        for (int idx : indices) {
            edgeList.Append(allEdges[idx]);
        }
        
        return edgeList;
    }
    
    /**
     * 运行所有复杂测试
     * Run all complex tests
     */
    static void RunComplexTests() 
    {
        std::cout << "\n=== 复杂多环测试 / Complex Multi-Loop Tests ===" << std::endl;
        
        // 测试1: 复杂嵌套结构
        std::cout << "\n1. 复杂嵌套结构测试 / Complex Nested Structure Test:" << std::endl;
        {
            TopTools_ListOfShape edges = CreateComplexNestedStructure();
            std::vector<TopoDS_Wire> wires = CreateMultiLoopWires(edges);
            std::cout << "Complex nested test result: " << wires.size() << " wires created" << std::endl;
        }
        
        // 测试2: 打乱顺序的多环
        std::cout << "\n2. 打乱顺序多环测试 / Shuffled Multi-Loop Test:" << std::endl;
        {
            TopTools_ListOfShape edges = CreateShuffledMultiLoopEdges();
            std::vector<TopoDS_Wire> wires = CreateMultiLoopWires(edges);
            std::cout << "Shuffled multi-loop test result: " << wires.size() << " wires created" << std::endl;
        }
        
        // 测试3: 使用FreeBounds算法对比
        std::cout << "\n3. FreeBounds算法对比测试 / FreeBounds Algorithm Comparison:" << std::endl;
        {
            MultiLoopWireBuilder builder;
            TopTools_ListOfShape edges = CreateComplexNestedStructure();
            builder.AddEdges(edges);
            
            std::cout << "Custom algorithm:" << std::endl;
            std::vector<TopoDS_Wire> customWires = builder.BuildAllWires();
            
            std::cout << "FreeBounds algorithm:" << std::endl;
            std::vector<TopoDS_Wire> freeBoundWires = builder.BuildWiresUsingFreeBounds();
            
            std::cout << "Comparison: Custom=" << customWires.size() 
                     << " vs FreeBounds=" << freeBoundWires.size() << " wires" << std::endl;
        }
    }
};

/**
 * 主测试函数
 * Main test function
 */
void TestComplexMultiLoopScenarios() 
{
    std::cout << "=== OCCT复杂多环Wire创建测试 ===" << std::endl;
    std::cout << "=== OCCT Complex Multi-Loop Wire Creation Test ===" << std::endl;
    
    ComplexMultiLoopTest::RunComplexTests();
    
    std::cout << "\n=== 总结 / Summary ===" << std::endl;
    std::cout << "多环Wire创建的核心挑战和解决方案:" << std::endl;
    std::cout << "1. 连通性分析 - 使用图论算法识别连通组件" << std::endl;
    std::cout << "2. 边排序 - 确保边按正确的连接顺序排列" << std::endl;
    std::cout << "3. 多环处理 - 同时处理多个独立的环结构" << std::endl;
    std::cout << "4. 容差管理 - 正确处理几何精度问题" << std::endl;
    std::cout << "5. 算法选择 - 提供多种算法以应对不同情况" << std::endl;
    
    std::cout << "\nCore challenges and solutions for multi-loop wire creation:" << std::endl;
    std::cout << "1. Connectivity analysis - Use graph algorithms to identify connected components" << std::endl;
    std::cout << "2. Edge sorting - Ensure edges are arranged in correct connection order" << std::endl;
    std::cout << "3. Multi-loop handling - Process multiple independent loop structures simultaneously" << std::endl;
    std::cout << "4. Tolerance management - Properly handle geometric precision issues" << std::endl;
    std::cout << "5. Algorithm selection - Provide multiple algorithms for different scenarios" << std::endl;
}

/*
编译说明 / Compilation Instructions:
===================================

g++ -std=c++11 MultiLoopWireBuilder.cpp multi_loop_wire_example.cpp \
    -lTKernel -lTKMath -lTKBRep -lTKTopAlgo -lTKShHealing \
    -lTKG3d -lTKGeomBase -lTKGeomAlgo \
    -o multi_loop_test

或者创建一个测试程序:
Or create a test program:

#include "MultiLoopWireBuilder.h"

int main() {
    TestComplexMultiLoopScenarios();
    return 0;
}

实际应用场景 / Real-world Application Scenarios:
==============================================

1. CAD文件导入 - 处理从DXF/DWG等格式导入的不规则边数据
2. 网格边界重建 - 从三角网格提取边界并重建为Wire
3. 图像轮廓跟踪 - 将图像处理得到的轮廓点转换为几何Wire
4. 路径规划 - 在复杂环境中规划多条连续路径
5. 制造工艺 - 为CNC加工生成工具路径

1. CAD file import - Handle irregular edge data imported from DXF/DWG formats
2. Mesh boundary reconstruction - Extract boundaries from triangle meshes and rebuild as wires
3. Image contour tracing - Convert contour points from image processing to geometric wires
4. Path planning - Plan multiple continuous paths in complex environments
5. Manufacturing processes - Generate tool paths for CNC machining

*/