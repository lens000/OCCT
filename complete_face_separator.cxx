// ===========================================================================
// OCCT Face Separator - Complete Implementation
// OCCT 面分离器 - 完整实现
// ===========================================================================

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
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepTools.hxx>
#include <Standard_TypeDef.hxx>

// 面属性分析相关头文件
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <BRep_Tool.hxx>
#include <GeomLProp_SLProps.hxx>
#include <Geom_Surface.hxx>

#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

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
     */
    static TopoDS_Compound SeparateAllFaces(const TopoDS_Shape& theShape, 
                                           const Standard_Boolean copyGeometry = Standard_True);

    /**
     * @brief 从给定的形状中分离所有面并返回面的集合
     * Separate all faces from the given shape and return a collection of faces
     */
    static std::vector<TopoDS_Face> SeparateFacesToVector(const TopoDS_Shape& theShape,
                                                         const Standard_Boolean copyGeometry = Standard_True);

    /**
     * @brief 计算形状中面的数量
     * Count the number of faces in a shape
     */
    static Standard_Integer CountFaces(const TopoDS_Shape& theShape);

    /**
     * @brief 将分离的面保存到BREP文件
     * Save separated faces to BREP files
     */
    static void SaveSeparatedFacesToFiles(const TopoDS_Shape& theShape,
                                        const std::string& filePrefix = "face_",
                                        const Standard_Boolean copyGeometry = Standard_True);

    /**
     * @brief 分析面的几何属性
     * Analyze geometric properties of faces
     */
    static void AnalyzeFaceProperties(const TopoDS_Shape& theShape);

    /**
     * @brief 按面积过滤面
     * Filter faces by area
     */
    static std::vector<TopoDS_Face> FilterFacesByArea(const TopoDS_Shape& theShape,
                                                     const Standard_Real minArea,
                                                     const Standard_Real maxArea = -1.0);

private:
    FaceSeparator() = delete;
    FaceSeparator(const FaceSeparator&) = delete;
    FaceSeparator& operator=(const FaceSeparator&) = delete;
};

// ===========================================================================
// 实现部分 / Implementation
// ===========================================================================

TopoDS_Compound FaceSeparator::SeparateAllFaces(const TopoDS_Shape& theShape, 
                                               const Standard_Boolean copyGeometry)
{
    std::cout << "  [FaceSeparator] 开始分离面到复合体..." << std::endl;
    std::cout << "  [FaceSeparator] Starting to separate faces to compound..." << std::endl;

    // 创建复合体建造器
    BRep_Builder aBuilder;
    TopoDS_Compound aCompound;
    aBuilder.MakeCompound(aCompound);

    Standard_Integer faceCount = 0;

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
        faceCount++;
    }

    std::cout << "  [FaceSeparator] 成功分离 " << faceCount << " 个面" << std::endl;
    std::cout << "  [FaceSeparator] Successfully separated " << faceCount << " faces" << std::endl;

    return aCompound;
}

std::vector<TopoDS_Face> FaceSeparator::SeparateFacesToVector(const TopoDS_Shape& theShape,
                                                             const Standard_Boolean copyGeometry)
{
    std::cout << "  [FaceSeparator] 开始分离面到向量..." << std::endl;
    std::cout << "  [FaceSeparator] Starting to separate faces to vector..." << std::endl;

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

    std::cout << "  [FaceSeparator] 成功分离 " << aFaceVector.size() << " 个面到向量" << std::endl;
    std::cout << "  [FaceSeparator] Successfully separated " << aFaceVector.size() << " faces to vector" << std::endl;

    return aFaceVector;
}

Standard_Integer FaceSeparator::CountFaces(const TopoDS_Shape& theShape)
{
    Standard_Integer aFaceCount = 0;
    
    for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next())
    {
        aFaceCount++;
    }
    
    return aFaceCount;
}

