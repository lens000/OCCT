#include "Curve2DTo3DConverter.h"

#include <Geom2d_Line.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_BezierCurve.hxx>

#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>

#include <gp_Pnt2d.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Elips2d.hxx>
#include <gp_Ax2d.hxx>

#include <TColgp_Array1OfPnt2d.hxx>
#include <TColStd_Array1OfReal.hxx>

#include <iostream>

void Example_ConvertLineToXYPlane()
{
  std::cout << "\n=== Converting 2D Line to 3D Line on XY Plane ===" << std::endl;
  
  // Create a 2D line from (0,0) with direction (1,1)
  gp_Pnt2d origin2d(0.0, 0.0);
  gp_Dir2d direction2d(1.0, 1.0);
  Handle(Geom2d_Line) line2d = new Geom2d_Line(origin2d, direction2d);
  
  // Convert to 3D
  Curve2DTo3DConverter converter;
  Handle(Geom_Curve) line3d = converter.ConvertToXYPlane(line2d);
  
  // Test the conversion
  gp_Pnt pt3d_start = line3d->Value(0.0);
  gp_Pnt pt3d_end = line3d->Value(10.0);
  
  std::cout << "2D Line converted to 3D:" << std::endl;
  std::cout << "Start point: (" << pt3d_start.X() << ", " << pt3d_start.Y() << ", " << pt3d_start.Z() << ")" << std::endl;
  std::cout << "End point: (" << pt3d_end.X() << ", " << pt3d_end.Y() << ", " << pt3d_end.Z() << ")" << std::endl;
}

void Example_ConvertCircleToCustomPlane()
{
  std::cout << "\n=== Converting 2D Circle to 3D Circle on Custom Plane ===" << std::endl;
  
  // Create a 2D circle centered at (5,5) with radius 3
  gp_Ax2d ax2d(gp_Pnt2d(5.0, 5.0), gp_Dir2d(1.0, 0.0));
  gp_Circ2d circle2d(ax2d, 3.0);
  Handle(Geom2d_Circle) geomCircle2d = new Geom2d_Circle(circle2d);
  
  // Create a custom plane (tilted 45 degrees around X axis)
  gp_Ax3 customCoordSystem(gp_Pnt(0, 0, 10), 
                          gp_Dir(0, M_PI/4, M_PI/4).Normalized(),
                          gp_Dir(1, 0, 0));
  gp_Pln customPlane(customCoordSystem);
  
  // Convert to 3D on custom plane
  Curve2DTo3DConverter converter;
  Handle(Geom_Curve) circle3d = converter.ConvertToPlane(geomCircle2d, customPlane);
  
  // Test points on the circle
  std::cout << "2D Circle converted to 3D on custom plane:" << std::endl;
  for (int i = 0; i < 4; i++)
  {
    Standard_Real param = i * M_PI / 2;
    gp_Pnt pt = circle3d->Value(param);
    std::cout << "Point " << i << ": (" << pt.X() << ", " << pt.Y() << ", " << pt.Z() << ")" << std::endl;
  }
}

void Example_ConvertEllipseWithExtrusion()
{
  std::cout << "\n=== Converting 2D Ellipse with Extrusion ===" << std::endl;
  
  // Create a 2D ellipse
  gp_Ax2d ax2d(gp_Pnt2d(0.0, 0.0), gp_Dir2d(1.0, 0.0));
  gp_Elips2d ellipse2d(ax2d, 5.0, 3.0); // Major radius 5, minor radius 3
  Handle(Geom2d_Ellipse) geomEllipse2d = new Geom2d_Ellipse(ellipse2d);
  
  // Convert with extrusion along Z direction
  Curve2DTo3DConverter converter;
  gp_Vec extrusionDirection(0.0, 0.0, 1.0);
  Standard_Real extrusionHeight = 10.0;
  
  Handle(Geom_Curve) ellipse3d = converter.ConvertWithExtrusion(
    geomEllipse2d, extrusionDirection, extrusionHeight);
  
  // Test points on the ellipse
  std::cout << "2D Ellipse converted to 3D with extrusion:" << std::endl;
  for (int i = 0; i < 4; i++)
  {
    Standard_Real param = i * M_PI / 2;
    gp_Pnt pt = ellipse3d->Value(param);
    std::cout << "Point " << i << ": (" << pt.X() << ", " << pt.Y() << ", " << pt.Z() << ")" << std::endl;
  }
}

