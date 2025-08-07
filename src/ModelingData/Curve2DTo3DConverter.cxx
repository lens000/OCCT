#include "Curve2DTo3DConverter.h"

#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_OffsetCurve.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_CurveOnSurface.hxx>

#include <Geom2d_Line.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_BezierCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>

#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Lin.hxx>

#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

#include <GeomAdaptor_Surface.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>

#include <Standard_ConstructionError.hxx>
#include <Standard_NullObject.hxx>

//=======================================================================
//function : Curve2DTo3DConverter
//purpose  : Default constructor
//=======================================================================
Curve2DTo3DConverter::Curve2DTo3DConverter()
: myDefaultPlane(gp_Pln(gp_Pnt(0,0,0), gp_Dir(0,0,1))),
  myDefaultCoordSystem(gp_Ax3(gp_Pnt(0,0,0), gp_Dir(0,0,1), gp_Dir(1,0,0)))
{
}

//=======================================================================
//function : Curve2DTo3DConverter
//purpose  : Constructor with reference plane
//=======================================================================
Curve2DTo3DConverter::Curve2DTo3DConverter(const gp_Pln& thePlane)
: myDefaultPlane(thePlane),
  myDefaultCoordSystem(thePlane.Position())
{
}

//=======================================================================
//function : Curve2DTo3DConverter
//purpose  : Constructor with coordinate system
//=======================================================================
Curve2DTo3DConverter::Curve2DTo3DConverter(const gp_Ax3& theCoordSystem)
: myDefaultPlane(gp_Pln(theCoordSystem)),
  myDefaultCoordSystem(theCoordSystem)
{
}

//=======================================================================
//function : ~Curve2DTo3DConverter
//purpose  : Destructor
//=======================================================================
Curve2DTo3DConverter::~Curve2DTo3DConverter()
{
}

//=======================================================================
//function : ConvertToXYPlane
//purpose  : Convert 2D curve to 3D on XY plane
//=======================================================================
Handle(Geom_Curve) Curve2DTo3DConverter::ConvertToXYPlane(
  const Handle(Geom2d_Curve)& theCurve2D) const
{
  if (theCurve2D.IsNull())
    throw Standard_NullObject("Curve2DTo3DConverter::ConvertToXYPlane - null input curve");

  gp_Pln xyPlane(gp_Pnt(0,0,0), gp_Dir(0,0,1));
  return ConvertToPlane(theCurve2D, xyPlane);
}

//=======================================================================
//function : ConvertToPlane
//purpose  : Convert 2D curve to 3D on specified plane
//=======================================================================
Handle(Geom_Curve) Curve2DTo3DConverter::ConvertToPlane(
  const Handle(Geom2d_Curve)& theCurve2D,
  const gp_Pln& thePlane) const
{
  if (theCurve2D.IsNull())
    throw Standard_NullObject("Curve2DTo3DConverter::ConvertToPlane - null input curve");

  return ConvertWithCoordSystem(theCurve2D, thePlane.Position());
}

