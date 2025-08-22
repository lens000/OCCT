#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>

// OCCT includes
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepCheck_Wire.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepLib.hxx>

#include <GC_MakeSegment.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeCircle.hxx>

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <gp_Vec.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>

#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

#include <Geom_TrimmedCurve.hxx>
#include <Geom_Curve.hxx>

// 顶点信息结构
struct VertexInfo {
    gp_Pnt point;
    std::vector<size_t> connectedEdges;
    bool isStart;
    bool isEnd;
    
    VertexInfo() : isStart(false), isEnd(false) {}
};

// Edge信息结构
struct EdgeInfo {
    TopoDS_Edge edge;
    TopoDS_Vertex startVertex;
    TopoDS_Vertex endVertex;
    double length;
    bool isUsed;
    
    EdgeInfo() : isUsed(false) {}
};

// 高级Edge到Wire转换器类
class AdvancedEdgeToWireConverter {
private:
    std::vector<EdgeInfo> edgeInfos;
    std::map<TopoDS_Vertex, VertexInfo> vertexMap;
    double tolerance;
    
public:
    AdvancedEdgeToWireConverter(double tol = 1e-6) : tolerance(tol) {}
    
    // 添加edge到转换器
    void AddEdge(const TopoDS_Edge& edge) {
        EdgeInfo edgeInfo;
        edgeInfo.edge = edge;
        
        // 获取edge的顶点
        TopExp_Explorer vertexExplorer(edge, TopAbs_VERTEX);
        if (vertexExplorer.More()) {
            edgeInfo.startVertex = TopoDS::Vertex(vertexExplorer.Current());
            vertexExplorer.Next();
            if (vertexExplorer.More()) {
                edgeInfo.endVertex = TopoDS::Vertex(vertexExplorer.Current());
            } else {
                edgeInfo.endVertex = edgeInfo.startVertex; // 闭合edge
            }
        }
        
        // 计算edge长度
        edgeInfo.length = CalculateEdgeLength(edge);
        
        edgeInfos.push_back(edgeInfo);
    }
    
    // 构建顶点映射
    void BuildVertexMap() {
        vertexMap.clear();
        
        for (size_t i = 0; i < edgeInfos.size(); ++i) {
            const EdgeInfo& edgeInfo = edgeInfos[i];
            
            // 处理起始顶点
            if (vertexMap.find(edgeInfo.startVertex) == vertexMap.end()) {
                VertexInfo& vInfo = vertexMap[edgeInfo.startVertex];
                vInfo.point = BRep_Tool::Pnt(edgeInfo.startVertex);
                vInfo.isStart = true;
            }
            vertexMap[edgeInfo.startVertex].connectedEdges.push_back(i);
            
            // 处理结束顶点
            if (vertexMap.find(edgeInfo.endVertex) == vertexMap.end()) {
                VertexInfo& vInfo = vertexMap[edgeInfo.endVertex];
                vInfo.point = BRep_Tool::Pnt(edgeInfo.endVertex);
                vInfo.isEnd = true;
            }
            vertexMap[edgeInfo.endVertex].connectedEdges.push_back(i);
        }
    }
    
    // 使用深度优先搜索查找连接的edge序列
    std::vector<size_t> FindConnectedEdgeSequence(size_t startEdgeIndex) {
        std::vector<size_t> sequence;
        std::set<size_t> visited;
        
        std::function<void(size_t)> dfs = [&](size_t edgeIndex) {
            if (visited.find(edgeIndex) != visited.end()) return;
            
            visited.insert(edgeIndex);
            sequence.push_back(edgeIndex);
            
            const EdgeInfo& edgeInfo = edgeInfos[edgeIndex];
            
            // 查找可以连接的edge
            for (size_t nextEdgeIndex : vertexMap[edgeInfo.endVertex].connectedEdges) {
                if (visited.find(nextEdgeIndex) == visited.end()) {
                    const EdgeInfo& nextEdgeInfo = edgeInfos[nextEdgeIndex];
                    
                    // 检查是否可以连接
                    if (CanEdgesConnect(edgeInfo, nextEdgeInfo)) {
                        dfs(nextEdgeIndex);
                        break; // 只取第一个可连接的edge
                    }
                }
            }
        };
        
        dfs(startEdgeIndex);
        return sequence;
    }
    
