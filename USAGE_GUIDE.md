# OCCT Face Separator 使用指南 / Usage Guide

## 快速开始 / Quick Start

### 1. 检查依赖 / Check Dependencies

```bash
# 检查OCCT安装 / Check OCCT installation
make check-deps

# 查看帮助 / View help
make help
```

### 2. 编译程序 / Compile Programs

```bash
# 编译所有程序 / Compile all programs
make all

# 或者只编译完整版本 / Or compile only complete version
make complete_face_separator
```

### 3. 运行示例 / Run Examples

```bash
# 运行完整示例程序 / Run complete example program
make run-complete

# 或者直接运行 / Or run directly
./complete_face_separator
```

## 详细功能说明 / Detailed Feature Description

### 核心类：FaceSeparator

这是一个静态工具类，提供以下主要功能：

#### 1. 面分离到复合体 / Separate Faces to Compound

```cpp
TopoDS_Compound FaceSeparator::SeparateAllFaces(
    const TopoDS_Shape& theShape, 
    const Standard_Boolean copyGeometry = Standard_True
);
```

**功能说明 / Function Description:**
- 将输入形状的所有面提取并组合成一个复合体
- Extract all faces from input shape and combine into a compound

**参数 / Parameters:**
- `theShape`: 输入的几何形状 / Input geometric shape
- `copyGeometry`: 是否复制几何数据 / Whether to copy geometry data
  - `Standard_True`: 创建独立副本 / Create independent copies
  - `Standard_False`: 共享原始数据 / Share original data

**使用示例 / Usage Example:**
```cpp
BRepPrimAPI_MakeBox boxMaker(10.0, 15.0, 20.0);
TopoDS_Shape box = boxMaker.Shape();

// 分离所有面 / Separate all faces
TopoDS_Compound separatedFaces = FaceSeparator::SeparateAllFaces(box);
```

#### 2. 面分离到向量 / Separate Faces to Vector

```cpp
std::vector<TopoDS_Face> FaceSeparator::SeparateFacesToVector(
    const TopoDS_Shape& theShape,
    const Standard_Boolean copyGeometry = Standard_True
);
```

**功能说明 / Function Description:**
- 将所有面分离并存储在STL向量中，便于单独访问每个面
- Separate all faces and store in STL vector for individual access

**使用示例 / Usage Example:**
```cpp
std::vector<TopoDS_Face> faces = FaceSeparator::SeparateFacesToVector(box);

// 遍历每个面 / Iterate through each face
for (size_t i = 0; i < faces.size(); ++i) {
    const TopoDS_Face& face = faces[i];
    // 处理单个面 / Process individual face
}
```

#### 3. 面计数 / Face Counting

```cpp
Standard_Integer FaceSeparator::CountFaces(const TopoDS_Shape& theShape);
```

**功能说明 / Function Description:**
- 快速计算几何体中包含的面数量
- Quickly count the number of faces in a geometry

**使用示例 / Usage Example:**
```cpp
Standard_Integer faceCount = FaceSeparator::CountFaces(box);
std::cout << "该几何体包含 " << faceCount << " 个面" << std::endl;
```

#### 4. 保存面到文件 / Save Faces to Files

```cpp
void FaceSeparator::SaveSeparatedFacesToFiles(
    const TopoDS_Shape& theShape,
    const std::string& filePrefix = "face_",
    const Standard_Boolean copyGeometry = Standard_True
);
```

**功能说明 / Function Description:**
- 将每个面保存为独立的BREP文件
- Save each face as an individual BREP file

**参数 / Parameters:**
- `filePrefix`: 文件名前缀 / Filename prefix
- 生成的文件名格式：`{prefix}{index}.brep`

**使用示例 / Usage Example:**
```cpp
// 保存为 box_face_0.brep, box_face_1.brep, ...
FaceSeparator::SaveSeparatedFacesToFiles(box, "box_face_");
```

#### 5. 面属性分析 / Face Properties Analysis

```cpp
void FaceSeparator::AnalyzeFaceProperties(const TopoDS_Shape& theShape);
```

**功能说明 / Function Description:**
- 分析每个面的几何属性：面积、质心、法向量
- Analyze geometric properties of each face: area, centroid, normal vector

