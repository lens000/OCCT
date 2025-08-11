#include <BRepMAT2d_BisectingLocus.hxx>
#include <BRepMAT2d_Explorer.hxx>
#include <MAT_Node.hxx>
#include <MAT_Graph.hxx>
#include <MAT_DataMapIteratorOfDataMapOfIntegerNode.hxx>
#include <gp_Pnt2d.hxx>
#include <TopoDS_Wire.hxx>
#include <iostream>

/*
 * 获取MAT_Node中的几何点
 * 
 * MAT_Node是OpenCASCADE中用于表示中轴变换(Medial Axis Transform)图中节点的类。
 * 每个节点都有一个几何索引(GeomIndex)，通过这个索引可以获取对应的几何点坐标。
 */

class MAT_NodeGeometryExtractor {
private:
    BRepMAT2d_BisectingLocus bisectingLocus;
    Handle(MAT_Graph) graph;
    
public:
    // 初始化：计算给定线框的中轴变换
    bool Initialize(const TopoDS_Wire& wire) {
        try {
            BRepMAT2d_Explorer explorer(wire);
            
            // 计算中轴变换
            bisectingLocus.Compute(explorer, 1, MAT_Left, GeomAbs_Arc, Standard_False);
            
            if (!bisectingLocus.IsDone()) {
                std::cerr << "Error: Failed to compute bisecting locus" << std::endl;
                return false;
            }
            
            graph = bisectingLocus.Graph();
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception during initialization: " << e.what() << std::endl;
            return false;
        }
    }
    
    // 方法1：通过MAT_Node直接获取几何点
    gp_Pnt2d GetGeometricPoint(const Handle(MAT_Node)& node) const {
        if (node.IsNull()) {
            std::cerr << "Error: Node is null" << std::endl;
            return gp_Pnt2d(0, 0);
        }
        
        // 使用BisectingLocus的GeomElt方法获取节点对应的几何点
        return bisectingLocus.GeomElt(node);
    }
    
    // 方法2：通过GeomIndex获取几何点
    gp_Pnt2d GetGeometricPointByIndex(const Handle(MAT_Node)& node) const {
        if (node.IsNull()) {
            std::cerr << "Error: Node is null" << std::endl;
            return gp_Pnt2d(0, 0);
        }
        
        // 获取节点的几何索引
        Standard_Integer geomIndex = node->GeomIndex();
        
        // 注意：这里需要访问内部的MAT2d_Tool2d来获取点
        // 在实际应用中，通常使用方法1更为直接
        std::cout << "Node geometric index: " << geomIndex << std::endl;
        
        // 使用BisectingLocus的GeomElt方法
        return bisectingLocus.GeomElt(node);
    }
    
    // 获取节点的其他几何信息
    void GetNodeGeometricInfo(const Handle(MAT_Node)& node) const {
        if (node.IsNull()) {
            std::cerr << "Error: Node is null" << std::endl;
            return;
        }
        
        std::cout << "=== 节点几何信息 ===" << std::endl;
        std::cout << "节点索引 (Node Index): " << node->Index() << std::endl;
        std::cout << "几何索引 (Geometric Index): " << node->GeomIndex() << std::endl;
        std::cout << "距离 (Distance): " << node->Distance() << std::endl;
        std::cout << "是否为悬挂节点 (Pending Node): " << (node->PendingNode() ? "是" : "否") << std::endl;
        std::cout << "是否在基本元素上 (On Basic Element): " << (node->OnBasicElt() ? "是" : "否") << std::endl;
        std::cout << "距离是否为无穷 (Infinite Distance): " << (node->Infinite() ? "是" : "否") << std::endl;
        
        // 获取几何点坐标
        gp_Pnt2d point = GetGeometricPoint(node);
        std::cout << "几何点坐标 (Point Coordinates): (" << point.X() << ", " << point.Y() << ")" << std::endl;
        std::cout << "===================" << std::endl;
    }
    