void FaceSeparator::SaveSeparatedFacesToFiles(const TopoDS_Shape& theShape,
                                             const std::string& filePrefix,
                                             const Standard_Boolean copyGeometry)
{
    std::cout << "  [FaceSeparator] 开始保存分离的面到文件..." << std::endl;
    std::cout << "  [FaceSeparator] Starting to save separated faces to files..." << std::endl;

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
            std::cout << "    面 " << aFaceIndex << " 已保存到: " << aFileName << std::endl;
            std::cout << "    Face " << aFaceIndex << " saved to: " << aFileName << std::endl;
        }
        else
        {
            std::cerr << "    错误: 无法打开文件 " << aFileName << " 进行写入。" << std::endl;
            std::cerr << "    Error: Could not open file " << aFileName << " for writing." << std::endl;
        }
        
        aFaceIndex++;
    }
    
    std::cout << "  [FaceSeparator] 总共分离并保存了 " << aFaceIndex << " 个面" << std::endl;
    std::cout << "  [FaceSeparator] Total " << aFaceIndex << " faces separated and saved" << std::endl;
}

void FaceSeparator::AnalyzeFaceProperties(const TopoDS_Shape& theShape)
{
    std::cout << "  [FaceSeparator] 开始分析面的几何属性..." << std::endl;
    std::cout << "  [FaceSeparator] Starting to analyze face geometric properties..." << std::endl;
    
    Standard_Integer faceIndex = 0;
    Standard_Real totalArea = 0.0;
    
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "\n  面属性分析结果 / Face Properties Analysis Results:" << std::endl;
    std::cout << "  " << std::setw(5) << "索引" << " | " 
              << std::setw(12) << "面积" << " | " 
              << std::setw(30) << "质心坐标" << " | "
              << std::setw(30) << "法向量" << std::endl;
    std::cout << "  " << std::setw(5) << "Index" << " | " 
              << std::setw(12) << "Area" << " | " 
              << std::setw(30) << "Centroid" << " | "
              << std::setw(30) << "Normal" << std::endl;
    std::cout << "  " << std::string(85, '-') << std::endl;
    
    for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next())
    {
        const TopoDS_Face& aFace = TopoDS::Face(anExp.Current());
        
        // 计算面积
        GProp_GProps aProps;
        BRepGProp::SurfaceProperties(aFace, aProps);
        Standard_Real area = aProps.Mass();
        totalArea += area;
        
        // 获取质心
        gp_Pnt centroid = aProps.CentreOfMass();
        
        // 计算法向量（在质心处）
        Handle(Geom_Surface) aSurface = BRep_Tool::Surface(aFace);
        Standard_Real u1, u2, v1, v2;
        aSurface->Bounds(u1, u2, v1, v2);
        Standard_Real uMid = (u1 + u2) * 0.5;
        Standard_Real vMid = (v1 + v2) * 0.5;
        
        GeomLProp_SLProps aProp(aSurface, uMid, vMid, 1, 1e-6);
        gp_Dir normal;
        if (aProp.IsNormalDefined())
        {
            normal = aProp.Normal();
            // 根据面的方向调整法向量
            if (aFace.Orientation() == TopAbs_REVERSED)
            {
                normal.Reverse();
            }
        }
        
        std::cout << "  " << std::setw(5) << faceIndex << " | " 
                  << std::setw(12) << area << " | "
                  << "(" << std::setw(8) << centroid.X() << ", " 
                  << std::setw(8) << centroid.Y() << ", " 
                  << std::setw(8) << centroid.Z() << ") | "
                  << "(" << std::setw(8) << normal.X() << ", " 
                  << std::setw(8) << normal.Y() << ", " 
                  << std::setw(8) << normal.Z() << ")" << std::endl;
        
        faceIndex++;
    }
    
    std::cout << "  " << std::string(85, '-') << std::endl;
    std::cout << "  总面积 / Total Area: " << totalArea << std::endl;
    std::cout << "  平均面积 / Average Area: " << (faceIndex > 0 ? totalArea / faceIndex : 0.0) << std::endl;
}

