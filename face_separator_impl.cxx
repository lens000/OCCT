// Copyright (c) 2024 OCCT Face Separator Example
// 
// 面分离器实现文件 - Face Separator Implementation File

#include "face_separator.hxx"

#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepTools.hxx>

#include <iostream>
#include <fstream>

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