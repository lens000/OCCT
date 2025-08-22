#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>

// OCCT includes
#include <gp_Pnt.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Compound.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRep_Builder.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepLib.hxx>
#include <BRep_Tool.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>

/**
 * 边排序和多环检测工具类
 * Edge Sorting and Multi-Loop Detection Utility
 */
class EdgeSortingUtility
{
public:
    /**
     * 边连接信息结构
     * Edge connectivity information structure
     */
    struct EdgeConnection {
        TopoDS_Edge edge;
        gp_Pnt startPoint;
        gp_Pnt endPoint;
        int groupId;        // 所属组ID
        bool processed;     // 是否已处理
        
        EdgeConnection(const TopoDS_Edge& e) : edge(e), groupId(-1), processed(false) {
            TopoDS_Vertex v1, v2;
            TopExp::Vertices(e, v1, v2);
            startPoint = BRep_Tool::Pnt(v1);
            endPoint = BRep_Tool::Pnt(v2);
        }
    };
    
    /**
     * Wire组信息
     * Wire group information
     */
    struct WireGroup {
        std::vector<TopoDS_Edge> edges;
        bool isClosed;
        gp_Pnt startPoint;
        gp_Pnt endPoint;
        
        WireGroup() : isClosed(false) {}
    };

private:
    std::vector<EdgeConnection> edges;
    std::vector<WireGroup> wireGroups;
    Standard_Real tolerance;

public:
    EdgeSortingUtility(Standard_Real tol = 1e-6) : tolerance(tol) {}
    
    /**
     * 添加边进行分析
     * Add edges for analysis
     */
    void AddEdges(const TopTools_ListOfShape& edgeList) {
        edges.clear();
        for (TopTools_ListIteratorOfListOfShape it(edgeList); it.More(); it.Next()) {
            if (it.Value().ShapeType() == TopAbs_EDGE) {
                edges.emplace_back(TopoDS::Edge(it.Value()));
            }
        }
        std::cout << "Added " << edges.size() << " edges for analysis" << std::endl;
    }
    
    /**
     * 检查两点是否重合
     * Check if two points are coincident
     */
    bool ArePointsCoincident(const gp_Pnt& p1, const gp_Pnt& p2) const {
        return p1.Distance(p2) <= tolerance;
    }
    
    /**
     * 查找与指定点连接的边
     * Find edges connected to specified point
     */
    std::vector<int> FindConnectedEdges(const gp_Pnt& point, int excludeIndex = -1) {
        std::vector<int> connectedEdges;
        
        for (size_t i = 0; i < edges.size(); ++i) {
            if ((int)i == excludeIndex || edges[i].processed) continue;
            
            if (ArePointsCoincident(point, edges[i].startPoint) ||
                ArePointsCoincident(point, edges[i].endPoint)) {
                connectedEdges.push_back((int)i);
            }
        }
        
        return connectedEdges;
    }
    
    /**
     * 使用广度优先搜索分析连通性
     * Analyze connectivity using breadth-first search
     */
    void AnalyzeConnectivity() {
        wireGroups.clear();
        
        // 重置处理状态
        for (auto& edge : edges) {
            edge.processed = false;
            edge.groupId = -1;
        }
        
        int currentGroupId = 0;
        
        for (size_t i = 0; i < edges.size(); ++i) {
            if (edges[i].processed) continue;
            
            // 开始新的连通组
            WireGroup newGroup;
            std::queue<int> edgeQueue;
            edgeQueue.push((int)i);
            
            while (!edgeQueue.empty()) {
                int currentEdgeIndex = edgeQueue.front();
                edgeQueue.pop();
                
                if (edges[currentEdgeIndex].processed) continue;
                
                // 标记为已处理
                edges[currentEdgeIndex].processed = true;
                edges[currentEdgeIndex].groupId = currentGroupId;
                newGroup.edges.push_back(edges[currentEdgeIndex].edge);
                
                // 查找连接的边
                std::vector<int> connectedToStart = FindConnectedEdges(edges[currentEdgeIndex].startPoint, currentEdgeIndex);
                std::vector<int> connectedToEnd = FindConnectedEdges(edges[currentEdgeIndex].endPoint, currentEdgeIndex);
                
                // 添加连接的边到队列
                for (int idx : connectedToStart) {
                    if (!edges[idx].processed) {
                        edgeQueue.push(idx);
                    }
                }
                for (int idx : connectedToEnd) {
                    if (!edges[idx].processed) {
                        edgeQueue.push(idx);
                    }
                }
            }
            
            wireGroups.push_back(newGroup);
            currentGroupId++;
        }
        
        std::cout << "Found " << wireGroups.size() << " connected groups" << std::endl;
    }
    
