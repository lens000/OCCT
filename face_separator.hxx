// Copyright (c) 2024 OCCT Face Separator Example
// 
// 面分离器头文件 - Face Separator Header File
// 这个头文件包含用于从实体中分离所有面的类声明

#ifndef _FaceSeparator_HeaderFile
#define _FaceSeparator_HeaderFile

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>
#include <Standard_TypeDef.hxx>

#include <vector>
#include <string>

/**
 * @brief 面分离器类 - 用于从实体中分离所有面
 * Face Separator Class - for separating all faces from a solid
 * 
 * 这个类提供了多种方法来从OCCT几何体中分离面：
 * This class provides multiple methods to separate faces from OCCT geometries:
 * 
 * 1. 分离面到复合体 (Separate faces to compound)
 * 2. 分离面到向量 (Separate faces to vector)
 * 3. 计算面数量 (Count faces)
 * 4. 保存面到文件 (Save faces to files)
 */
class FaceSeparator
{
public:
    /**
     * @brief 从给定的形状中分离所有面
     * Separate all faces from the given shape
     * 
     * 该方法使用TopExp_Explorer遍历输入形状的所有面，并将它们添加到一个复合体中。
     * This method uses TopExp_Explorer to traverse all faces in the input shape 
     * and adds them to a compound.
     * 
     * @param theShape 输入的形状（实体）/ Input shape (solid)
     * @param copyGeometry 是否复制几何数据 / Whether to copy geometry data
     *                     - True: 创建独立的几何副本 / Create independent geometry copies
     *                     - False: 共享原始几何数据 / Share original geometry data
     * @return 包含所有分离面的复合体 / Compound containing all separated faces
     */
    static TopoDS_Compound SeparateAllFaces(const TopoDS_Shape& theShape, 
                                           const Standard_Boolean copyGeometry = Standard_True);

    /**
     * @brief 从给定的形状中分离所有面并返回面的集合
     * Separate all faces from the given shape and return a collection of faces
     * 
     * 该方法与SeparateAllFaces类似，但返回std::vector<TopoDS_Face>以便更方便地访问单个面。
     * This method is similar to SeparateAllFaces but returns std::vector<TopoDS_Face> 
     * for more convenient access to individual faces.
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
     * 该方法快速计算给定形状中包含的面的总数，无需实际分离面。
     * This method quickly counts the total number of faces in the given shape 
     * without actually separating them.
     * 
     * @param theShape 输入的形状 / Input shape
     * @return 面的数量 / Number of faces
     */
    static Standard_Integer CountFaces(const TopoDS_Shape& theShape);

    /**
     * @brief 将分离的面保存到BREP文件
     * Save separated faces to BREP files
     * 
     * 该方法将每个分离的面保存为独立的BREP文件，文件名格式为：prefix + index + ".brep"
     * This method saves each separated face as an individual BREP file with 
     * filename format: prefix + index + ".brep"
     * 
     * @param theShape 输入的形状 / Input shape
     * @param filePrefix 文件前缀 / File prefix (默认为"face_" / default is "face_")
     * @param copyGeometry 是否复制几何数据 / Whether to copy geometry data
     */
    static void SaveSeparatedFacesToFiles(const TopoDS_Shape& theShape,
                                        const std::string& filePrefix = "face_",
                                        const Standard_Boolean copyGeometry = Standard_True);

private:
    // 私有构造函数，防止实例化
    // Private constructor to prevent instantiation
    FaceSeparator() = delete;
    FaceSeparator(const FaceSeparator&) = delete;
    FaceSeparator& operator=(const FaceSeparator&) = delete;
};

#endif // _FaceSeparator_HeaderFile