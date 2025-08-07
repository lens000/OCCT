// Copyright (c) 2024 OCCT Face Separator Example
// 
// 面分离器示例程序 - Face Separator Example Program

#include "face_separator.hxx"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>

#include <iostream>

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

        // 示例3：创建一个球体并分离其面
        // Example 3: Create a sphere and separate its faces
        std::cout << "3. Creating a sphere and separating its faces..." << std::endl;
        std::cout << "3. 创建球体并分离其面..." << std::endl;
        
        BRepPrimAPI_MakeSphere aSphereMaker(8.0);  // 半径8
        TopoDS_Shape aSphere = aSphereMaker.Shape();
        
        // 计算面的数量
        Standard_Integer aSphereFaceCount = FaceSeparator::CountFaces(aSphere);
        std::cout << "   Sphere has " << aSphereFaceCount << " faces." << std::endl;
        std::cout << "   球体有 " << aSphereFaceCount << " 个面。" << std::endl;

        // 分离所有面到向量
        std::vector<TopoDS_Face> aSphereFaceVector = FaceSeparator::SeparateFacesToVector(aSphere, Standard_False);
        std::cout << "   " << aSphereFaceVector.size() << " sphere faces separated (shared geometry)." << std::endl;
        std::cout << "   " << aSphereFaceVector.size() << " 个球体面已分离（共享几何）。" << std::endl;

        // 保存分离的面到文件
        FaceSeparator::SaveSeparatedFacesToFiles(aSphere, "sphere_face_", Standard_True);

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