#ifndef MAT_NODE_GEOMETRY_H
#define MAT_NODE_GEOMETRY_H

#include <MAT_Node.hxx>
#include <BRepMAT2d_BisectingLocus.hxx>
#include <gp_Pnt2d.hxx>

/*
 * 获取MAT_Node几何点的核心方法
 * Core methods for extracting geometric points from MAT_Node
 */

// 方法1：通过BRepMAT2d_BisectingLocus获取几何点（推荐）
// Method 1: Get geometric point via BRepMAT2d_BisectingLocus (Recommended)
inline gp_Pnt2d GetNodeGeometryPoint(const Handle(MAT_Node)& node, 
                                     const BRepMAT2d_BisectingLocus& bisectingLocus) {
    if (node.IsNull()) {
        return gp_Pnt2d(0, 0);
    }
    return bisectingLocus.GeomElt(node);
}

// 方法2：获取节点的几何索引
// Method 2: Get the geometric index of the node
inline Standard_Integer GetNodeGeometryIndex(const Handle(MAT_Node)& node) {
    if (node.IsNull()) {
        return -1;
    }
    return node->GeomIndex();
}

// 方法3：获取节点的详细信息
// Method 3: Get detailed node information
struct NodeGeometryInfo {
    Standard_Integer nodeIndex;      // 节点索引
    Standard_Integer geomIndex;      // 几何索引
    Standard_Real distance;          // 距离
    gp_Pnt2d point;                 // 几何点坐标
    Standard_Boolean isPending;      // 是否为悬挂节点
    Standard_Boolean onBasicElt;     // 是否在基本元素上
    Standard_Boolean isInfinite;     // 距离是否为无穷
};

inline NodeGeometryInfo GetCompleteNodeInfo(const Handle(MAT_Node)& node,
                                           const BRepMAT2d_BisectingLocus& bisectingLocus) {
    NodeGeometryInfo info;
    
    if (node.IsNull()) {
        info.nodeIndex = -1;
        info.geomIndex = -1;
        info.distance = 0.0;
        info.point = gp_Pnt2d(0, 0);
        info.isPending = Standard_False;
        info.onBasicElt = Standard_False;
        info.isInfinite = Standard_False;
        return info;
    }
    
    info.nodeIndex = node->Index();
    info.geomIndex = node->GeomIndex();
    info.distance = node->Distance();
    info.point = bisectingLocus.GeomElt(node);
    info.isPending = node->PendingNode();
    info.onBasicElt = node->OnBasicElt();
    info.isInfinite = node->Infinite();
    
    return info;
}

/*
 * 使用说明：
 * Usage Instructions:
 * 
 * 1. 首先创建BRepMAT2d_BisectingLocus对象并完成计算
 *    First create a BRepMAT2d_BisectingLocus object and complete computation
 * 
 * 2. 从MAT_Graph中获取MAT_Node对象
 *    Get MAT_Node objects from MAT_Graph
 * 
 * 3. 调用上述函数获取几何点坐标
 *    Call the above functions to get geometric point coordinates
 * 
 * 示例代码：
 * Example code:
 * 
 * BRepMAT2d_BisectingLocus bisectingLocus;
 * // ... 初始化和计算 bisectingLocus ...
 * 
 * Handle(MAT_Node) node = ...; // 从graph中获取
 * gp_Pnt2d point = GetNodeGeometryPoint(node, bisectingLocus);
 * 
 * std::cout << "Point coordinates: (" << point.X() << ", " << point.Y() << ")" << std::endl;
 */

#endif // MAT_NODE_GEOMETRY_H