    // 转换为wires
    std::vector<TopoDS_Wire> ConvertToWires() {
        BuildVertexMap();
        std::vector<TopoDS_Wire> wires;
        
        // 重置使用状态
        for (auto& edgeInfo : edgeInfos) {
            edgeInfo.isUsed = false;
        }
        
        // 查找所有可能的wire
        for (size_t i = 0; i < edgeInfos.size(); ++i) {
            if (edgeInfos[i].isUsed) continue;
            
            std::vector<size_t> sequence = FindConnectedEdgeSequence(i);
            if (sequence.size() > 1) {
                TopoDS_Wire wire = CreateWireFromSequence(sequence);
                if (!wire.IsNull()) {
                    wires.push_back(wire);
                }
                
                // 标记为已使用
                for (size_t edgeIndex : sequence) {
                    edgeInfos[edgeIndex].isUsed = true;
                }
            }
        }
        
        // 处理剩余的单个edge
        for (size_t i = 0; i < edgeInfos.size(); ++i) {
            if (!edgeInfos[i].isUsed) {
                TopoDS_Wire wire = CreateWireFromSingleEdge(i);
                if (!wire.IsNull()) {
                    wires.push_back(wire);
                }
            }
        }
        
        return wires;
    }
    
    // 优化wires（合并小的wire，修复拓扑问题）
    std::vector<TopoDS_Wire> OptimizeWires(const std::vector<TopoDS_Wire>& wires) {
        std::vector<TopoDS_Wire> optimizedWires;
        
        for (const TopoDS_Wire& wire : wires) {
            if (IsWireValid(wire)) {
                TopoDS_Wire optimizedWire = OptimizeWire(wire);
                if (!optimizedWire.IsNull()) {
                    optimizedWires.push_back(optimizedWire);
                }
            }
        }
        
        return optimizedWires;
    }
    
private:
    // 计算edge长度
    double CalculateEdgeLength(const TopoDS_Edge& edge) {
        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
        if (curve.IsNull()) return 0.0;
        
        return curve->Value(last).Distance(curve->Value(first));
    }
    
    // 检查两个edge是否可以连接
    bool CanEdgesConnect(const EdgeInfo& edge1, const EdgeInfo& edge2) {
        // 检查顶点是否重合
        if (AreVerticesCoincident(edge1.endVertex, edge2.startVertex)) {
            return true;
        }
        
        if (AreVerticesCoincident(edge1.endVertex, edge2.endVertex)) {
            return true;
        }
        
        if (AreVerticesCoincident(edge1.startVertex, edge2.startVertex)) {
            return true;
        }
        
        if (AreVerticesCoincident(edge1.startVertex, edge2.endVertex)) {
            return true;
        }
        
        return false;
    }
    
    // 检查两个顶点是否重合
    bool AreVerticesCoincident(const TopoDS_Vertex& v1, const TopoDS_Vertex& v2) {
        gp_Pnt p1 = BRep_Tool::Pnt(v1);
        gp_Pnt p2 = BRep_Tool::Pnt(v2);
        return p1.Distance(p2) < tolerance;
    }
    
    // 从edge序列创建wire
    TopoDS_Wire CreateWireFromSequence(const std::vector<size_t>& sequence) {
        if (sequence.empty()) return TopoDS_Wire();
        
        BRepBuilderAPI_MakeWire wireMaker;
        
        for (size_t edgeIndex : sequence) {
            wireMaker.Add(edgeInfos[edgeIndex].edge);
        }
        
        if (wireMaker.IsDone()) {
            return wireMaker.Wire();
        }
        
        return TopoDS_Wire();
    }
    
    // 从单个edge创建wire
    TopoDS_Wire CreateWireFromSingleEdge(size_t edgeIndex) {
        BRepBuilderAPI_MakeWire wireMaker(edgeInfos[edgeIndex].edge);
        if (wireMaker.IsDone()) {
            return wireMaker.Wire();
        }
        return TopoDS_Wire();
    }
    
    // 检查wire是否有效
    bool IsWireValid(const TopoDS_Wire& wire) {
        BRepCheck_Analyzer analyzer(wire);
        return analyzer.IsValid();
    }
    
    // 优化单个wire
    TopoDS_Wire OptimizeWire(const TopoDS_Wire& wire) {
        // 重建3D曲线
        BRepLib::BuildCurves3d(wire);
        
        // 检查并修复拓扑问题
        BRepCheck_Analyzer analyzer(wire);
        if (!analyzer.IsValid()) {
            // 这里可以添加更多的修复逻辑
            return TopoDS_Wire();
        }
        
        return wire;
    }
};

