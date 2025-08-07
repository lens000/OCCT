// Copyright (c) 2024 OCCT Face Separator Example
// 
// 这个示例演示如何使用OCCT将一个实体中的所有面分离出来
// This example demonstrates how to separate all faces from a solid using OCCT

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepTools.hxx>
#include <Standard_TypeDef.hxx>

#include <vector>
#include <iostream>
#include <fstream>

/**
 * @brief 面分离器类 - 用于从实体中分离所有面
 * Face Separator Class - for separating all faces from a solid
 */
class FaceSeparator
{
public:
    /**
     * @brief 从给定的形状中分离所有面
     * Separate all faces from the given shape
     * 
     * @param theShape 输入的形状（实体）/ Input shape (solid)
     * @param copyGeometry 是否复制几何数据 / Whether to copy geometry data
     * @return 包含所有分离面的复合体 / Compound containing all separated faces
     */
    static TopoDS_Compound SeparateAllFaces(const TopoDS_Shape& theShape, 
                                           const Standard_Boolean copyGeometry = Standard_True);

    /**
     * @brief 从给定的形状中分离所有面并返回面的集合
     * Separate all faces from the given shape and return a collection of faces
     * 
     * @param theShape 输入的形状（实体）/ Input shape (solid)
     * @param copyGeometry 是否复制几何数据 / Whether to copy geometry data
     * @return 分离出的面的向量 / Vector of separated faces
     */
    static std::vector<TopoDS_Face> SeparateFacesToVector(const TopoDS_Shape& theShape,
                                                         const Standard_Boolean copyGeometry = Standard_True);

    /**
     * @brief 计算形状中面的数量
     * Count the number of faces in a shape
     * 
     * @param theShape 输入的形状 / Input shape
     * @return 面的数量 / Number of faces
     */
    static Standard_Integer CountFaces(const TopoDS_Shape& theShape);

    /**
     * @brief 将分离的面保存到BREP文件
     * Save separated faces to BREP files
     * 
     * @param theShape 输入的形状 / Input shape
     * @param filePrefix 文件前缀 / File prefix
     * @param copyGeometry 是否复制几何数据 / Whether to copy geometry data
     */
    static void SaveSeparatedFacesToFiles(const TopoDS_Shape& theShape,
                                        const std::string& filePrefix = "face_",
                                        const Standard_Boolean copyGeometry = Standard_True);
};

// 实现 SeparateAllFaces 方法
TopoDS_Compound FaceSeparator::SeparateAllFaces(const TopoDS_Shape& theShape, 
                                               const Standard_Boolean copyGeometry)
{
    // 创建复合体建造器
    BRep_Builder aBuilder;
    TopoDS_Compound aCompound;
    aBuilder.MakeCompound(aCompound);

    // 使用TopExp_Explorer遍历所有面
    for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next())
    {
        const TopoDS_Face& aFace = TopoDS::Face(anExp.Current());
        
        if (copyGeometry)
        {
            // 创建面的副本，包含几何数据
            BRepBuilderAPI_Copy aCopyMaker(aFace, Standard_True, Standard_False);
            TopoDS_Shape aCopiedFace = aCopyMaker.Shape();
            aBuilder.Add(aCompound, aCopiedFace);
        }
        else
        {
            // 直接添加面的引用（共享几何数据）
            aBuilder.Add(aCompound, aFace);
        }
    }

    return aCompound;
}

// 实现 SeparateFacesToVector 方法
std::vector<TopoDS_Face> FaceSeparator::SeparateFacesToVector(const TopoDS_Shape& theShape,
                                                             const Standard_Boolean copyGeometry)
{
    std::vector<TopoDS_Face> aFaceVector;

    // 使用TopExp_Explorer遍历所有面
    for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next())
    {
        const TopoDS_Face& aFace = TopoDS::Face(anExp.Current());
        
        if (copyGeometry)
        {
            // 创建面的副本，包含几何数据
            BRepBuilderAPI_Copy aCopyMaker(aFace, Standard_True, Standard_False);
            TopoDS_Face aCopiedFace = TopoDS::Face(aCopyMaker.Shape());
            aFaceVector.push_back(aCopiedFace);
        }
        else
        {
            // 直接添加面的引用（共享几何数据）
            aFaceVector.push_back(aFace);
        }
    }

    return aFaceVector;
}

// 实现 CountFaces 方法
Standard_Integer FaceSeparator::CountFaces(const TopoDS_Shape& theShape)
{
    Standard_Integer aFaceCount = 0;
    
    for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next())
    {
        aFaceCount++;
    }
    
    return aFaceCount;
}