std::vector<TopoDS_Face> FaceSeparator::FilterFacesByArea(const TopoDS_Shape& theShape,
                                                         const Standard_Real minArea,
                                                         const Standard_Real maxArea)
{
    std::cout << "  [FaceSeparator] 按面积过滤面..." << std::endl;
    std::cout << "  [FaceSeparator] Filtering faces by area..." << std::endl;
    
    std::vector<TopoDS_Face> filteredFaces;
    Standard_Integer totalFaces = 0;
    
    for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next())
    {
        const TopoDS_Face& aFace = TopoDS::Face(anExp.Current());
        totalFaces++;
        
        // 计算面积
        GProp_GProps aProps;
        BRepGProp::SurfaceProperties(aFace, aProps);
        Standard_Real area = aProps.Mass();
        
        // 检查面积是否在指定范围内
        bool inRange = (area >= minArea);
        if (maxArea > 0.0)
        {
            inRange = inRange && (area <= maxArea);
        }
        
        if (inRange)
        {
            filteredFaces.push_back(aFace);
        }
    }
    
    std::cout << "  从 " << totalFaces << " 个面中过滤出 " << filteredFaces.size() << " 个面" << std::endl;
    std::cout << "  Filtered " << filteredFaces.size() << " faces from " << totalFaces << " total faces" << std::endl;
    
    return filteredFaces;
}

// ===========================================================================
// 示例程序 / Example Program
// ===========================================================================

void demonstrateBoxSeparation()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例1: 长方体面分离 / Example 1: Box Face Separation" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建一个长方体
    BRepPrimAPI_MakeBox aBoxMaker(10.0, 15.0, 20.0);
    TopoDS_Shape aBox = aBoxMaker.Shape();
    
    std::cout << "创建了一个 10×15×20 的长方体" << std::endl;
    std::cout << "Created a 10×15×20 box" << std::endl;
    
    // 计算面的数量
    Standard_Integer aFaceCount = FaceSeparator::CountFaces(aBox);
    std::cout << "长方体包含 " << aFaceCount << " 个面" << std::endl;
    std::cout << "Box contains " << aFaceCount << " faces" << std::endl;

    // 分离所有面到复合体
    TopoDS_Compound aSeparatedFaces = FaceSeparator::SeparateAllFaces(aBox, Standard_True);
    
    // 分离所有面到向量
    std::vector<TopoDS_Face> aFaceVector = FaceSeparator::SeparateFacesToVector(aBox, Standard_True);
    
    // 分析面的属性
    FaceSeparator::AnalyzeFaceProperties(aBox);
    
    // 保存分离的面到文件
    FaceSeparator::SaveSeparatedFacesToFiles(aBox, "box_face_", Standard_True);
}

void demonstrateCylinderSeparation()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例2: 圆柱体面分离 / Example 2: Cylinder Face Separation" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建一个圆柱体 (半径5, 高度10)
    BRepPrimAPI_MakeCylinder aCylMaker(5.0, 10.0);
    TopoDS_Shape aCylinder = aCylMaker.Shape();
    
    std::cout << "创建了一个半径5、高度10的圆柱体" << std::endl;
    std::cout << "Created a cylinder with radius 5 and height 10" << std::endl;
    
    // 计算面的数量
    Standard_Integer aCylFaceCount = FaceSeparator::CountFaces(aCylinder);
    std::cout << "圆柱体包含 " << aCylFaceCount << " 个面" << std::endl;
    std::cout << "Cylinder contains " << aCylFaceCount << " faces" << std::endl;

    // 分离所有面
    TopoDS_Compound aCylSeparatedFaces = FaceSeparator::SeparateAllFaces(aCylinder, Standard_True);
    
    // 分析面的属性
    FaceSeparator::AnalyzeFaceProperties(aCylinder);
    
    // 保存分离的面到文件
    FaceSeparator::SaveSeparatedFacesToFiles(aCylinder, "cylinder_face_", Standard_True);
}

