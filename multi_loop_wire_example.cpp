#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

// OCCT核心几何类
#include <gp_Pnt.hxx>
#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>

// OCCT拓扑类
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Compound.hxx>

// OCCT构建API
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRep_Builder.hxx>

// OCCT工具类
#include <TopTools_ListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <BRepBuilderAPI_WireError.hxx>
#include <BRepLib.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

// OCCT分析工具
#include <ShapeAnalysis_FreeBounds.hxx>

/**
 * 多环Wire创建器
 * Multi-loop Wire Creator
 * 
 * 处理多条边可能形成多个独立环的复杂情况
 * Handles complex cases where multiple edges may form multiple independent loops
 */
class MultiLoopWireBuilder
{
private:
    struct EdgeInfo {
        TopoDS_Edge edge;
        TopoDS_Vertex startVertex;
        TopoDS_Vertex endVertex;
        bool used;
        
        EdgeInfo(const TopoDS_Edge& e) : edge(e), used(false) {
            TopExp::Vertices(e, startVertex, endVertex);
        }
    };
    
    std::vector<EdgeInfo> edges;
    std::vector<TopoDS_Wire> resultWires;
    Standard_Real tolerance;

public:
    MultiLoopWireBuilder(Standard_Real tol = 1e-6) : tolerance(tol) {}
    
    /**
     * 添加边到构建器
     * Add edge to builder
     */
    void AddEdge(const TopoDS_Edge& edge) {
        edges.emplace_back(edge);
    }
    
    /**
     * 添加多条边
     * Add multiple edges
     */
    void AddEdges(const TopTools_ListOfShape& edgeList) {
        for (TopTools_ListIteratorOfListOfShape it(edgeList); it.More(); it.Next()) {
            if (it.Value().ShapeType() == TopAbs_EDGE) {
                AddEdge(TopoDS::Edge(it.Value()));
            }
        }
    }
    
    /**
     * 检查两个顶点是否几何重合
     * Check if two vertices are geometrically coincident
     */
    bool AreVerticesCoincident(const TopoDS_Vertex& v1, const TopoDS_Vertex& v2) {
        if (v1.IsSame(v2)) return true;
        
        gp_Pnt p1 = BRep_Tool::Pnt(v1);
        gp_Pnt p2 = BRep_Tool::Pnt(v2);
        
        return p1.Distance(p2) <= tolerance;
    }
    
    /**
     * 查找与给定顶点连接的下一条边
     * Find next edge connected to given vertex
     */
    int FindConnectedEdge(const TopoDS_Vertex& vertex, int excludeIndex = -1) {
        for (size_t i = 0; i < edges.size(); ++i) {
            if (edges[i].used || (int)i == excludeIndex) continue;
            
            if (AreVerticesCoincident(vertex, edges[i].startVertex) ||
                AreVerticesCoincident(vertex, edges[i].endVertex)) {
                return (int)i;
            }
        }
        return -1;
    }
    
    /**
     * 从给定边开始构建一个连续的Wire
     * Build a continuous wire starting from given edge
     */
    TopoDS_Wire BuildSingleWire(int startEdgeIndex) {
        BRepBuilderAPI_MakeWire wireBuilder;
        std::vector<int> usedEdges;
        
        // 添加起始边
        wireBuilder.Add(edges[startEdgeIndex].edge);
        edges[startEdgeIndex].used = true;
        usedEdges.push_back(startEdgeIndex);
        
        if (!wireBuilder.IsDone()) {
            std::cout << "Failed to add starting edge" << std::endl;
            return TopoDS_Wire();
        }
        
        // 获取当前Wire的端点
        TopoDS_Vertex currentEnd = edges[startEdgeIndex].endVertex;
        TopoDS_Vertex wireStart = edges[startEdgeIndex].startVertex;
        
        bool foundConnection = true;
        while (foundConnection) {
            foundConnection = false;
            
            // 查找连接到当前端点的边
            int nextEdgeIndex = FindConnectedEdge(currentEnd, -1);
            if (nextEdgeIndex >= 0) {
                wireBuilder.Add(edges[nextEdgeIndex].edge);
                
                if (wireBuilder.IsDone()) {
                    edges[nextEdgeIndex].used = true;
                    usedEdges.push_back(nextEdgeIndex);
                    
                    // 更新当前端点
                    if (AreVerticesCoincident(currentEnd, edges[nextEdgeIndex].startVertex)) {
                        currentEnd = edges[nextEdgeIndex].endVertex;
                    } else {
                        currentEnd = edges[nextEdgeIndex].startVertex;
                    }
                    
                    foundConnection = true;
                    
                    // 检查是否形成闭合环
                    if (AreVerticesCoincident(currentEnd, wireStart)) {
                        std::cout << "Closed loop detected!" << std::endl;
                        break;
                    }
                } else {
                    // 如果添加失败，撤销标记
                    std::cout << "Failed to add edge, wire may be disconnected" << std::endl;
                    break;
                }
            }
        }
        
        if (wireBuilder.IsDone()) {
            TopoDS_Wire wire = wireBuilder.Wire();
            BRepLib::BuildCurves3d(wire);
            return wire;
        }
        
        return TopoDS_Wire();
    }
    