// 示例：创建复杂的edge集合
std::vector<TopoDS_Edge> CreateComplexEdges() {
    std::vector<TopoDS_Edge> edges;
    
    // 创建多个几何形状
    std::vector<gp_Pnt> points = {
        gp_Pnt(0, 0, 0),    // p1
        gp_Pnt(1, 0, 0),    // p2
        gp_Pnt(1, 1, 0),    // p3
        gp_Pnt(0, 1, 0),    // p4
        gp_Pnt(0.5, 0.5, 0), // p5
        gp_Pnt(2, 0, 0),    // p6
        gp_Pnt(2, 1, 0),    // p7
        gp_Pnt(1.5, 0.5, 0) // p8
    };
    
    // 矩形1的边
    std::vector<std::pair<int, int>> segments1 = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};
    for (const auto& seg : segments1) {
        Handle(Geom_TrimmedCurve) segment = GC_MakeSegment(points[seg.first], points[seg.second]);
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(segment);
        edges.push_back(edge);
    }
    
    // 矩形2的边
    std::vector<std::pair<int, int>> segments2 = {{1, 5}, {5, 6}, {6, 2}};
    for (const auto& seg : segments2) {
        Handle(Geom_TrimmedCurve) segment = GC_MakeSegment(points[seg.first], points[seg.second]);
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(segment);
        edges.push_back(edge);
    }
    
    // 圆弧
    Handle(Geom_TrimmedCurve) arc1 = GC_MakeArcOfCircle(points[0], points[4], points[2]);
    TopoDS_Edge edgeArc1 = BRepBuilderAPI_MakeEdge(arc1);
    edges.push_back(edgeArc1);
    
    Handle(Geom_TrimmedCurve) arc2 = GC_MakeArcOfCircle(points[1], points[7], points[6]);
    TopoDS_Edge edgeArc2 = BRepBuilderAPI_MakeEdge(arc2);
    edges.push_back(edgeArc2);
    
    return edges;
}

// 主函数
int main() {
    try {
        std::cout << "Advanced OCCT Edge to Wire Conversion Example" << std::endl;
        std::cout << "=============================================" << std::endl;
        
        // 创建复杂的edge集合
        std::vector<TopoDS_Edge> edges = CreateComplexEdges();
        std::cout << "Created " << edges.size() << " complex edges" << std::endl;
        
        // 使用高级转换器
        AdvancedEdgeToWireConverter converter(1e-6);
        
        for (const auto& edge : edges) {
            converter.AddEdge(edge);
        }
        
        // 转换为wires
        std::vector<TopoDS_Wire> wires = converter.ConvertToWires();
        std::cout << "Generated " << wires.size() << " initial wires" << std::endl;
        
        // 优化wires
        std::vector<TopoDS_Wire> optimizedWires = converter.OptimizeWires(wires);
        std::cout << "Optimized to " << optimizedWires.size() << " wires" << std::endl;
        
        // 分析结果
        for (size_t i = 0; i < optimizedWires.size(); ++i) {
            const TopoDS_Wire& wire = optimizedWires[i];
            std::cout << "\nWire " << i + 1 << ":" << std::endl;
            
            // 计算edge数量
            TopExp_Explorer edgeExplorer(wire, TopAbs_EDGE);
            int edgeCount = 0;
            while (edgeExplorer.More()) {
                edgeCount++;
                edgeExplorer.Next();
            }
            std::cout << "  - Contains " << edgeCount << " edges" << std::endl;
            
            // 检查wire属性
            if (BRep_Tool::IsClosed(wire)) {
                std::cout << "  - Closed wire" << std::endl;
            } else {
                std::cout << "  - Open wire" << std::endl;
            }
            
            // 验证wire有效性
            BRepCheck_Analyzer analyzer(wire);
            if (analyzer.IsValid()) {
                std::cout << "  - Valid topology" << std::endl;
            } else {
                std::cout << "  - Invalid topology" << std::endl;
            }
        }
        
        std::cout << "\nAdvanced conversion completed successfully!" << std::endl;
        
    } catch (const Standard_Failure& e) {
        std::cerr << "OCCT Error: " << e.GetMessageString() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Standard Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}