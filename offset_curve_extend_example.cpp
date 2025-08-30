#include <iostream>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <Geom_Curve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomLib.hxx>
#include <gp_Circ.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomConvert.hxx>
#include <Standard_Version.hxx>

// 演示如何在OpenCascade中偏移曲线时延伸偏移的曲线
class OffsetCurveExtender
{
public:
    // 方法1: 使用GeomLib::ExtendCurveToPoint延伸曲线
    static Handle(Geom_Curve) ExtendCurveToPoint(
        const Handle(Geom_Curve)& theCurve,
        const gp_Pnt& theTargetPoint,
        const Standard_Integer theContinuity = 1,
        const Standard_Boolean theAfter = Standard_True)
    {
        // 检查曲线是否为有界曲线
        Handle(Geom_BoundedCurve) aBoundedCurve = 
            Handle(Geom_BoundedCurve)::DownCast(theCurve);
        
        if (aBoundedCurve.IsNull())
        {
            std::cout << "警告: 曲线不是有界曲线，无法延伸" << std::endl;
            return theCurve;
        }
        
        try
        {
            // 使用GeomLib::ExtendCurveToPoint延伸曲线
            // theContinuity: 1=G1连续, 2=G2连续, 3=G3连续
            // theAfter: true=向后延伸, false=向前延伸
            GeomLib::ExtendCurveToPoint(aBoundedCurve, theTargetPoint, theContinuity, theAfter);
            return aBoundedCurve;
        }
        catch (const Standard_Failure& e)
        {
            std::cout << "延伸曲线失败: " << e.GetMessageString() << std::endl;
            return theCurve;
        }
    }
    
    // 方法2: 创建偏移曲线并延伸
    static Handle(Geom_Curve) CreateOffsetCurveWithExtension(
        const Handle(Geom_Curve)& theBaseCurve,
        const Standard_Real theOffset,
        const gp_Dir& theDirection,
        const Standard_Real theExtensionLength = 0.0)
    {
        try
        {
            // 创建偏移曲线
            Handle(Geom_OffsetCurve) anOffsetCurve = 
                new Geom_OffsetCurve(theBaseCurve, theOffset, theDirection);
            
            if (theExtensionLength <= 0.0)
            {
                return anOffsetCurve;
            }
            
            // 获取偏移曲线的参数范围
            Standard_Real f, l;
            anOffsetCurve->Bounds(f, l);
            
            // 计算延伸点
            gp_Pnt startPoint, endPoint;
            gp_Vec startTangent, endTangent;
            
            anOffsetCurve->D1(f, startPoint, startTangent);
            anOffsetCurve->D1(l, endPoint, endTangent);
            
            // 计算延伸后的目标点
            gp_Pnt extendedStartPoint = startPoint.Translated(-startTangent.Normalized() * theExtensionLength);
            gp_Pnt extendedEndPoint = endPoint.Translated(endTangent.Normalized() * theExtensionLength);
            
            // 延伸曲线
            Handle(Geom_Curve) extendedCurve = anOffsetCurve;
            
            // 向前延伸
            if (theExtensionLength > 0.0)
            {
                extendedCurve = ExtendCurveToPoint(extendedCurve, extendedStartPoint, 1, Standard_False);
            }
            
            // 向后延伸
            if (theExtensionLength > 0.0)
            {
                extendedCurve = ExtendCurveToPoint(extendedCurve, extendedEndPoint, 1, Standard_True);
            }
            
            return extendedCurve;
        }
        catch (const Standard_Failure& e)
        {
            std::cout << "创建偏移曲线失败: " << e.GetMessageString() << std::endl;
            return theBaseCurve;
        }
    }
    
    // 方法3: 使用BRepOffsetAPI_MakeOffset进行拓扑偏移并处理延伸
    static TopoDS_Shape CreateOffsetWireWithExtension(
        const TopoDS_Wire& theWire,
        const Standard_Real theOffset,
        const Standard_Real theExtensionLength = 0.0)
    {
        try
        {
            // 创建偏移算法
            BRepOffsetAPI_MakeOffset anOffsetAlgo(theWire);
            
            // 执行偏移
            anOffsetAlgo.Perform(theOffset);
            
            if (!anOffsetAlgo.IsDone())
            {
                std::cout << "偏移操作失败" << std::endl;
                return TopoDS_Shape();
            }
            
            TopoDS_Shape offsetShape = anOffsetAlgo.Shape();
            
            // 如果需要延伸，处理每个边
            if (theExtensionLength > 0.0)
            {
                // 这里可以添加更复杂的延伸逻辑
                // 例如，遍历偏移后的边，对每条边进行延伸
                std::cout << "偏移完成，偏移距离: " << theOffset << std::endl;
                std::cout << "注意: 拓扑偏移的延伸需要更复杂的处理逻辑" << std::endl;
            }
            
            return offsetShape;
        }
        catch (const Standard_Failure& e)
        {
            std::cout << "拓扑偏移失败: " << e.GetMessageString() << std::endl;
            return TopoDS_Shape();
        }
    }
    
