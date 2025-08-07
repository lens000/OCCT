#ifndef _Curve2DTo3DConverter_HeaderFile
#define _Curve2DTo3DConverter_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <gp_Pln.hxx>
#include <gp_Ax3.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

//! This class provides methods to convert 2D curves to 3D curves
//! using various transformation approaches
class Curve2DTo3DConverter
{
public:

  DEFINE_STANDARD_ALLOC

  //! Default constructor
  Standard_EXPORT Curve2DTo3DConverter();

  //! Constructor with a reference plane
  Standard_EXPORT Curve2DTo3DConverter(const gp_Pln& thePlane);

  //! Constructor with a coordinate system
  Standard_EXPORT Curve2DTo3DConverter(const gp_Ax3& theCoordSystem);

  //! Destructor
  Standard_EXPORT ~Curve2DTo3DConverter();

public:

  //! Convert a 2D curve to 3D by placing it on the XY plane (Z=0)
  //! @param theCurve2D the input 2D curve
  //! @return the resulting 3D curve
  Standard_EXPORT Handle(Geom_Curve) ConvertToXYPlane(
    const Handle(Geom2d_Curve)& theCurve2D) const;

  //! Convert a 2D curve to 3D by placing it on a specified plane
  //! @param theCurve2D the input 2D curve
  //! @param thePlane the target plane
  //! @return the resulting 3D curve
  Standard_EXPORT Handle(Geom_Curve) ConvertToPlane(
    const Handle(Geom2d_Curve)& theCurve2D,
    const gp_Pln& thePlane) const;

  //! Convert a 2D curve to 3D using a coordinate system
  //! @param theCurve2D the input 2D curve
  //! @param theCoordSystem the target coordinate system
  //! @return the resulting 3D curve
  Standard_EXPORT Handle(Geom_Curve) ConvertWithCoordSystem(
    const Handle(Geom2d_Curve)& theCurve2D,
    const gp_Ax3& theCoordSystem) const;

  //! Convert a 2D curve to 3D by extruding along a direction
  //! @param theCurve2D the input 2D curve
  //! @param theDirection the extrusion direction
  //! @param theHeight the extrusion height (creates a line if > 0)
  //! @return the resulting 3D curve (original curve + translation)
  Standard_EXPORT Handle(Geom_Curve) ConvertWithExtrusion(
    const Handle(Geom2d_Curve)& theCurve2D,
    const gp_Vec& theDirection,
    const Standard_Real theHeight = 0.0) const;

  //! Convert a 2D curve to 3D by mapping it onto a surface
  //! @param theCurve2D the input 2D curve
  //! @param theSurface the target surface
  //! @return the resulting 3D curve on the surface
  Standard_EXPORT Handle(Geom_Curve) ConvertOnSurface(
    const Handle(Geom2d_Curve)& theCurve2D,
    const Handle(Geom_Surface)& theSurface) const;

  //! Convert a 2D curve to 3D using a transformation matrix
  //! @param theCurve2D the input 2D curve
  //! @param theTransformation the transformation matrix
  //! @return the resulting 3D curve
  Standard_EXPORT Handle(Geom_Curve) ConvertWithTransformation(
    const Handle(Geom2d_Curve)& theCurve2D,
    const gp_Trsf& theTransformation) const;

  //! Convert a 2D curve to 3D by revolution around an axis
  //! @param theCurve2D the input 2D curve
  //! @param theAxis the revolution axis
  //! @param theAngle the revolution angle in radians
  //! @return the resulting 3D curve
  Standard_EXPORT Handle(Geom_Curve) ConvertWithRevolution(
    const Handle(Geom2d_Curve)& theCurve2D,
    const gp_Ax1& theAxis,
    const Standard_Real theAngle) const;

public:

  //! Set the default plane for conversion
  Standard_EXPORT void SetDefaultPlane(const gp_Pln& thePlane);

  //! Get the default plane
  Standard_EXPORT const gp_Pln& GetDefaultPlane() const;

  //! Set the default coordinate system for conversion
  Standard_EXPORT void SetDefaultCoordSystem(const gp_Ax3& theCoordSystem);

  //! Get the default coordinate system
  Standard_EXPORT const gp_Ax3& GetDefaultCoordSystem() const;

private:

  gp_Pln myDefaultPlane;        //!< Default plane for conversion
  gp_Ax3 myDefaultCoordSystem;  //!< Default coordinate system

};

#endif // _Curve2DTo3DConverter_HeaderFile