    // 遍历所有节点并获取其几何点
    void ExtractAllNodeGeometry() const {
        if (graph.IsNull()) {
            std::cerr << "Error: Graph is null" << std::endl;
            return;
        }
        
        std::cout << "提取所有节点的几何点..." << std::endl;
        
        // 获取图中的所有节点
        MAT_DataMapOfIntegerNode& nodes = graph->ChangeNodes();
        MAT_DataMapIteratorOfDataMapOfIntegerNode nodeIter(nodes);
        
        int nodeCount = 0;
        while (nodeIter.More()) {
            Handle(MAT_Node) currentNode = nodeIter.Value();
            
            std::cout << "\n--- 节点 " << ++nodeCount << " ---" << std::endl;
            GetNodeGeometricInfo(currentNode);
            
            nodeIter.Next();
        }
        
        std::cout << "\n总共处理了 " << nodeCount << " 个节点" << std::endl;
    }
    
    // 根据节点索引获取几何点
    gp_Pnt2d GetGeometricPointByNodeIndex(Standard_Integer nodeIndex) const {
        if (graph.IsNull()) {
            std::cerr << "Error: Graph is null" << std::endl;
            return gp_Pnt2d(0, 0);
        }
        
        MAT_DataMapOfIntegerNode& nodes = graph->ChangeNodes();
        
        if (nodes.IsBound(nodeIndex)) {
            Handle(MAT_Node) node = nodes.Find(nodeIndex);
            return GetGeometricPoint(node);
        } else {
            std::cerr << "Error: Node with index " << nodeIndex << " not found" << std::endl;
            return gp_Pnt2d(0, 0);
        }
    }
    
    // 获取图的基本信息
    void PrintGraphInfo() const {
        if (graph.IsNull()) {
            std::cerr << "Error: Graph is null" << std::endl;
            return;
        }
        
        std::cout << "=== 图信息 ===" << std::endl;
        std::cout << "节点数量: " << graph->NumberOfNodes() << std::endl;
        std::cout << "弧数量: " << graph->NumberOfArcs() << std::endl;
        std::cout << "基本元素数量: " << graph->NumberOfBasicElts() << std::endl;
        std::cout << "=============" << std::endl;
    }
};

// 使用示例
void ExampleUsage() {
    std::cout << "MAT_Node几何点提取示例" << std::endl;
    std::cout << "=====================" << std::endl;
    
    // 注意：这里需要一个有效的TopoDS_Wire对象
    // TopoDS_Wire wire = ...; // 您的线框对象
    
    MAT_NodeGeometryExtractor extractor;
    
    // 1. 初始化
    // if (!extractor.Initialize(wire)) {
    //     std::cerr << "初始化失败" << std::endl;
    //     return;
    // }
    
    // 2. 打印图信息
    // extractor.PrintGraphInfo();
    
    // 3. 提取所有节点的几何点
    // extractor.ExtractAllNodeGeometry();
    
    // 4. 根据节点索引获取特定节点的几何点
    // gp_Pnt2d point = extractor.GetGeometricPointByNodeIndex(1);
    // std::cout << "节点1的几何点: (" << point.X() << ", " << point.Y() << ")" << std::endl;
    
    std::cout << "\n=== 关键要点 ===" << std::endl;
    std::cout << "1. MAT_Node通过GeomIndex()方法提供几何索引" << std::endl;
    std::cout << "2. 使用BRepMAT2d_BisectingLocus的GeomElt()方法获取实际几何点" << std::endl;
    std::cout << "3. 几何点以gp_Pnt2d类型返回，包含X和Y坐标" << std::endl;
    std::cout << "4. 节点的Distance()方法返回到最近边界的距离" << std::endl;
    std::cout << "5. 可以通过节点的状态方法判断节点的特殊属性" << std::endl;
}

int main() {
    ExampleUsage();
    return 0;
}