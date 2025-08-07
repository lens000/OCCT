# Curve2DTo3DConverter

A comprehensive utility class for converting 2D curves to 3D curves in OpenCASCADE Technology (OCCT) applications.

## Overview

The `Curve2DTo3DConverter` class provides multiple methods to transform 2D parametric curves (`Geom2d_Curve`) into 3D curves (`Geom_Curve`) using various transformation approaches. This is particularly useful in CAD applications where you need to:

- Place 2D sketches on arbitrary planes
- Extrude 2D profiles into 3D space
- Map curves onto surfaces
- Create 3D geometry from 2D templates

## Features

### Supported 2D Curve Types

The converter handles all standard OCCT 2D curve types with optimized conversions:

- **Lines** (`Geom2d_Line`) → `Geom_Line`
- **Circles** (`Geom2d_Circle`) → `Geom_Circle`
- **Ellipses** (`Geom2d_Ellipse`) → `Geom_Ellipse`
- **B-Splines** (`Geom2d_BSplineCurve`) → `Geom_BSplineCurve`
- **Bézier curves** (`Geom2d_BezierCurve`) → `Geom_BezierCurve`
- **Generic curves** → `Geom_BSplineCurve` (through sampling)

### Conversion Methods

#### 1. Plane-Based Conversion

```cpp
// Convert to XY plane (Z = 0)
Handle(Geom_Curve) curve3d = converter.ConvertToXYPlane(curve2d);

// Convert to custom plane
gp_Pln customPlane(gp_Pnt(0,0,5), gp_Dir(0,0,1));
Handle(Geom_Curve) curve3d = converter.ConvertToPlane(curve2d, customPlane);

// Convert using coordinate system
gp_Ax3 coordSystem(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0));
Handle(Geom_Curve) curve3d = converter.ConvertWithCoordSystem(curve2d, coordSystem);
```

#### 2. Extrusion-Based Conversion

```cpp
// Simple placement on XY plane
Handle(Geom_Curve) curve3d = converter.ConvertWithExtrusion(curve2d, gp_Vec(0,0,1), 0.0);

// Extrusion with offset
gp_Vec direction(0, 0, 1);
Standard_Real height = 10.0;
Handle(Geom_Curve) curve3d = converter.ConvertWithExtrusion(curve2d, direction, height);
```

#### 3. Surface Mapping

```cpp
// Map 2D curve onto a surface
Handle(Geom_Surface) surface = new Geom_CylindricalSurface(axis, radius);
Handle(Geom_Curve) curve3d = converter.ConvertOnSurface(curve2d, surface);
```

#### 4. Transformation-Based Conversion

```cpp
// Apply custom transformation
gp_Trsf transformation;
transformation.SetRotation(axis, angle);
Handle(Geom_Curve) curve3d = converter.ConvertWithTransformation(curve2d, transformation);
```

#### 5. Revolution

```cpp
// Revolve around an axis
gp_Ax1 axis(gp_Pnt(0,0,0), gp_Dir(0,1,0));
Standard_Real angle = M_PI / 2; // 90 degrees
Handle(Geom_Curve) curve3d = converter.ConvertWithRevolution(curve2d, axis, angle);
```

## Usage Examples

### Basic Line Conversion

```cpp
#include "Curve2DTo3DConverter.h"
#include <Geom2d_Line.hxx>

// Create a 2D line
gp_Pnt2d origin(0.0, 0.0);
gp_Dir2d direction(1.0, 1.0);
Handle(Geom2d_Line) line2d = new Geom2d_Line(origin, direction);

// Convert to 3D
Curve2DTo3DConverter converter;
Handle(Geom_Curve) line3d = converter.ConvertToXYPlane(line2d);

// Use the 3D line
gp_Pnt point = line3d->Value(5.0); // Point at parameter t=5
```

### Circle on Custom Plane

```cpp
// Create 2D circle
gp_Ax2d ax2d(gp_Pnt2d(0.0, 0.0), gp_Dir2d(1.0, 0.0));
gp_Circ2d circle2d(ax2d, 5.0);
Handle(Geom2d_Circle) geomCircle2d = new Geom2d_Circle(circle2d);

// Define custom plane (45° rotation around X-axis)
gp_Ax3 customCoordSystem(
    gp_Pnt(0, 0, 0), 
    gp_Dir(0, sin(M_PI/4), cos(M_PI/4)),
    gp_Dir(1, 0, 0)
);

// Convert
Curve2DTo3DConverter converter;
Handle(Geom_Curve) circle3d = converter.ConvertWithCoordSystem(geomCircle2d, customCoordSystem);
```

### Helical Curve on Cylinder

```cpp
// Create 2D line (becomes helix when mapped to cylinder)
gp_Pnt2d start(0.0, 0.0);
gp_Pnt2d end(2*M_PI, 10.0); // One revolution, height 10
gp_Dir2d dir((end.XY() - start.XY()).Normalized());
Handle(Geom2d_Line) line2d = new Geom2d_Line(start, dir);

// Create cylindrical surface
gp_Ax3 cylinderAxis(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0));
Handle(Geom_CylindricalSurface) cylinder = new Geom_CylindricalSurface(cylinderAxis, 5.0);

// Convert to helical curve
Curve2DTo3DConverter converter;
Handle(Geom_Curve) helix = converter.ConvertOnSurface(line2d, cylinder);
```