    /**
     * 构建所有可能的Wire
     * Build all possible wires
     */
    bool BuildAllWires() {
        resultWires.clear();
        
        // 重置所有边的使用状态
        for (auto& edgeInfo : edges) {
            edgeInfo.used = false;
        }
        
        // 遍历所有未使用的边，尝试构建Wire
        for (size_t i = 0; i < edges.size(); ++i) {
            if (!edges[i].used) {
                TopoDS_Wire wire = BuildSingleWire((int)i);
                if (!wire.IsNull()) {
                    resultWires.push_back(wire);
                    std::cout << "Created wire " << resultWires.size() << std::endl;
                }
            }
        }
        
        std::cout << "Total wires created: " << resultWires.size() << std::endl;
        return !resultWires.empty();
    }
    
    /**
     * 使用OCCT内置算法构建Wire（备选方法）
     * Build wires using OCCT built-in algorithm (alternative method)
     */
    bool BuildWiresUsingFreeBounds() {
        resultWires.clear();
        
        // 创建包含所有边的复合体
        BRep_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        
        for (const auto& edgeInfo : edges) {
            builder.Add(compound, edgeInfo.edge);
        }
        
        // 使用ShapeAnalysis_FreeBounds分析自由边界
        ShapeAnalysis_FreeBounds analyzer(compound, tolerance, Standard_False, Standard_True);
        
        TopoDS_Compound closedWires = analyzer.GetClosedWires();
        TopoDS_Compound openWires = analyzer.GetOpenWires();
        
        // 提取闭合Wire
        TopExp_Explorer exp1(closedWires, TopAbs_WIRE);
        for (; exp1.More(); exp1.Next()) {
            TopoDS_Wire wire = TopoDS::Wire(exp1.Current());
            BRepLib::BuildCurves3d(wire);
            resultWires.push_back(wire);
            std::cout << "Found closed wire using FreeBounds analysis" << std::endl;
        }
        
        // 提取开放Wire
        TopExp_Explorer exp2(openWires, TopAbs_WIRE);
        for (; exp2.More(); exp2.Next()) {
            TopoDS_Wire wire = TopoDS::Wire(exp2.Current());
            BRepLib::BuildCurves3d(wire);
            resultWires.push_back(wire);
            std::cout << "Found open wire using FreeBounds analysis" << std::endl;
        }
        
        std::cout << "FreeBounds analysis found " << resultWires.size() << " wires" << std::endl;
        return !resultWires.empty();
    }
    
    /**
     * 获取构建的Wire列表
     * Get list of built wires
     */
    const std::vector<TopoDS_Wire>& GetWires() const {
        return resultWires;
    }
    
    /**
     * 获取Wire数量
     * Get number of wires
     */
    size_t GetWireCount() const {
        return resultWires.size();
    }
    
    /**
     * 重置构建器
     * Reset builder
     */
    void Reset() {
        edges.clear();
        resultWires.clear();
    }
};

/**
 * 创建测试用的多个环的边
 * Create test edges that form multiple loops
 */
