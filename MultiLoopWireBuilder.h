#ifndef MULTILOOPWIREBUILDER_H
#define MULTILOOPWIREBUILDER_H

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
#include <TopTools_ListOfShape.hxx>
#include <BRepBuilderAPI_WireError.hxx>

/**
 * @brief 多环Wire构建器 - 处理复杂的边连接情况
 * Multi-Loop Wire Builder - Handles complex edge connectivity cases
 * 
 * 这个类能够：
 * This class can:
 * - 自动分析边的连接关系 / Automatically analyze edge connectivity
 * - 将边分组为独立的连通组件 / Group edges into independent connected components  
 * - 自动排序边以形成连续的路径 / Automatically sort edges to form continuous paths
 * - 检测闭合环和开放路径 / Detect closed loops and open paths
 * - 处理多个独立的环 / Handle multiple independent loops
 */
class MultiLoopWireBuilder
{
public:
    /**
     * @brief 边连接信息结构
     * Edge connectivity information structure
     */
    struct EdgeInfo {
        TopoDS_Edge edge;           ///< OCCT边对象
        gp_Pnt startPoint;          ///< 起点坐标
        gp_Pnt endPoint;            ///< 终点坐标
        int groupId;                ///< 所属连通组ID
        bool processed;             ///< 是否已处理
        
        EdgeInfo(const TopoDS_Edge& e);
    };
    
    /**
     * @brief Wire组信息结构
     * Wire group information structure
     */
    struct WireGroup {
        std::vector<TopoDS_Edge> edges;  ///< 组内的边列表
        bool isClosed;                   ///< 是否为闭合环
        gp_Pnt startPoint;               ///< 起点（对于开放Wire）
        gp_Pnt endPoint;                 ///< 终点（对于开放Wire）
        
        WireGroup();
    };

private:
    std::vector<EdgeInfo> m_edges;      ///< 边信息列表
    std::vector<WireGroup> m_wireGroups; ///< Wire组列表
    Standard_Real m_tolerance;           ///< 几何容差

public:
    /**
     * @brief 构造函数
     * Constructor
     * @param tolerance 几何容差，用于判断顶点是否重合
     */
    explicit MultiLoopWireBuilder(Standard_Real tolerance = 1e-6);
    
    /**
     * @brief 析构函数
     * Destructor
     */
    ~MultiLoopWireBuilder() = default;
    
    /**
     * @brief 添加单个边
     * Add single edge
     * @param edge 要添加的边
     */
    void AddEdge(const TopoDS_Edge& edge);
    
    /**
     * @brief 添加边列表
     * Add edge list
     * @param edgeList OCCT边列表
     */
    void AddEdges(const TopTools_ListOfShape& edgeList);
    
    /**
     * @brief 添加边向量
     * Add edge vector
     * @param edges 边的向量容器
     */
    void AddEdges(const std::vector<TopoDS_Edge>& edges);
    
    /**
     * @brief 分析边的连通性并分组
     * Analyze edge connectivity and group them
     * @return 成功返回true
     */
    bool AnalyzeConnectivity();
    
    /**
     * @brief 对每个组内的边进行排序
     * Sort edges within each group
     * @return 成功返回true
     */
    bool SortEdgesInGroups();
    
    /**
     * @brief 构建所有可能的Wire
     * Build all possible wires
     * @return Wire向量
     */
    std::vector<TopoDS_Wire> BuildAllWires();
    
    /**
     * @brief 使用OCCT内置算法构建Wire
     * Build wires using OCCT built-in algorithms
     * @return Wire向量
     */
    std::vector<TopoDS_Wire> BuildWiresUsingFreeBounds();
    
    /**
     * @brief 获取连通组数量
     * Get number of connected groups
     * @return 组数量
     */
    size_t GetGroupCount() const { return m_wireGroups.size(); }
    
    /**
     * @brief 获取边数量
     * Get number of edges
     * @return 边数量
     */
    size_t GetEdgeCount() const { return m_edges.size(); }
    