void Example_ConvertBSplineCurve()
{
  std::cout << "\n=== Converting 2D B-Spline Curve ===" << std::endl;
  
  // Create a 2D B-spline curve
  TColgp_Array1OfPnt2d poles2d(1, 4);
  poles2d(1) = gp_Pnt2d(0.0, 0.0);
  poles2d(2) = gp_Pnt2d(3.0, 4.0);
  poles2d(3) = gp_Pnt2d(6.0, 2.0);
  poles2d(4) = gp_Pnt2d(9.0, 0.0);
  
  TColStd_Array1OfReal knots(1, 2);
  knots(1) = 0.0;
  knots(2) = 1.0;
  
  TColStd_Array1OfInteger multiplicities(1, 2);
  multiplicities(1) = 4;
  multiplicities(2) = 4;
  
  Handle(Geom2d_BSplineCurve) bspline2d = new Geom2d_BSplineCurve(
    poles2d, knots, multiplicities, 3);
  
  // Convert to 3D
  Curve2DTo3DConverter converter;
  Handle(Geom_Curve) bspline3d = converter.ConvertToXYPlane(bspline2d);
  
  // Test points along the curve
  std::cout << "2D B-Spline converted to 3D:" << std::endl;
  Standard_Real firstParam = bspline3d->FirstParameter();
  Standard_Real lastParam = bspline3d->LastParameter();
  
  for (int i = 0; i <= 5; i++)
  {
    Standard_Real t = firstParam + i * (lastParam - firstParam) / 5.0;
    gp_Pnt pt = bspline3d->Value(t);
    std::cout << "Point " << i << " (t=" << t << "): (" 
              << pt.X() << ", " << pt.Y() << ", " << pt.Z() << ")" << std::endl;
  }
}

void Example_ConvertBezierCurve()
{
  std::cout << "\n=== Converting 2D Bezier Curve ===" << std::endl;
  
  // Create a 2D Bezier curve
  TColgp_Array1OfPnt2d poles2d(1, 4);
  poles2d(1) = gp_Pnt2d(0.0, 0.0);
  poles2d(2) = gp_Pnt2d(1.0, 3.0);
  poles2d(3) = gp_Pnt2d(4.0, 3.0);
  poles2d(4) = gp_Pnt2d(5.0, 0.0);
  
  Handle(Geom2d_BezierCurve) bezier2d = new Geom2d_BezierCurve(poles2d);
  
  // Convert to 3D with transformation
  Curve2DTo3DConverter converter;
  
  // Create a transformation (rotation around Y-axis)
  gp_Trsf transformation;
  gp_Ax1 yAxis(gp_Pnt(0,0,0), gp_Dir(0,1,0));
  transformation.SetRotation(yAxis, M_PI / 4); // 45 degrees
  
  Handle(Geom_Curve) bezier3d = converter.ConvertWithTransformation(
    bezier2d, transformation);
  
  // Test points along the curve
  std::cout << "2D Bezier converted to 3D with rotation:" << std::endl;
  for (int i = 0; i <= 5; i++)
  {
    Standard_Real t = i / 5.0;
    gp_Pnt pt = bezier3d->Value(t);
    std::cout << "Point " << i << " (t=" << t << "): (" 
              << pt.X() << ", " << pt.Y() << ", " << pt.Z() << ")" << std::endl;
  }
}