class MultiLoopTestData
{
public:
    /**
     * 创建两个独立的矩形环
     * Create two independent rectangular loops
     */
    static TopTools_ListOfShape CreateTwoRectangularLoops() {
        TopTools_ListOfShape edgeList;
        
        // 第一个矩形 (0,0) to (10,10)
        gp_Pnt p1(0.0, 0.0, 0.0);
        gp_Pnt p2(10.0, 0.0, 0.0);
        gp_Pnt p3(10.0, 10.0, 0.0);
        gp_Pnt p4(0.0, 10.0, 0.0);
        
        edgeList.Append(BRepBuilderAPI_MakeEdge(p1, p2));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p2, p3));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p3, p4));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p4, p1));
        
        // 第二个矩形 (20,0) to (30,10)
        gp_Pnt p5(20.0, 0.0, 0.0);
        gp_Pnt p6(30.0, 0.0, 0.0);
        gp_Pnt p7(30.0, 10.0, 0.0);
        gp_Pnt p8(20.0, 10.0, 0.0);
        
        edgeList.Append(BRepBuilderAPI_MakeEdge(p5, p6));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p6, p7));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p7, p8));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p8, p5));
        
        return edgeList;
    }
    
    /**
     * 创建一个大环和一个内部小环
     * Create one large loop and one internal small loop
     */
    static TopTools_ListOfShape CreateNestedLoops() {
        TopTools_ListOfShape edgeList;
        
        // 外环 - 大矩形
        gp_Pnt p1(0.0, 0.0, 0.0);
        gp_Pnt p2(20.0, 0.0, 0.0);
        gp_Pnt p3(20.0, 20.0, 0.0);
        gp_Pnt p4(0.0, 20.0, 0.0);
        
        edgeList.Append(BRepBuilderAPI_MakeEdge(p1, p2));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p2, p3));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p3, p4));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p4, p1));
        
        // 内环 - 小矩形
        gp_Pnt p5(5.0, 5.0, 0.0);
        gp_Pnt p6(15.0, 5.0, 0.0);
        gp_Pnt p7(15.0, 15.0, 0.0);
        gp_Pnt p8(5.0, 15.0, 0.0);
        
        edgeList.Append(BRepBuilderAPI_MakeEdge(p5, p6));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p6, p7));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p7, p8));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p8, p5));
        
        return edgeList;
    }
    
    /**
     * 创建混乱顺序的边（需要排序）
     * Create edges in random order (requires sorting)
     */
    static TopTools_ListOfShape CreateRandomOrderEdges() {
        TopTools_ListOfShape edgeList;
        
        // 创建一个三角形的边，但顺序打乱
        gp_Pnt p1(0.0, 0.0, 0.0);
        gp_Pnt p2(10.0, 0.0, 0.0);
        gp_Pnt p3(5.0, 8.66, 0.0);  // 等边三角形
        
        // 故意打乱顺序
        edgeList.Append(BRepBuilderAPI_MakeEdge(p2, p3));  // 第二条边
        edgeList.Append(BRepBuilderAPI_MakeEdge(p3, p1));  // 第三条边
        edgeList.Append(BRepBuilderAPI_MakeEdge(p1, p2));  // 第一条边
        
        return edgeList;
    }
    
    /**
     * 创建部分连接的边（形成开放Wire和闭合Wire）
     * Create partially connected edges (forming open and closed wires)
     */
    static TopTools_ListOfShape CreateMixedConnectivity() {
        TopTools_ListOfShape edgeList;
        
        // 闭合的正方形
        gp_Pnt p1(0.0, 0.0, 0.0);
        gp_Pnt p2(5.0, 0.0, 0.0);
        gp_Pnt p3(5.0, 5.0, 0.0);
        gp_Pnt p4(0.0, 5.0, 0.0);
        
        edgeList.Append(BRepBuilderAPI_MakeEdge(p1, p2));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p2, p3));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p3, p4));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p4, p1));
        
        // 开放的折线
        gp_Pnt p5(10.0, 0.0, 0.0);
        gp_Pnt p6(15.0, 0.0, 0.0);
        gp_Pnt p7(15.0, 5.0, 0.0);
        
        edgeList.Append(BRepBuilderAPI_MakeEdge(p5, p6));
        edgeList.Append(BRepBuilderAPI_MakeEdge(p6, p7));
        
        // 孤立的边
        gp_Pnt p8(20.0, 0.0, 0.0);
        gp_Pnt p9(25.0, 0.0, 0.0);
        
        edgeList.Append(BRepBuilderAPI_MakeEdge(p8, p9));
        
        return edgeList;
    }
};

/**
 * 演示函数
 * Demonstration functions
 */