    /**
     * 对每个组内的边进行排序
     * Sort edges within each group
     */
    void SortEdgesInGroups() {
        for (auto& group : wireGroups) {
            if (group.edges.size() <= 1) {
                continue; // 单个边无需排序
            }
            
            std::vector<TopoDS_Edge> sortedEdges;
            std::vector<bool> used(group.edges.size(), false);
            
            // 选择起始边
            sortedEdges.push_back(group.edges[0]);
            used[0] = true;
            
            TopoDS_Vertex currentVertex;
            TopExp::Vertices(group.edges[0], currentVertex, currentVertex, Standard_True); // 获取结束顶点
            gp_Pnt currentPoint = BRep_Tool::Pnt(currentVertex);
            group.startPoint = currentPoint;
            
            // 按连接顺序排序
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
                     << (group.isClosed ? "closed" : "open") << std::endl;
        }
    }
    
    /**
     * 构建所有Wire
     * Build all wires
     */
    std::vector<TopoDS_Wire> BuildAllWires() {
        std::vector<TopoDS_Wire> resultWires;
        
        // 分析连通性
        AnalyzeConnectivity();
        
        // 排序边
        SortEdgesInGroups();
        
        // 为每个组创建Wire
        for (const auto& group : wireGroups) {
            if (group.edges.empty()) continue;
            
            BRepBuilderAPI_MakeWire wireBuilder;
            
            for (const auto& edge : group.edges) {
                wireBuilder.Add(edge);
                if (!wireBuilder.IsDone()) {
                    std::cout << "Failed to add edge to wire in group" << std::endl;
                    break;
                }
            }
            
            if (wireBuilder.IsDone()) {
                TopoDS_Wire wire = wireBuilder.Wire();
                BRepLib::BuildCurves3d(wire);
                resultWires.push_back(wire);
                
                std::cout << "✓ Created " << (group.isClosed ? "closed" : "open") 
                         << " wire with " << group.edges.size() << " edges" << std::endl;
            }
        }
        
        return resultWires;
    }
    
    /**
     * 获取连通组信息
     * Get connectivity group information
     */
    const std::vector<WireGroup>& GetWireGroups() const {
        return wireGroups;
    }
    
    /**
     * 打印分析结果
     * Print analysis results
     */
    void PrintAnalysisResults() {
        std::cout << "\n=== 分析结果 / Analysis Results ===" << std::endl;
        std::cout << "Total edges: " << edges.size() << std::endl;
        std::cout << "Connected groups: " << wireGroups.size() << std::endl;
        
        for (size_t i = 0; i < wireGroups.size(); ++i) {
            const auto& group = wireGroups[i];
            std::cout << "Group " << i + 1 << ": " 
                     << group.edges.size() << " edges, "
                     << (group.isClosed ? "closed" : "open") << std::endl;
        }
    }
};

/**
 * 实用函数：快速创建多环Wire
 * Utility function: Quick multi-loop wire creation
 */
std::vector<TopoDS_Wire> QuickCreateMultiLoopWires(const TopTools_ListOfShape& edges, 
                                                   Standard_Real tolerance = 1e-6)
{
    EdgeSortingUtility utility(tolerance);
    utility.AddEdges(edges);
    std::vector<TopoDS_Wire> wires = utility.BuildAllWires();
    utility.PrintAnalysisResults();
    return wires;
}