//=======================================================================
//function : ConvertWithCoordSystem
//purpose  : Convert 2D curve to 3D using coordinate system
//=======================================================================
Handle(Geom_Curve) Curve2DTo3DConverter::ConvertWithCoordSystem(
  const Handle(Geom2d_Curve)& theCurve2D,
  const gp_Ax3& theCoordSystem) const
{
  if (theCurve2D.IsNull())
    throw Standard_NullObject("Curve2DTo3DConverter::ConvertWithCoordSystem - null input curve");

  Handle(Geom_Curve) result;
  
  // Handle specific curve types for better performance and accuracy
  if (Handle(Geom2d_Line) line2d = Handle(Geom2d_Line)::DownCast(theCurve2D))
  {
    gp_Pnt2d pt2d = line2d->Location();
    gp_Dir2d dir2d = line2d->Direction();
    
    gp_Pnt pt3d = theCoordSystem.Location().XYZ() + 
                  pt2d.X() * theCoordSystem.XDirection().XYZ() +
                  pt2d.Y() * theCoordSystem.YDirection().XYZ();
    
    gp_Dir dir3d(dir2d.X() * theCoordSystem.XDirection().XYZ() +
                 dir2d.Y() * theCoordSystem.YDirection().XYZ());
    
    result = new Geom_Line(pt3d, dir3d);
  }
  else if (Handle(Geom2d_Circle) circle2d = Handle(Geom2d_Circle)::DownCast(theCurve2D))
  {
    gp_Circ2d circ2d = circle2d->Circ2d();
    gp_Pnt2d center2d = circ2d.Location();
    Standard_Real radius = circ2d.Radius();
    
    gp_Pnt center3d = theCoordSystem.Location().XYZ() + 
                      center2d.X() * theCoordSystem.XDirection().XYZ() +
                      center2d.Y() * theCoordSystem.YDirection().XYZ();
    
    gp_Ax2 ax2(center3d, theCoordSystem.Direction(), theCoordSystem.XDirection());
    gp_Circ circ3d(ax2, radius);
    
    result = new Geom_Circle(circ3d);
  }
  else if (Handle(Geom2d_Ellipse) ellipse2d = Handle(Geom2d_Ellipse)::DownCast(theCurve2D))
  {
    gp_Elips2d elips2d = ellipse2d->Elips2d();
    gp_Pnt2d center2d = elips2d.Location();
    Standard_Real majorRadius = elips2d.MajorRadius();
    Standard_Real minorRadius = elips2d.MinorRadius();
    
    gp_Pnt center3d = theCoordSystem.Location().XYZ() + 
                      center2d.X() * theCoordSystem.XDirection().XYZ() +
                      center2d.Y() * theCoordSystem.YDirection().XYZ();
    
    gp_Ax2 ax2(center3d, theCoordSystem.Direction(), theCoordSystem.XDirection());
    gp_Elips elips3d(ax2, majorRadius, minorRadius);
    
    result = new Geom_Ellipse(elips3d);
  }
  else if (Handle(Geom2d_BSplineCurve) bspline2d = Handle(Geom2d_BSplineCurve)::DownCast(theCurve2D))
  {
    Standard_Integer nbPoles = bspline2d->NbPoles();
    TColgp_Array1OfPnt poles3d(1, nbPoles);
    
    for (Standard_Integer i = 1; i <= nbPoles; i++)
    {
      gp_Pnt2d pole2d = bspline2d->Pole(i);
      gp_Pnt pole3d = theCoordSystem.Location().XYZ() + 
                      pole2d.X() * theCoordSystem.XDirection().XYZ() +
                      pole2d.Y() * theCoordSystem.YDirection().XYZ();
      poles3d(i) = pole3d;
    }
    
    TColStd_Array1OfReal weights(1, nbPoles);
    for (Standard_Integer i = 1; i <= nbPoles; i++)
    {
      weights(i) = bspline2d->Weight(i);
    }
    
    Standard_Integer nbKnots = bspline2d->NbKnots();
    TColStd_Array1OfReal knots(1, nbKnots);
    TColStd_Array1OfInteger multiplicities(1, nbKnots);
    
    for (Standard_Integer i = 1; i <= nbKnots; i++)
    {
      knots(i) = bspline2d->Knot(i);
      multiplicities(i) = bspline2d->Multiplicity(i);
    }
    
    result = new Geom_BSplineCurve(poles3d, weights, knots, multiplicities, bspline2d->Degree());
  }
  else if (Handle(Geom2d_BezierCurve) bezier2d = Handle(Geom2d_BezierCurve)::DownCast(theCurve2D))
  {
    Standard_Integer nbPoles = bezier2d->NbPoles();
    TColgp_Array1OfPnt poles3d(1, nbPoles);
    
    for (Standard_Integer i = 1; i <= nbPoles; i++)
    {
      gp_Pnt2d pole2d = bezier2d->Pole(i);
      gp_Pnt pole3d = theCoordSystem.Location().XYZ() + 
                      pole2d.X() * theCoordSystem.XDirection().XYZ() +
                      pole2d.Y() * theCoordSystem.YDirection().XYZ();
      poles3d(i) = pole3d;
    }
    
    if (bezier2d->IsRational())
    {
      TColStd_Array1OfReal weights(1, nbPoles);
      for (Standard_Integer i = 1; i <= nbPoles; i++)
      {
        weights(i) = bezier2d->Weight(i);
      }
      result = new Geom_BezierCurve(poles3d, weights);
    }
    else
    {
      result = new Geom_BezierCurve(poles3d);
    }
  }
  else
  {
    // Generic approach: sample points and create B-spline
    Standard_Integer nbSamples = 100;
    Standard_Real uFirst = theCurve2D->FirstParameter();
    Standard_Real uLast = theCurve2D->LastParameter();
    
    TColgp_Array1OfPnt points3d(1, nbSamples);
    Standard_Real du = (uLast - uFirst) / (nbSamples - 1);
    
    for (Standard_Integer i = 1; i <= nbSamples; i++)
    {
      Standard_Real u = uFirst + (i - 1) * du;
      gp_Pnt2d pt2d = theCurve2D->Value(u);
      gp_Pnt pt3d = theCoordSystem.Location().XYZ() + 
                    pt2d.X() * theCoordSystem.XDirection().XYZ() +
                    pt2d.Y() * theCoordSystem.YDirection().XYZ();
      points3d(i) = pt3d;
    }
    
    // Create B-spline through points (simplified approach)
    result = new Geom_BSplineCurve(points3d, 3, 8);
  }
  
  return result;
}

