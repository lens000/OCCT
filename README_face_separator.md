# OCCT Face Separator / OCCT 面分离器

这个项目演示了如何使用Open CASCADE Technology (OCCT)从三维实体中分离所有面。
This project demonstrates how to separate all faces from 3D solids using Open CASCADE Technology (OCCT).

## 功能特性 / Features

- **面分离到复合体** / **Separate faces to compound**: 将所有面合并到一个TopoDS_Compound中
- **面分离到向量** / **Separate faces to vector**: 将所有面存储在std::vector<TopoDS_Face>中
- **面计数** / **Face counting**: 快速计算几何体中的面数量
- **文件保存** / **File saving**: 将分离的面保存为独立的BREP文件
- **几何复制选项** / **Geometry copy options**: 支持复制几何数据或共享引用

## 核心类 / Core Class

### FaceSeparator

静态工具类，提供面分离的各种方法：
Static utility class providing various methods for face separation:

#### 主要方法 / Main Methods

```cpp
// 分离所有面到复合体 / Separate all faces to compound
static TopoDS_Compound SeparateAllFaces(
    const TopoDS_Shape& theShape, 
    const Standard_Boolean copyGeometry = Standard_True
);

// 分离所有面到向量 / Separate all faces to vector
static std::vector<TopoDS_Face> SeparateFacesToVector(
    const TopoDS_Shape& theShape,
    const Standard_Boolean copyGeometry = Standard_True
);

// 计算面数量 / Count faces
static Standard_Integer CountFaces(const TopoDS_Shape& theShape);

// 保存面到文件 / Save faces to files
static void SaveSeparatedFacesToFiles(
    const TopoDS_Shape& theShape,
    const std::string& filePrefix = "face_",
    const Standard_Boolean copyGeometry = Standard_True
);
```

## 文件结构 / File Structure

```
workspace/
├── face_separator.hxx              # 头文件 / Header file
├── face_separator_impl.cxx         # 实现文件 / Implementation file
├── face_separator_example.cxx      # 示例程序 / Example program
├── face_separator.cxx              # 完整的单文件实现 / Complete single-file implementation
├── CMakeLists_face_separator.txt   # CMake构建文件 / CMake build file
└── README_face_separator.md        # 说明文档 / Documentation
```

## 依赖要求 / Dependencies

- **OCCT (Open CASCADE Technology)** 7.0 或更高版本 / 7.0 or higher
- **CMake** 3.16 或更高版本 / 3.16 or higher
- **C++14** 兼容的编译器 / compatible compiler

## 构建说明 / Build Instructions

### 1. 确保已安装OCCT / Ensure OCCT is installed

```bash
# Ubuntu/Debian
sudo apt-get install libocct-dev

# CentOS/RHEL
sudo yum install opencascade-devel

# macOS (使用Homebrew / using Homebrew)
brew install opencascade
```

### 2. 构建项目 / Build the project

```bash
# 创建构建目录 / Create build directory
mkdir build && cd build

# 配置CMake / Configure CMake
cmake ..

# 编译 / Compile
make
# 或者 / or
cmake --build .
```

### 3. 运行示例 / Run examples

```bash
# 运行分离的示例程序 / Run separated example program
./bin/face_separator_example

# 运行完整的示例程序 / Run complete example program
./bin/face_separator_full
```

## 使用示例 / Usage Examples

### 基本用法 / Basic Usage

```cpp
#include "face_separator.hxx"
#include <BRepPrimAPI_MakeBox.hxx>

int main()
{
    // 创建一个长方体 / Create a box
    BRepPrimAPI_MakeBox aBoxMaker(10.0, 15.0, 20.0);
    TopoDS_Shape aBox = aBoxMaker.Shape();
    
    // 计算面数量 / Count faces
    Standard_Integer faceCount = FaceSeparator::CountFaces(aBox);
    std::cout << "Box has " << faceCount << " faces." << std::endl;
    
    // 分离所有面 / Separate all faces
    TopoDS_Compound separatedFaces = FaceSeparator::SeparateAllFaces(aBox);
    
    // 分离面到向量 / Separate faces to vector
    std::vector<TopoDS_Face> faceVector = FaceSeparator::SeparateFacesToVector(aBox);
    
    // 保存面到文件 / Save faces to files
    FaceSeparator::SaveSeparatedFacesToFiles(aBox, "box_face_");
    
    return 0;
}
```

### 几何复制选项 / Geometry Copy Options

