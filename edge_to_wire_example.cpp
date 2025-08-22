#include <iostream>
#include <vector>
#include <algorithm>

// OCCT includes
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepCheck_Wire.hxx>
#include <BRepCheck_Analyzer.hxx>

#include <GC_MakeSegment.hxx>
#include <GC_MakeArcOfCircle.hxx>

#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Shape.hxx>

#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

#include <Geom_TrimmedCurve.hxx>

// 辅助函数：获取edge的顶点
void GetEdgeVertices(const TopoDS_Edge& edge, TopoDS_Vertex& firstVertex, TopoDS_Vertex& lastVertex)
{
    TopExp_Explorer vertexExplorer(edge, TopAbs_VERTEX);
    if (vertexExplorer.More()) {
        firstVertex = TopoDS::Vertex(vertexExplorer.Current());
        vertexExplorer.Next();
        if (vertexExplorer.More()) {
            lastVertex = TopoDS::Vertex(vertexExplorer.Current());
        } else {
            lastVertex = firstVertex; // 闭合的edge
        }
    }
}

// 辅助函数：检查两个顶点是否重合
bool AreVerticesCoincident(const TopoDS_Vertex& v1, const TopoDS_Vertex& v2, double tolerance = 1e-6)
{
    gp_Pnt p1 = BRep_Tool::Pnt(v1);
    gp_Pnt p2 = BRep_Tool::Pnt(v2);
    return p1.Distance(p2) < tolerance;
}

// 辅助函数：检查两个edge是否可以连接
bool CanEdgesConnect(const TopoDS_Edge& edge1, const TopoDS_Edge& edge2, double tolerance = 1e-6)
{
    TopoDS_Vertex first1, last1, first2, last2;
    GetEdgeVertices(edge1, first1, last1);
    GetEdgeVertices(edge2, first2, last2);
    
    // 检查edge1的末端是否与edge2的首端重合
    if (AreVerticesCoincident(last1, first2, tolerance)) {
        return true;
    }
    
    // 检查edge1的末端是否与edge2的末端重合
    if (AreVerticesCoincident(last1, last2, tolerance)) {
        return true;
    }
    
    // 检查edge1的首端是否与edge2的首端重合
    if (AreVerticesCoincident(first1, first2, tolerance)) {
        return true;
    }
    
    // 检查edge1的首端是否与edge2的末端重合
    if (AreVerticesCoincident(first1, last2, tolerance)) {
        return true;
    }
    
    return false;
}

// 主要函数：将离散的edge转换为wires
std::vector<TopoDS_Wire> ConvertEdgesToWires(const std::vector<TopoDS_Edge>& edges, double tolerance = 1e-6)
{
    std::vector<TopoDS_Wire> wires;
    std::vector<bool> used(edges.size(), false);
    
    for (size_t i = 0; i < edges.size(); ++i) {
        if (used[i]) continue;
        
        // 开始构建新的wire
        BRepBuilderAPI_MakeWire wireMaker;
        wireMaker.Add(edges[i]);
        used[i] = true;
        
        bool addedEdge = true;
        while (addedEdge) {
            addedEdge = false;
            
            for (size_t j = 0; j < edges.size(); ++j) {
                if (used[j]) continue;
                
                // 检查当前edge是否可以添加到wire中
                if (CanEdgesConnect(edges[j], edges[i], tolerance)) {
                    wireMaker.Add(edges[j]);
                    used[j] = true;
                    addedEdge = true;
                    break;
                }
            }
        }
        
        if (wireMaker.IsDone()) {
            wires.push_back(wireMaker.Wire());
        }
    }
    
    return wires;
}

// 高级函数：使用拓扑分析器检查wire的有效性
bool IsWireValid(const TopoDS_Wire& wire)
{
    BRepCheck_Analyzer analyzer(wire);
    return analyzer.IsValid();
}

// 示例：创建一些离散的edge
std::vector<TopoDS_Edge> CreateSampleEdges()
{
    std::vector<TopoDS_Edge> edges;
    
    // 创建一些线段和圆弧
    gp_Pnt p1(0, 0, 0);
    gp_Pnt p2(1, 0, 0);
    gp_Pnt p3(1, 1, 0);
    gp_Pnt p4(0, 1, 0);
    gp_Pnt p5(0.5, 0.5, 0);
    
    // 线段1: p1 -> p2
    Handle(Geom_TrimmedCurve) segment1 = GC_MakeSegment(p1, p2);
    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(segment1);
    edges.push_back(edge1);
    
    // 线段2: p2 -> p3
    Handle(Geom_TrimmedCurve) segment2 = GC_MakeSegment(p2, p3);
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(segment2);
    edges.push_back(edge2);
    
    // 线段3: p3 -> p4
    Handle(Geom_TrimmedCurve) segment3 = GC_MakeSegment(p3, p4);
    TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(segment3);
    edges.push_back(edge3);
    
    // 线段4: p4 -> p1 (闭合)
    Handle(Geom_TrimmedCurve) segment4 = GC_MakeSegment(p4, p1);
    TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(segment4);
    edges.push_back(edge4);
    
    // 圆弧: 从p1到p3，通过p5
    Handle(Geom_TrimmedCurve) arc = GC_MakeArcOfCircle(p1, p5, p3);
    TopoDS_Edge edge5 = BRepBuilderAPI_MakeEdge(arc);
    edges.push_back(edge5);
    
    return edges;
}

// 主函数
int main()
{
    try {
        std::cout << "OCCT Edge to Wire Conversion Example" << std::endl;
        std::cout << "===================================" << std::endl;
        
        // 创建示例edge
        std::vector<TopoDS_Edge> edges = CreateSampleEdges();
        std::cout << "Created " << edges.size() << " edges" << std::endl;
        
        // 转换为wires
        std::vector<TopoDS_Wire> wires = ConvertEdgesToWires(edges);
        std::cout << "Generated " << wires.size() << " wires" << std::endl;
        
        // 验证每个wire
        for (size_t i = 0; i < wires.size(); ++i) {
            std::cout << "Wire " << i + 1 << ":" << std::endl;
            
            if (IsWireValid(wires[i])) {
                std::cout << "  - Valid wire" << std::endl;
                
                // 计算wire中的edge数量
                TopExp_Explorer edgeExplorer(wires[i], TopAbs_EDGE);
                int edgeCount = 0;
                while (edgeExplorer.More()) {
                    edgeCount++;
                    edgeExplorer.Next();
                }
                std::cout << "  - Contains " << edgeCount << " edges" << std::endl;
                
                // 检查wire是否闭合
                if (BRep_Tool::IsClosed(wires[i])) {
                    std::cout << "  - Closed wire" << std::endl;
                } else {
                    std::cout << "  - Open wire" << std::endl;
                }
            } else {
                std::cout << "  - Invalid wire" << std::endl;
            }
        }
        
        std::cout << "\nConversion completed successfully!" << std::endl;
        
    } catch (const Standard_Failure& e) {
        std::cerr << "OCCT Error: " << e.GetMessageString() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Standard Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}