/**
 * 测试函数
 * Test function
 */
void TestMultiLoopCreation()
{
    std::cout << "=== 多环Wire创建测试 / Multi-Loop Wire Creation Test ===" << std::endl;
    
    // 创建测试数据：三个独立的几何形状
    TopTools_ListOfShape allEdges;
    
    // 矩形1
    gp_Pnt r1p1(0, 0, 0), r1p2(5, 0, 0), r1p3(5, 5, 0), r1p4(0, 5, 0);
    allEdges.Append(BRepBuilderAPI_MakeEdge(r1p1, r1p2));
    allEdges.Append(BRepBuilderAPI_MakeEdge(r1p2, r1p3));
    allEdges.Append(BRepBuilderAPI_MakeEdge(r1p3, r1p4));
    allEdges.Append(BRepBuilderAPI_MakeEdge(r1p4, r1p1));
    
    // 三角形
    gp_Pnt t1(10, 0, 0), t2(15, 0, 0), t3(12.5, 4.33, 0);
    allEdges.Append(BRepBuilderAPI_MakeEdge(t1, t2));
    allEdges.Append(BRepBuilderAPI_MakeEdge(t2, t3));
    allEdges.Append(BRepBuilderAPI_MakeEdge(t3, t1));
    
    // 开放的折线
    gp_Pnt l1(20, 0, 0), l2(25, 0, 0), l3(25, 3, 0), l4(27, 3, 0);
    allEdges.Append(BRepBuilderAPI_MakeEdge(l1, l2));
    allEdges.Append(BRepBuilderAPI_MakeEdge(l2, l3));
    allEdges.Append(BRepBuilderAPI_MakeEdge(l3, l4));
    
    // 使用工具类创建Wire
    std::vector<TopoDS_Wire> wires = QuickCreateMultiLoopWires(allEdges);
    
    std::cout << "\n最终结果: 成功创建 " << wires.size() << " 个Wire" << std::endl;
    std::cout << "Final result: Successfully created " << wires.size() << " wires" << std::endl;
}

int main()
{
    std::cout << "OCCT边排序和多环检测工具 / OCCT Edge Sorting and Multi-Loop Detection Tool" << std::endl;
    std::cout << "============================================================================" << std::endl;
    
    TestMultiLoopCreation();
    
    std::cout << "\n=== 使用指南 / Usage Guide ===" << std::endl;
    std::cout << "1. 创建EdgeSortingUtility实例" << std::endl;
    std::cout << "2. 使用AddEdges()添加边列表" << std::endl;
    std::cout << "3. 调用BuildAllWires()自动分析和构建" << std::endl;
    std::cout << "4. 获取结果Wire列表" << std::endl;
    
    std::cout << "\n1. Create EdgeSortingUtility instance" << std::endl;
    std::cout << "2. Use AddEdges() to add edge list" << std::endl;
    std::cout << "3. Call BuildAllWires() for automatic analysis and building" << std::endl;
    std::cout << "4. Get result wire list" << std::endl;
    
    return 0;
}

/*
高级特性 / Advanced Features:
=============================

1. 自动连通性分析 - 使用图论算法分析边的连接关系
2. 智能边排序 - 自动将边按正确顺序排列
3. 多环检测 - 识别独立的环和开放的路径
4. 容差处理 - 处理几何精度问题
5. 错误恢复 - 在部分连接失败时提供最佳结果

1. Automatic connectivity analysis - Use graph algorithms to analyze edge connections
2. Intelligent edge sorting - Automatically arrange edges in correct order
3. Multi-loop detection - Identify independent loops and open paths
4. Tolerance handling - Handle geometric precision issues
5. Error recovery - Provide best results when partial connections fail

实际应用 / Practical Applications:
=================================

// 基本用法
EdgeSortingUtility utility;
utility.AddEdges(myEdgeList);
std::vector<TopoDS_Wire> wires = utility.BuildAllWires();

// 或者使用快速函数
std::vector<TopoDS_Wire> wires = QuickCreateMultiLoopWires(myEdgeList);

*/