## Class Interface

### Constructors

```cpp
// Default constructor (uses XY plane)
Curve2DTo3DConverter();

// Constructor with reference plane
Curve2DTo3DConverter(const gp_Pln& thePlane);

// Constructor with coordinate system
Curve2DTo3DConverter(const gp_Ax3& theCoordSystem);
```

### Main Conversion Methods

```cpp
// Basic conversions
Handle(Geom_Curve) ConvertToXYPlane(const Handle(Geom2d_Curve)& theCurve2D) const;
Handle(Geom_Curve) ConvertToPlane(const Handle(Geom2d_Curve)& theCurve2D, const gp_Pln& thePlane) const;
Handle(Geom_Curve) ConvertWithCoordSystem(const Handle(Geom2d_Curve)& theCurve2D, const gp_Ax3& theCoordSystem) const;

// Advanced conversions
Handle(Geom_Curve) ConvertWithExtrusion(const Handle(Geom2d_Curve)& theCurve2D, const gp_Vec& theDirection, const Standard_Real theHeight = 0.0) const;
Handle(Geom_Curve) ConvertOnSurface(const Handle(Geom2d_Curve)& theCurve2D, const Handle(Geom_Surface)& theSurface) const;
Handle(Geom_Curve) ConvertWithTransformation(const Handle(Geom2d_Curve)& theCurve2D, const gp_Trsf& theTransformation) const;
Handle(Geom_Curve) ConvertWithRevolution(const Handle(Geom2d_Curve)& theCurve2D, const gp_Ax1& theAxis, const Standard_Real theAngle) const;
```

### Configuration Methods

```cpp
// Default settings
void SetDefaultPlane(const gp_Pln& thePlane);
const gp_Pln& GetDefaultPlane() const;
void SetDefaultCoordSystem(const gp_Ax3& theCoordSystem);
const gp_Ax3& GetDefaultCoordSystem() const;
```

## Building and Integration

### CMake Integration

The converter can be integrated into existing OCCT projects using the provided CMake configuration:

```cmake
# Include the converter
include(CMakeLists_Curve2DTo3D.txt)

# Link with your target
target_link_libraries(your_target PRIVATE Curve2DTo3DConverter)
```

### Required Dependencies

The converter depends on the following OCCT modules:

- `TKernel` - Standard types and utilities
- `TKMath` - Mathematical functions
- `TKG2d` - 2D geometry classes
- `TKG3d` - 3D geometry classes  
- `TKGeomBase` - Base geometry classes
- `TKBRep` - Boundary representation

### Header Includes

```cpp
#include "Curve2DTo3DConverter.h"
#include <Geom2d_Curve.hxx>    // For 2D curves
#include <Geom_Curve.hxx>      // For 3D curves
// Additional includes as needed for specific curve types
```

## Performance Considerations

### Optimized Conversions

The converter provides optimized paths for standard curve types:

- **Analytical curves** (lines, circles, ellipses) are converted exactly
- **B-splines and Bézier curves** preserve their mathematical representation
- **Generic curves** are sampled and approximated with B-splines

### Memory Management

- All returned curves are managed by OCCT's handle system
- Input curves are not modified during conversion
- Memory is automatically managed through reference counting

### Sampling Parameters

For generic curves that require sampling:

- Default: 100 sample points
- Degree: 3 (cubic B-splines)
- Smoothness: C2 continuity where possible

## Error Handling

The converter throws `Standard_NullObject` exceptions for null input parameters:

```cpp
try {
    Handle(Geom_Curve) result = converter.ConvertToXYPlane(curve2d);
} catch (const Standard_NullObject& e) {
    std::cout << "Error: " << e.GetMessageString() << std::endl;
}
```

## Common Use Cases

### CAD Sketching
- Convert 2D sketches to 3D for further modeling operations
- Place profiles on construction planes

### Surface Generation
- Create 3D guide curves for surface creation
- Map parametric curves onto complex surfaces

### Manufacturing
- Convert 2D tool paths to 3D machining paths
- Transform profiles for extrusion operations

### Visualization
- Display 2D designs in 3D space
- Create perspective views of planar geometry

## Future Enhancements

Potential areas for expansion:

- Support for offset operations during conversion
- Batch conversion of curve arrays
- Integration with surface creation tools
- Advanced approximation options for generic curves

## See Also

- [OpenCASCADE Geometry Documentation](https://dev.opencascade.org/doc/overview/html/occt_user_guides__modeling_data.html)
- [2D Geometry Classes (Geom2d)](https://dev.opencascade.org/doc/refman/html/package_geom2d.html)
- [3D Geometry Classes (Geom)](https://dev.opencascade.org/doc/refman/html/package_geom.html)