void Example_ConvertOnCylindricalSurface()
{
  std::cout << "\n=== Converting 2D Curve on Cylindrical Surface ===" << std::endl;
  
  // Create a 2D line (will become a helical curve on cylinder)
  gp_Pnt2d start2d(0.0, 0.0);
  gp_Pnt2d end2d(2*M_PI, 10.0); // One full revolution, height 10
  gp_Dir2d direction2d = gp_Dir2d(end2d.XY() - start2d.XY());
  Handle(Geom2d_Line) line2d = new Geom2d_Line(start2d, direction2d);
  
  // Create a cylindrical surface (radius 5, along Z-axis)
  gp_Ax3 cylinderAxis(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0));
  Handle(Geom_CylindricalSurface) cylinderSurface = 
    new Geom_CylindricalSurface(cylinderAxis, 5.0);
  
  // Convert 2D line to 3D curve on cylinder
  Curve2DTo3DConverter converter;
  Handle(Geom_Curve) helicalCurve = converter.ConvertOnSurface(line2d, cylinderSurface);
  
  // Test points along the helical curve
  std::cout << "2D Line converted to helical curve on cylinder:" << std::endl;
  Standard_Real firstParam = helicalCurve->FirstParameter();
  Standard_Real lastParam = helicalCurve->LastParameter();
  
  for (int i = 0; i <= 5; i++)
  {
    Standard_Real t = firstParam + i * (lastParam - firstParam) / 5.0;
    gp_Pnt pt = helicalCurve->Value(t);
    std::cout << "Point " << i << ": (" 
              << pt.X() << ", " << pt.Y() << ", " << pt.Z() << ")" << std::endl;
  }
}

void Example_ConvertWithRevolution()
{
  std::cout << "\n=== Converting 2D Curve with Revolution ===" << std::endl;
  
  // Create a 2D circle that will be revolved
  gp_Ax2d ax2d(gp_Pnt2d(3.0, 0.0), gp_Dir2d(1.0, 0.0)); // Circle at distance 3 from origin
  gp_Circ2d circle2d(ax2d, 1.0); // Small circle with radius 1
  Handle(Geom2d_Circle) geomCircle2d = new Geom2d_Circle(circle2d);
  
  // Revolve around Y-axis by 90 degrees
  Curve2DTo3DConverter converter;
  gp_Ax1 revolutionAxis(gp_Pnt(0,0,0), gp_Dir(0,1,0));
  Standard_Real revolutionAngle = M_PI / 2; // 90 degrees
  
  Handle(Geom_Curve) revolvedCurve = converter.ConvertWithRevolution(
    geomCircle2d, revolutionAxis, revolutionAngle);
  
  // Test points on the revolved curve
  std::cout << "2D Circle revolved around Y-axis:" << std::endl;
  for (int i = 0; i < 8; i++)
  {
    Standard_Real param = i * M_PI / 4;
    gp_Pnt pt = revolvedCurve->Value(param);
    std::cout << "Point " << i << ": (" 
              << pt.X() << ", " << pt.Y() << ", " << pt.Z() << ")" << std::endl;
  }
}

int main()
{
  std::cout << "2D to 3D Curve Conversion Examples" << std::endl;
  std::cout << "===================================" << std::endl;
  
  try
  {
    Example_ConvertLineToXYPlane();
    Example_ConvertCircleToCustomPlane();
    Example_ConvertEllipseWithExtrusion();
    Example_ConvertBSplineCurve();
    Example_ConvertBezierCurve();
    Example_ConvertOnCylindricalSurface();
    Example_ConvertWithRevolution();
    
    std::cout << "\n=== All examples completed successfully! ===" << std::endl;
  }
  catch (const Standard_Failure& e)
  {
    std::cout << "Error: " << e.GetMessageString() << std::endl;
    return 1;
  }
  catch (const std::exception& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;
  }
  
  return 0;
}