void demonstrateSphereSeparation()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例3: 球体面分离 / Example 3: Sphere Face Separation" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建一个球体 (半径8)
    BRepPrimAPI_MakeSphere aSphereMaker(8.0);
    TopoDS_Shape aSphere = aSphereMaker.Shape();
    
    std::cout << "创建了一个半径8的球体" << std::endl;
    std::cout << "Created a sphere with radius 8" << std::endl;
    
    // 计算面的数量
    Standard_Integer aSphereFaceCount = FaceSeparator::CountFaces(aSphere);
    std::cout << "球体包含 " << aSphereFaceCount << " 个面" << std::endl;
    std::cout << "Sphere contains " << aSphereFaceCount << " faces" << std::endl;

    // 分离所有面到向量（使用共享几何）
    std::vector<TopoDS_Face> aSphereFaceVector = FaceSeparator::SeparateFacesToVector(aSphere, Standard_False);
    
    // 分析面的属性
    FaceSeparator::AnalyzeFaceProperties(aSphere);
    
    // 保存分离的面到文件
    FaceSeparator::SaveSeparatedFacesToFiles(aSphere, "sphere_face_", Standard_True);
}

void demonstrateAdvancedFeatures()
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "示例4: 高级功能演示 / Example 4: Advanced Features" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 创建一个长方体用于演示高级功能
    BRepPrimAPI_MakeBox aBoxMaker(20.0, 30.0, 40.0);
    TopoDS_Shape aBox = aBoxMaker.Shape();
    
    std::cout << "创建了一个 20×30×40 的长方体用于高级功能演示" << std::endl;
    std::cout << "Created a 20×30×40 box for advanced features demonstration" << std::endl;
    
    // 演示面积过滤
    std::cout << "\n面积过滤演示 / Area Filtering Demonstration:" << std::endl;
    
    // 获取面积大于500的面
    std::vector<TopoDS_Face> largeFaces = FaceSeparator::FilterFacesByArea(aBox, 500.0);
    std::cout << "面积大于500的面数量: " << largeFaces.size() << std::endl;
    std::cout << "Number of faces with area > 500: " << largeFaces.size() << std::endl;
    
    // 获取面积在500-1000之间的面
    std::vector<TopoDS_Face> mediumFaces = FaceSeparator::FilterFacesByArea(aBox, 500.0, 1000.0);
    std::cout << "面积在500-1000之间的面数量: " << mediumFaces.size() << std::endl;
    std::cout << "Number of faces with area between 500-1000: " << mediumFaces.size() << std::endl;
}

int main()
{
    std::cout << "====================================================" << std::endl;
    std::cout << "OCCT Face Separator - Complete Implementation" << std::endl;
    std::cout << "OCCT 面分离器 - 完整实现" << std::endl;
    std::cout << "====================================================" << std::endl;

    try
    {
        // 运行所有示例
        demonstrateBoxSeparation();
        demonstrateCylinderSeparation();
        demonstrateSphereSeparation();
        demonstrateAdvancedFeatures();
        
        std::cout << "\n====================================================" << std::endl;
        std::cout << "所有示例执行完成！/ All examples completed successfully!" << std::endl;
        std::cout << "====================================================" << std::endl;
        
        // 输出文件说明
        std::cout << "\n生成的文件 / Generated Files:" << std::endl;
        std::cout << "- box_face_*.brep      : 长方体的面文件 / Box face files" << std::endl;
        std::cout << "- cylinder_face_*.brep : 圆柱体的面文件 / Cylinder face files" << std::endl;
        std::cout << "- sphere_face_*.brep   : 球体的面文件 / Sphere face files" << std::endl;
        std::cout << "\n这些BREP文件可以在FreeCAD或其他支持OCCT的CAD软件中打开查看。" << std::endl;
        std::cout << "These BREP files can be opened in FreeCAD or other CAD software supporting OCCT." << std::endl;

    }
    catch (const std::exception& e)
    {
        std::cerr << "发生错误 / Error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}