**输出信息 / Output Information:**
- 面索引 / Face index
- 面积 / Area
- 质心坐标 / Centroid coordinates
- 法向量 / Normal vector
- 总面积和平均面积 / Total and average area

#### 6. 面积过滤 / Area Filtering

```cpp
std::vector<TopoDS_Face> FaceSeparator::FilterFacesByArea(
    const TopoDS_Shape& theShape,
    const Standard_Real minArea,
    const Standard_Real maxArea = -1.0
);
```

**功能说明 / Function Description:**
- 根据面积范围过滤面
- Filter faces by area range

**参数 / Parameters:**
- `minArea`: 最小面积 / Minimum area
- `maxArea`: 最大面积（-1表示无上限）/ Maximum area (-1 for no limit)

## 实际应用示例 / Practical Application Examples

### 示例1：分析CAD模型的面

```cpp
#include "face_separator.hxx"
#include <BRepTools.hxx>

int main() {
    // 从文件加载CAD模型 / Load CAD model from file
    TopoDS_Shape shape;
    std::ifstream file("model.brep");
    if (file.is_open()) {
        BRepTools::Read(shape, file);
        file.close();
        
        // 分析面属性 / Analyze face properties
        FaceSeparator::AnalyzeFaceProperties(shape);
        
        // 找出大面积的面 / Find large area faces
        std::vector<TopoDS_Face> largeFaces = 
            FaceSeparator::FilterFacesByArea(shape, 100.0);
        
        std::cout << "找到 " << largeFaces.size() << " 个大面积面" << std::endl;
    }
    
    return 0;
}
```

### 示例2：批量处理面

```cpp
// 分离所有面 / Separate all faces
std::vector<TopoDS_Face> faces = FaceSeparator::SeparateFacesToVector(shape);

// 对每个面进行特定处理 / Process each face specifically
for (size_t i = 0; i < faces.size(); ++i) {
    const TopoDS_Face& face = faces[i];
    
    // 计算面积 / Calculate area
    GProp_GProps props;
    BRepGProp::SurfaceProperties(face, props);
    Standard_Real area = props.Mass();
    
    // 根据面积进行不同处理 / Different processing based on area
    if (area > 500.0) {
        // 处理大面 / Process large faces
        std::string filename = "large_face_" + std::to_string(i) + ".brep";
        std::ofstream outFile(filename);
        BRepTools::Write(face, outFile);
        outFile.close();
    }
}
```

## 文件分析工具 / File Analysis Tool

使用附带的Python脚本分析生成的BREP文件：

```bash
# 基本分析 / Basic analysis
python3 analyze_faces.py

# 验证文件完整性 / Validate file integrity
python3 analyze_faces.py --validate

# 清理所有BREP文件 / Clean up all BREP files
python3 analyze_faces.py --cleanup

# 强制清理（无需确认）/ Force cleanup (no confirmation)
python3 analyze_faces.py --cleanup --force

# 分析特定模式的文件 / Analyze specific pattern files
python3 analyze_faces.py --pattern "box_face_*.brep"
```

## 性能优化建议 / Performance Optimization Tips

### 1. 几何复制 vs 共享 / Geometry Copy vs Share

**何时使用复制 / When to use copying:**
- 需要修改分离后的面 / Need to modify separated faces
- 原始几何体会被删除 / Original geometry will be deleted
- 需要长期保存面数据 / Need to preserve face data long-term

**何时使用共享 / When to use sharing:**
- 只需要读取面信息 / Only need to read face information
- 内存使用是关键考虑 / Memory usage is critical
- 原始几何体保持不变 / Original geometry remains unchanged

```cpp
// 复制模式（安全但占用更多内存）/ Copy mode (safe but more memory)
TopoDS_Compound copied = FaceSeparator::SeparateAllFaces(shape, Standard_True);

// 共享模式（节省内存但有依赖）/ Share mode (memory efficient but has dependencies)
TopoDS_Compound shared = FaceSeparator::SeparateAllFaces(shape, Standard_False);
```

### 2. 大型模型处理 / Large Model Processing

对于包含大量面的复杂模型：