```cpp
// 复制几何数据（默认）/ Copy geometry data (default)
TopoDS_Compound copied = FaceSeparator::SeparateAllFaces(shape, Standard_True);

// 共享几何数据 / Share geometry data
TopoDS_Compound shared = FaceSeparator::SeparateAllFaces(shape, Standard_False);
```

## 核心技术原理 / Core Technical Principles

### 1. TopExp_Explorer

使用OCCT的TopExp_Explorer类遍历几何体的拓扑结构：
Uses OCCT's TopExp_Explorer class to traverse the topological structure:

```cpp
for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next())
{
    const TopoDS_Face& aFace = TopoDS::Face(anExp.Current());
    // 处理面 / Process face
}
```

### 2. BRepBuilderAPI_Copy

使用BRepBuilderAPI_Copy创建几何对象的副本：
Uses BRepBuilderAPI_Copy to create copies of geometric objects:

```cpp
BRepBuilderAPI_Copy aCopyMaker(aFace, Standard_True, Standard_False);
TopoDS_Shape aCopiedFace = aCopyMaker.Shape();
```

### 3. BRep_Builder

使用BRep_Builder构建复合体：
Uses BRep_Builder to construct compounds:

```cpp
BRep_Builder aBuilder;
TopoDS_Compound aCompound;
aBuilder.MakeCompound(aCompound);
aBuilder.Add(aCompound, face);
```

## 支持的几何体类型 / Supported Geometry Types

- **实体** / **Solids**: 立方体、圆柱体、球体等 / Boxes, cylinders, spheres, etc.
- **壳体** / **Shells**: 开放或闭合的壳体 / Open or closed shells
- **复合体** / **Compounds**: 包含多个形状的复合对象 / Compound objects containing multiple shapes
- **任何包含面的形状** / **Any shape containing faces**

## 输出格式 / Output Formats

### 1. TopoDS_Compound
所有分离的面合并在一个复合体中
All separated faces combined in a single compound

### 2. std::vector<TopoDS_Face>
每个面作为向量中的独立元素
Each face as an independent element in a vector

### 3. BREP文件
每个面保存为独立的.brep文件
Each face saved as an independent .brep file

## 性能考虑 / Performance Considerations

- **几何复制** / **Geometry copying**: 复制几何数据会增加内存使用 / Copying geometry increases memory usage
- **几何共享** / **Geometry sharing**: 共享几何数据更节省内存但可能有依赖关系 / Sharing geometry saves memory but may have dependencies
- **大型模型** / **Large models**: 对于包含大量面的模型，建议分批处理 / For models with many faces, consider batch processing

## 错误处理 / Error Handling

程序包含基本的错误处理机制：
The program includes basic error handling:

- 文件写入错误检查 / File writing error checking
- 异常捕获和报告 / Exception catching and reporting
- 参数验证 / Parameter validation

## 扩展功能 / Extended Features

可以基于这个基础实现扩展以下功能：
You can extend this implementation with the following features:

- **面过滤** / **Face filtering**: 按几何类型、面积等条件过滤面 / Filter faces by geometry type, area, etc.
- **面分析** / **Face analysis**: 计算面积、法向量等属性 / Calculate area, normal vectors, etc.
- **可视化** / **Visualization**: 集成OCCT的可视化功能 / Integrate OCCT visualization capabilities
- **并行处理** / **Parallel processing**: 多线程处理大型模型 / Multi-threaded processing for large models

## 常见问题 / FAQ

### Q: 如何处理复杂的CAD模型？ / How to handle complex CAD models?
A: 对于复杂模型，建议先使用CountFaces检查面数量，然后选择合适的处理方法。
For complex models, recommend using CountFaces to check face count first, then choose appropriate processing method.

### Q: copyGeometry参数什么时候设为false？ / When to set copyGeometry to false?
A: 当您只需要引用面而不修改几何数据时，或者当内存使用是关键考虑因素时。
When you only need references to faces without modifying geometry, or when memory usage is a critical concern.

### Q: 输出的BREP文件可以用什么软件打开？ / What software can open the output BREP files?
A: BREP文件可以用FreeCAD、Open CASCADE工具、或任何支持OCCT格式的CAD软件打开。
BREP files can be opened with FreeCAD, Open CASCADE tools, or any CAD software supporting OCCT format.

## 许可证 / License

本项目使用与OCCT相同的LGPL 2.1许可证。
This project uses the same LGPL 2.1 license as OCCT.

## 贡献 / Contributing

欢迎提交问题报告和改进建议！
Issues and improvement suggestions are welcome!