void DemonstrateMultiLoopWireCreation()
{
    std::cout << "=== 多环Wire创建演示 / Multi-Loop Wire Creation Demo ===" << std::endl;
    
    // 测试1: 两个独立的矩形环
    std::cout << "\n1. 两个独立的矩形环 / Two Independent Rectangular Loops:" << std::endl;
    {
        MultiLoopWireBuilder builder;
        TopTools_ListOfShape edges = MultiLoopTestData::CreateTwoRectangularLoops();
        builder.AddEdges(edges);
        
        if (builder.BuildAllWires()) {
            std::cout << "✓ Successfully created " << builder.GetWireCount() << " wires" << std::endl;
        } else {
            std::cout << "✗ Failed to create wires" << std::endl;
        }
    }
    
    // 测试2: 使用OCCT内置算法
    std::cout << "\n2. 使用OCCT FreeBounds分析 / Using OCCT FreeBounds Analysis:" << std::endl;
    {
        MultiLoopWireBuilder builder;
        TopTools_ListOfShape edges = MultiLoopTestData::CreateNestedLoops();
        builder.AddEdges(edges);
        
        if (builder.BuildWiresUsingFreeBounds()) {
            std::cout << "✓ FreeBounds analysis created " << builder.GetWireCount() << " wires" << std::endl;
        } else {
            std::cout << "✗ FreeBounds analysis failed" << std::endl;
        }
    }
    
    // 测试3: 混乱顺序的边
    std::cout << "\n3. 处理混乱顺序的边 / Handling Random Order Edges:" << std::endl;
    {
        MultiLoopWireBuilder builder;
        TopTools_ListOfShape edges = MultiLoopTestData::CreateRandomOrderEdges();
        builder.AddEdges(edges);
        
        if (builder.BuildAllWires()) {
            std::cout << "✓ Successfully sorted and created " << builder.GetWireCount() << " wires" << std::endl;
        } else {
            std::cout << "✗ Failed to sort and create wires" << std::endl;
        }
    }
    
    // 测试4: 混合连接性（闭合+开放+孤立）
    std::cout << "\n4. 混合连接性 / Mixed Connectivity:" << std::endl;
    {
        MultiLoopWireBuilder builder;
        TopTools_ListOfShape edges = MultiLoopTestData::CreateMixedConnectivity();
        builder.AddEdges(edges);
        
        if (builder.BuildAllWires()) {
            std::cout << "✓ Successfully handled mixed connectivity, created " << builder.GetWireCount() << " wires" << std::endl;
        } else {
            std::cout << "✗ Failed to handle mixed connectivity" << std::endl;
        }
    }
}

/**
 * 高级Wire构建器 - 使用图论算法
 * Advanced Wire Builder - Using graph theory algorithms
 */
class AdvancedWireBuilder
{
private:
    struct Graph {
        std::map<gp_Pnt, std::vector<std::pair<gp_Pnt, TopoDS_Edge>>, 
                 std::function<bool(const gp_Pnt&, const gp_Pnt&)>> adjacencyList;
        
        Graph() : adjacencyList([](const gp_Pnt& a, const gp_Pnt& b) {
            if (std::abs(a.X() - b.X()) > 1e-6) return a.X() < b.X();
            if (std::abs(a.Y() - b.Y()) > 1e-6) return a.Y() < b.Y();
            return a.Z() < b.Z();
        }) {}
        
        void AddEdge(const TopoDS_Edge& edge) {
            TopoDS_Vertex v1, v2;
            TopExp::Vertices(edge, v1, v2);
            gp_Pnt p1 = BRep_Tool::Pnt(v1);
            gp_Pnt p2 = BRep_Tool::Pnt(v2);
            
            adjacencyList[p1].emplace_back(p2, edge);
            adjacencyList[p2].emplace_back(p1, edge);
        }
        
        std::vector<std::vector<TopoDS_Edge>> FindAllCycles() {
            std::vector<std::vector<TopoDS_Edge>> cycles;
            std::set<TopoDS_Edge> usedEdges;
            
            for (const auto& vertex : adjacencyList) {
                const gp_Pnt& startPoint = vertex.first;
                
                for (const auto& neighbor : vertex.second) {
                    const TopoDS_Edge& startEdge = neighbor.second;
                    if (usedEdges.count(startEdge)) continue;
                    
                    std::vector<TopoDS_Edge> currentPath;
                    std::set<TopoDS_Edge> pathEdges;
                    
                    if (FindCycleFromEdge(startPoint, neighbor.first, startEdge, 
                                        currentPath, pathEdges, usedEdges)) {
                        cycles.push_back(currentPath);
                        for (const auto& e : currentPath) {
                            usedEdges.insert(e);
                        }
                    }
                }
            }
            
            return cycles;
        }
        