// 实现 SaveSeparatedFacesToFiles 方法
void FaceSeparator::SaveSeparatedFacesToFiles(const TopoDS_Shape& theShape,
                                             const std::string& filePrefix,
                                             const Standard_Boolean copyGeometry)
{
    Standard_Integer aFaceIndex = 0;

    // 遍历所有面并保存到单独的文件
    for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next())
    {
        const TopoDS_Face& aFace = TopoDS::Face(anExp.Current());
        TopoDS_Shape aShapeToSave;
        
        if (copyGeometry)
        {
            // 创建面的副本
            BRepBuilderAPI_Copy aCopyMaker(aFace, Standard_True, Standard_False);
            aShapeToSave = aCopyMaker.Shape();
        }
        else
        {
            aShapeToSave = aFace;
        }

        // 生成文件名
        std::string aFileName = filePrefix + std::to_string(aFaceIndex) + ".brep";
        
        // 保存到BREP文件
        std::ofstream aFile(aFileName);
        if (aFile.is_open())
        {
            BRepTools::Write(aShapeToSave, aFile);
            aFile.close();
            std::cout << "Face " << aFaceIndex << " saved to: " << aFileName << std::endl;
        }
        else
        {
            std::cerr << "Error: Could not open file " << aFileName << " for writing." << std::endl;
        }
        
        aFaceIndex++;
    }
    
    std::cout << "Total " << aFaceIndex << " faces separated and saved." << std::endl;
}

/**
 * @brief 示例程序主函数
 * Example program main function
 */
int main()
{
    std::cout << "=== OCCT Face Separator Example ===" << std::endl;
    std::cout << "=== OCCT 面分离示例程序 ===" << std::endl << std::endl;

    try
    {
        // 示例1：创建一个长方体并分离其面
        // Example 1: Create a box and separate its faces
        std::cout << "1. Creating a box and separating its faces..." << std::endl;
        std::cout << "1. 创建长方体并分离其面..." << std::endl;
        
        BRepPrimAPI_MakeBox aBoxMaker(10.0, 15.0, 20.0);
        TopoDS_Shape aBox = aBoxMaker.Shape();
        
        // 计算面的数量
        Standard_Integer aFaceCount = FaceSeparator::CountFaces(aBox);
        std::cout << "   Box has " << aFaceCount << " faces." << std::endl;
        std::cout << "   长方体有 " << aFaceCount << " 个面。" << std::endl;

        // 分离所有面到复合体
        TopoDS_Compound aSeparatedFaces = FaceSeparator::SeparateAllFaces(aBox, Standard_True);
        std::cout << "   All faces separated into a compound." << std::endl;
        std::cout << "   所有面已分离到复合体中。" << std::endl;

        // 分离所有面到向量
        std::vector<TopoDS_Face> aFaceVector = FaceSeparator::SeparateFacesToVector(aBox, Standard_True);
        std::cout << "   " << aFaceVector.size() << " faces separated into vector." << std::endl;
        std::cout << "   " << aFaceVector.size() << " 个面已分离到向量中。" << std::endl;

        // 保存分离的面到文件
        FaceSeparator::SaveSeparatedFacesToFiles(aBox, "box_face_", Standard_True);

        std::cout << std::endl;

        // 示例2：创建一个圆柱体并分离其面
        // Example 2: Create a cylinder and separate its faces
        std::cout << "2. Creating a cylinder and separating its faces..." << std::endl;
        std::cout << "2. 创建圆柱体并分离其面..." << std::endl;
        
        BRepPrimAPI_MakeCylinder aCylMaker(5.0, 10.0);  // 半径5，高度10
        TopoDS_Shape aCylinder = aCylMaker.Shape();
        
        // 计算面的数量
        Standard_Integer aCylFaceCount = FaceSeparator::CountFaces(aCylinder);
        std::cout << "   Cylinder has " << aCylFaceCount << " faces." << std::endl;
        std::cout << "   圆柱体有 " << aCylFaceCount << " 个面。" << std::endl;

        // 分离所有面
        TopoDS_Compound aCylSeparatedFaces = FaceSeparator::SeparateAllFaces(aCylinder, Standard_True);
        std::cout << "   All cylinder faces separated into a compound." << std::endl;
        std::cout << "   所有圆柱体面已分离到复合体中。" << std::endl;

        // 保存分离的面到文件
        FaceSeparator::SaveSeparatedFacesToFiles(aCylinder, "cylinder_face_", Standard_True);

        std::cout << std::endl;
        std::cout << "=== Example completed successfully! ===" << std::endl;
        std::cout << "=== 示例程序执行成功！ ===" << std::endl;

    }
    catch (const std::exception& e)
    {
        std::cerr << "Error occurred: " << e.what() << std::endl;
        std::cerr << "发生错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}