```cpp
// 先检查面数量 / Check face count first
Standard_Integer faceCount = FaceSeparator::CountFaces(largeModel);

if (faceCount > 1000) {
    std::cout << "警告：模型包含大量面 (" << faceCount << ")，处理可能需要时间" << std::endl;
    
    // 考虑分批处理或使用共享模式 / Consider batch processing or shared mode
    std::vector<TopoDS_Face> faces = 
        FaceSeparator::SeparateFacesToVector(largeModel, Standard_False);
}
```

## 错误处理 / Error Handling

### 常见错误和解决方案 / Common Errors and Solutions

1. **编译错误：找不到OCCT头文件**
   ```
   fatal error: TopoDS_Shape.hxx: No such file or directory
   ```
   **解决方案 / Solution:**
   - 确保已安装OCCT开发包 / Ensure OCCT development packages are installed
   - 检查Makefile中的OCCT_INC路径 / Check OCCT_INC path in Makefile

2. **链接错误：找不到OCCT库**
   ```
   undefined reference to `BRep_Builder::MakeCompound`
   ```
   **解决方案 / Solution:**
   - 确保已安装OCCT库 / Ensure OCCT libraries are installed
   - 检查Makefile中的OCCT_LIB路径和库名称 / Check OCCT_LIB path and library names

3. **运行时错误：无法写入文件**
   ```
   Error: Could not open file face_0.brep for writing
   ```
   **解决方案 / Solution:**
   - 检查文件写入权限 / Check file write permissions
   - 确保目录存在 / Ensure directory exists

## 扩展开发 / Extension Development

### 添加新的面过滤条件 / Adding New Face Filtering Conditions

```cpp
// 示例：按面类型过滤 / Example: Filter by face type
std::vector<TopoDS_Face> FaceSeparator::FilterFacesByType(
    const TopoDS_Shape& theShape,
    const GeomAbs_SurfaceType surfaceType)
{
    std::vector<TopoDS_Face> filteredFaces;
    
    for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next()) {
        const TopoDS_Face& aFace = TopoDS::Face(anExp.Current());
        
        Handle(Geom_Surface) surface = BRep_Tool::Surface(aFace);
        GeomAdaptor_Surface adaptor(surface);
        
        if (adaptor.GetType() == surfaceType) {
            filteredFaces.push_back(aFace);
        }
    }
    
    return filteredFaces;
}
```

### 添加面可视化功能 / Adding Face Visualization

```cpp
// 使用AIS_ColoredShape进行可视化 / Use AIS_ColoredShape for visualization
#include <AIS_ColoredShape.hxx>
#include <Quantity_Color.hxx>

void FaceSeparator::VisualizeSeparatedFaces(
    const TopoDS_Shape& theShape,
    Handle(AIS_InteractiveContext) theContext)
{
    Standard_Integer index = 0;
    
    for (TopExp_Explorer anExp(theShape, TopAbs_FACE); anExp.More(); anExp.Next()) {
        const TopoDS_Face& aFace = TopoDS::Face(anExp.Current());
        
        Handle(AIS_ColoredShape) aisShape = new AIS_ColoredShape(aFace);
        
        // 为每个面设置不同颜色 / Set different color for each face
        Quantity_Color color(index * 0.1, 0.5, 0.8, Quantity_TOC_RGB);
        aisShape->SetColor(color);
        
        theContext->Display(aisShape, Standard_False);
        index++;
    }
    
    theContext->UpdateCurrentViewer();
}
```

## 故障排除 / Troubleshooting

### 调试模式编译 / Debug Mode Compilation

```bash
# 使用调试标志编译 / Compile with debug flags
g++ -std=c++14 -g -O0 -DDEBUG -I/usr/include/opencascade \
    complete_face_separator.cxx -o complete_face_separator_debug \
    -lTKernel -lTKMath -lTKBRep -lTKTopAlgo -lTKPrim
```

### 内存泄漏检查 / Memory Leak Check

```bash
# 使用Valgrind检查内存泄漏 / Use Valgrind to check memory leaks
valgrind --leak-check=full --show-leak-kinds=all ./complete_face_separator
```

### 性能分析 / Performance Analysis

```bash
# 使用perf进行性能分析 / Use perf for performance analysis
perf record ./complete_face_separator
perf report
```

这份使用指南涵盖了OCCT面分离器的所有主要功能和使用方法。如果您有任何问题或需要进一步的帮助，请参考代码注释或联系开发者。