    // 方法4: 创建自定义的偏移和延伸算法
    static Handle(Geom_Curve) CreateCustomOffsetWithExtension(
        const Handle(Geom_Curve)& theBaseCurve,
        const Standard_Real theOffset,
        const gp_Dir& theDirection,
        const Standard_Real theExtensionLength = 0.0)
    {
        try
        {
            // 步骤1: 创建偏移曲线
            Handle(Geom_OffsetCurve) anOffsetCurve = 
                new Geom_OffsetCurve(theBaseCurve, theOffset, theDirection);
            
            // 步骤2: 转换为B样条曲线以便更好地控制
            Handle(Geom_BSplineCurve) aBSplineCurve = 
                GeomConvert::CurveToBSplineCurve(anOffsetCurve);
            
            // 步骤3: 如果需要延伸，计算延伸参数
            if (theExtensionLength > 0.0)
            {
                Standard_Real f, l;
                aBSplineCurve->Bounds(f, l);
                
                // 计算延伸后的参数范围
                Standard_Real extendedF = f - theExtensionLength;
                Standard_Real extendedL = l + theExtensionLength;
                
                // 创建延伸后的曲线
                Handle(Geom_TrimmedCurve) anExtendedCurve = 
                    new Geom_TrimmedCurve(aBSplineCurve, extendedF, extendedL);
                
                return anExtendedCurve;
            }
            
            return aBSplineCurve;
        }
        catch (const Standard_Failure& e)
        {
            std::cout << "自定义偏移延伸失败: " << e.GetMessageString() << std::endl;
            return theBaseCurve;
        }
    }
};

// 主函数演示
int main()
{
    std::cout << "OpenCascade版本: " << OCC_VERSION_MAJOR << "." 
              << OCC_VERSION_MINOR << "." << OCC_VERSION_MAINTENANCE << std::endl;
    std::cout << "=== OpenCascade曲线偏移延伸示例 ===" << std::endl;
    
    try
    {
        // 示例1: 创建基础圆曲线
        std::cout << "\n1. 创建基础圆曲线..." << std::endl;
        gp_Circ aCircle(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), 5.0);
        Handle(Geom_Circle) aBaseCurve = new Geom_Circle(aCircle);
        
        // 示例2: 使用GeomLib::ExtendCurveToPoint延伸曲线
        std::cout << "\n2. 使用GeomLib::ExtendCurveToPoint延伸曲线..." << std::endl;
        gp_Pnt extensionPoint(10, 0, 0);
        Handle(Geom_Curve) extendedCurve = 
            OffsetCurveExtender::ExtendCurveToPoint(aBaseCurve, extensionPoint, 1, Standard_True);
        
        if (extendedCurve != aBaseCurve)
        {
            std::cout << "曲线延伸成功" << std::endl;
        }
        
        // 示例3: 创建偏移曲线并延伸
        std::cout << "\n3. 创建偏移曲线并延伸..." << std::endl;
        gp_Dir offsetDirection(0, 0, 1);
        Standard_Real offsetDistance = 2.0;
        Standard_Real extensionLength = 3.0;
        
        Handle(Geom_Curve) offsetExtendedCurve = 
            OffsetCurveExtender::CreateOffsetCurveWithExtension(
                aBaseCurve, offsetDistance, offsetDirection, extensionLength);
        
        if (offsetExtendedCurve != aBaseCurve)
        {
            std::cout << "偏移曲线创建并延伸成功" << std::endl;
        }
        
        // 示例4: 创建自定义偏移延伸
        std::cout << "\n4. 创建自定义偏移延伸..." << std::endl;
        Handle(Geom_Curve) customOffsetCurve = 
            OffsetCurveExtender::CreateCustomOffsetWithExtension(
                aBaseCurve, offsetDistance, offsetDirection, extensionLength);
        
        if (customOffsetCurve != aBaseCurve)
        {
            std::cout << "自定义偏移延伸成功" << std::endl;
        }
        
        // 示例5: 创建拓扑偏移（Wire）
        std::cout << "\n5. 创建拓扑偏移..." << std::endl;
        BRepBuilderAPI_MakeEdge edgeMaker(aBaseCurve);
        TopoDS_Edge anEdge = edgeMaker.Edge();
        
        BRepBuilderAPI_MakeWire wireMaker;
        wireMaker.Add(anEdge);
        TopoDS_Wire aWire = wireMaker.Wire();
        
        TopoDS_Shape offsetWire = 
            OffsetCurveExtender::CreateOffsetWireWithExtension(aWire, offsetDistance, extensionLength);
        
        if (!offsetWire.IsNull())
        {
            std::cout << "拓扑偏移成功" << std::endl;
        }
        
        std::cout << "\n=== 所有示例执行完成 ===" << std::endl;
    }
    catch (const Standard_Failure& e)
    {
        std::cout << "错误: " << e.GetMessageString() << std::endl;
        return 1;
    }
    
    return 0;
}