    private:
        bool FindCycleFromEdge(const gp_Pnt& start, const gp_Pnt& current, 
                              const TopoDS_Edge& edge,
                              std::vector<TopoDS_Edge>& path,
                              std::set<TopoDS_Edge>& pathEdges,
                              const std::set<TopoDS_Edge>& usedEdges) {
            
            if (usedEdges.count(edge) || pathEdges.count(edge)) {
                return false;
            }
            
            path.push_back(edge);
            pathEdges.insert(edge);
            
            // 检查是否回到起点
            if (path.size() > 2 && current.Distance(start) < 1e-6) {
                return true;
            }
            
            // 继续搜索
            auto it = adjacencyList.find(current);
            if (it != adjacencyList.end()) {
                for (const auto& neighbor : it->second) {
                    if (!pathEdges.count(neighbor.second)) {
                        if (FindCycleFromEdge(start, neighbor.first, neighbor.second,
                                            path, pathEdges, usedEdges)) {
                            return true;
                        }
                    }
                }
            }
            
            // 回溯
            path.pop_back();
            pathEdges.erase(edge);
            return false;
        }
    };
    
public:
    static std::vector<TopoDS_Wire> BuildWiresFromGraph(const TopTools_ListOfShape& edges) {
        Graph graph;
        
        // 构建图
        for (TopTools_ListIteratorOfListOfShape it(edges); it.More(); it.Next()) {
            if (it.Value().ShapeType() == TopAbs_EDGE) {
                graph.AddEdge(TopoDS::Edge(it.Value()));
            }
        }
        
        // 查找所有环
        std::vector<std::vector<TopoDS_Edge>> cycles = graph.FindAllCycles();
        
        // 为每个环创建Wire
        std::vector<TopoDS_Wire> wires;
        for (const auto& cycle : cycles) {
            BRepBuilderAPI_MakeWire wireBuilder;
            
            for (const auto& edge : cycle) {
                wireBuilder.Add(edge);
                if (!wireBuilder.IsDone()) {
                    std::cout << "Failed to add edge to wire" << std::endl;
                    break;
                }
            }
            
            if (wireBuilder.IsDone()) {
                TopoDS_Wire wire = wireBuilder.Wire();
                BRepLib::BuildCurves3d(wire);
                wires.push_back(wire);
            }
        }
        
        return wires;
    }
};

int main()
{
    std::cout << "OCCT多环Wire创建示例 / OCCT Multi-Loop Wire Creation Example" << std::endl;
    std::cout << "=============================================================" << std::endl;
    
    try {
        // 基础多环演示
        DemonstrateMultiLoopWireCreation();
        
        // 高级图论算法演示
        std::cout << "\n=== 高级图论算法演示 / Advanced Graph Algorithm Demo ===" << std::endl;
        TopTools_ListOfShape testEdges = MultiLoopTestData::CreateTwoRectangularLoops();
        std::vector<TopoDS_Wire> graphWires = AdvancedWireBuilder::BuildWiresFromGraph(testEdges);
        std::cout << "Graph algorithm created " << graphWires.size() << " wires" << std::endl;
        
        std::cout << "\n=== 总结 / Summary ===" << std::endl;
        std::cout << "多环Wire创建的关键点:" << std::endl;
        std::cout << "1. 使用图论算法分析边的连接关系" << std::endl;
        std::cout << "2. 自动检测和分离不同的环" << std::endl;
        std::cout << "3. 处理嵌套环和独立环" << std::endl;
        std::cout << "4. 使用ShapeAnalysis_FreeBounds进行高级分析" << std::endl;
        std::cout << "5. 提供多种构建策略以应对不同情况" << std::endl;
        
        std::cout << "\nKey points for multi-loop wire creation:" << std::endl;
        std::cout << "1. Use graph algorithms to analyze edge connectivity" << std::endl;
        std::cout << "2. Automatically detect and separate different loops" << std::endl;
        std::cout << "3. Handle nested and independent loops" << std::endl;
        std::cout << "4. Use ShapeAnalysis_FreeBounds for advanced analysis" << std::endl;
        std::cout << "5. Provide multiple building strategies for different cases" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    return 0;
}

/*
编译说明 / Compilation Instructions:
===================================

g++ -std=c++11 multi_loop_wire_example.cpp \
    -lTKernel -lTKMath -lTKBRep -lTKTopAlgo -lTKShHealing \
    -lTKG3d -lTKGeomBase -lTKGeomAlgo \
    -o multi_loop_wire_example

核心策略 / Core Strategies:
==========================

1. 自定义算法 (Custom Algorithm):
   - 使用图数据结构分析边的连接关系
   - 实现深度优先搜索找到所有环
   - 适用于复杂的边连接情况

2. OCCT内置算法 (OCCT Built-in Algorithm):
   - 使用ShapeAnalysis_FreeBounds类
   - 自动分析自由边界并构建Wire
   - 更可靠但灵活性较低

3. 混合方法 (Hybrid Method):
   - 结合多种算法的优势
   - 根据具体情况选择最佳策略
   - 提供最好的鲁棒性

使用场景 / Use Cases:
====================

1. CAD数据导入 - 处理从其他系统导入的不规则边数据
2. 网格重建 - 从三角网格边界重建Wire
3. 轮廓提取 - 从复杂几何体提取轮廓线
4. 路径规划 - 在复杂环境中规划连续路径

*/