    /**
     * @brief 获取连通组信息
     * Get connectivity group information
     * @return Wire组信息的常量引用
     */
    const std::vector<WireGroup>& GetWireGroups() const { return m_wireGroups; }
    
    /**
     * @brief 重置构建器状态
     * Reset builder state
     */
    void Reset();
    
    /**
     * @brief 设置几何容差
     * Set geometric tolerance
     * @param tolerance 新的容差值
     */
    void SetTolerance(Standard_Real tolerance) { m_tolerance = tolerance; }
    
    /**
     * @brief 获取当前容差
     * Get current tolerance
     * @return 当前容差值
     */
    Standard_Real GetTolerance() const { return m_tolerance; }
    
    /**
     * @brief 打印详细的分析结果
     * Print detailed analysis results
     */
    void PrintAnalysisResults() const;
    
    /**
     * @brief 验证Wire的有效性
     * Validate wire validity
     * @param wire 要验证的Wire
     * @return 有效返回true
     */
    static bool ValidateWire(const TopoDS_Wire& wire);

private:
    /**
     * @brief 检查两点是否在容差范围内重合
     * Check if two points are coincident within tolerance
     * @param p1 第一个点
     * @param p2 第二个点
     * @return 重合返回true
     */
    bool ArePointsCoincident(const gp_Pnt& p1, const gp_Pnt& p2) const;
    
    /**
     * @brief 查找与指定点连接的边索引
     * Find edge indices connected to specified point
     * @param point 指定点
     * @param excludeIndex 要排除的边索引
     * @return 连接的边索引向量
     */
    std::vector<int> FindConnectedEdges(const gp_Pnt& point, int excludeIndex = -1);
    
    /**
     * @brief 从指定边开始构建连续路径
     * Build continuous path starting from specified edge
     * @param startEdgeIndex 起始边索引
     * @param group 输出的Wire组
     * @return 成功返回true
     */
    bool BuildPathFromEdge(int startEdgeIndex, WireGroup& group);
};

/**
 * @brief 便利函数：快速从边列表创建多个Wire
 * Convenience function: Quickly create multiple wires from edge list
 * @param edges 边列表
 * @param tolerance 几何容差
 * @return Wire向量
 */
std::vector<TopoDS_Wire> CreateMultiLoopWires(const TopTools_ListOfShape& edges, 
                                              Standard_Real tolerance = 1e-6);

/**
 * @brief 便利函数：从边向量创建多个Wire
 * Convenience function: Create multiple wires from edge vector
 * @param edges 边向量
 * @param tolerance 几何容差
 * @return Wire向量
 */
std::vector<TopoDS_Wire> CreateMultiLoopWires(const std::vector<TopoDS_Edge>& edges,
                                              Standard_Real tolerance = 1e-6);

#endif // MULTILOOPWIREBUILDER_H

/*
使用示例 / Usage Example:
========================

#include "MultiLoopWireBuilder.h"

int main() {
    // 创建边列表
    TopTools_ListOfShape edges;
    // ... 添加边到列表 ...
    
    // 方法1: 使用类
    MultiLoopWireBuilder builder;
    builder.AddEdges(edges);
    std::vector<TopoDS_Wire> wires = builder.BuildAllWires();
    builder.PrintAnalysisResults();
    
    // 方法2: 使用便利函数
    std::vector<TopoDS_Wire> wires2 = CreateMultiLoopWires(edges);
    
    return 0;
}

关键特性 / Key Features:
=======================

1. 自动连通性分析 - 使用广度优先搜索算法
2. 智能边排序 - 确保边按正确顺序连接
3. 多环支持 - 同时处理多个独立的环
4. 容差处理 - 处理几何精度问题
5. 详细的错误报告和分析结果

1. Automatic connectivity analysis - Using breadth-first search algorithm
2. Intelligent edge sorting - Ensure edges are connected in correct order
3. Multi-loop support - Handle multiple independent loops simultaneously
4. Tolerance handling - Handle geometric precision issues
5. Detailed error reporting and analysis results
*/