//=======================================================================
//function : ConvertWithExtrusion
//purpose  : Convert 2D curve to 3D with extrusion
//=======================================================================
Handle(Geom_Curve) Curve2DTo3DConverter::ConvertWithExtrusion(
  const Handle(Geom2d_Curve)& theCurve2D,
  const gp_Vec& theDirection,
  const Standard_Real theHeight) const
{
  if (theCurve2D.IsNull())
    throw Standard_NullObject("Curve2DTo3DConverter::ConvertWithExtrusion - null input curve");

  Handle(Geom_Curve) baseCurve = ConvertToXYPlane(theCurve2D);
  
  if (Abs(theHeight) > Precision::Confusion())
  {
    gp_Trsf translation;
    translation.SetTranslation(theDirection.Normalized() * theHeight);
    baseCurve->Transform(translation);
  }
  
  return baseCurve;
}

//=======================================================================
//function : ConvertOnSurface
//purpose  : Convert 2D curve to 3D on surface
//=======================================================================
Handle(Geom_Curve) Curve2DTo3DConverter::ConvertOnSurface(
  const Handle(Geom2d_Curve)& theCurve2D,
  const Handle(Geom_Surface)& theSurface) const
{
  if (theCurve2D.IsNull() || theSurface.IsNull())
    throw Standard_NullObject("Curve2DTo3DConverter::ConvertOnSurface - null input");

  Handle(GeomAdaptor_Surface) surfaceAdaptor = new GeomAdaptor_Surface(theSurface);
  Handle(Adaptor3d_CurveOnSurface) curveOnSurface = 
    new Adaptor3d_CurveOnSurface(theCurve2D, surfaceAdaptor);
  
  // Sample points on the surface
  Standard_Integer nbSamples = 100;
  Standard_Real uFirst = theCurve2D->FirstParameter();
  Standard_Real uLast = theCurve2D->LastParameter();
  
  TColgp_Array1OfPnt points3d(1, nbSamples);
  Standard_Real du = (uLast - uFirst) / (nbSamples - 1);
  
  for (Standard_Integer i = 1; i <= nbSamples; i++)
  {
    Standard_Real u = uFirst + (i - 1) * du;
    gp_Pnt2d pt2d = theCurve2D->Value(u);
    gp_Pnt pt3d = theSurface->Value(pt2d.X(), pt2d.Y());
    points3d(i) = pt3d;
  }
  
  return new Geom_BSplineCurve(points3d, 3, 8);
}

//=======================================================================
//function : ConvertWithTransformation
//purpose  : Convert 2D curve to 3D using transformation
//=======================================================================
Handle(Geom_Curve) Curve2DTo3DConverter::ConvertWithTransformation(
  const Handle(Geom2d_Curve)& theCurve2D,
  const gp_Trsf& theTransformation) const
{
  if (theCurve2D.IsNull())
    throw Standard_NullObject("Curve2DTo3DConverter::ConvertWithTransformation - null input curve");

  Handle(Geom_Curve) curve3d = ConvertToXYPlane(theCurve2D);
  curve3d->Transform(theTransformation);
  
  return curve3d;
}

//=======================================================================
//function : ConvertWithRevolution
//purpose  : Convert 2D curve to 3D by revolution
//=======================================================================
Handle(Geom_Curve) Curve2DTo3DConverter::ConvertWithRevolution(
  const Handle(Geom2d_Curve)& theCurve2D,
  const gp_Ax1& theAxis,
  const Standard_Real theAngle) const
{
  if (theCurve2D.IsNull())
    throw Standard_NullObject("Curve2DTo3DConverter::ConvertWithRevolution - null input curve");

  // For revolution, we typically want to create a surface of revolution
  // and then extract a curve from it. For simplicity, we'll rotate the curve
  // by the specified angle around the axis.
  
  Handle(Geom_Curve) baseCurve = ConvertToXYPlane(theCurve2D);
  
  gp_Trsf rotation;
  rotation.SetRotation(theAxis, theAngle);
  baseCurve->Transform(rotation);
  
  return baseCurve;
}

//=======================================================================
//function : SetDefaultPlane
//purpose  : Set default plane
//=======================================================================
void Curve2DTo3DConverter::SetDefaultPlane(const gp_Pln& thePlane)
{
  myDefaultPlane = thePlane;
  myDefaultCoordSystem = thePlane.Position();
}

//=======================================================================
//function : GetDefaultPlane
//purpose  : Get default plane
//=======================================================================
const gp_Pln& Curve2DTo3DConverter::GetDefaultPlane() const
{
  return myDefaultPlane;
}

//=======================================================================
//function : SetDefaultCoordSystem
//purpose  : Set default coordinate system
//=======================================================================
void Curve2DTo3DConverter::SetDefaultCoordSystem(const gp_Ax3& theCoordSystem)
{
  myDefaultCoordSystem = theCoordSystem;
  myDefaultPlane = gp_Pln(theCoordSystem);
}

//=======================================================================
//function : GetDefaultCoordSystem
//purpose  : Get default coordinate system
//=======================================================================
const gp_Ax3& Curve2DTo3DConverter::GetDefaultCoordSystem() const
{
  return myDefaultCoordSystem;
}