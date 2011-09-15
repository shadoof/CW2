/**
 * /author Dan Keefe (dfk)
 *
 * /file  ProjectionVRCamera.H
 * /brief 
 *
 */ 

#ifndef PROJECTIONVRCAMERA_H
#define PROJECTIONVRCAMERA_H

#include <G3D/G3DAll.h>
#include "DisplayTile.H"

namespace VRG3D {

typedef ReferenceCountedPointer<class ProjectionVRCamera> ProjectionVRCameraRef;
/**
*/
class ProjectionVRCamera : public ReferenceCountedObject
{
public:

  enum EyeProjectionType {
    LeftEye,   // left eye in stereo rendering
    RightEye,  // right eye in stereo rendering
    Cyclops    // a single "eye" -> mono rendering only
  };

  /** Inter-ocular distance defaults to 2.5 inches = 0.2083 feet, this
      assumes your virtual coordinates are reported in feet.
  */
  ProjectionVRCamera(DisplayTile dispTile, CoordinateFrame initHeadFrame, 
                     double interOcularDist=0.2083);
  virtual ~ProjectionVRCamera();

  /** Sets the projection matrix and the camera to world matrix for
      the RenderDevice based on the current DisplayTile, HeadFrame,
      and which eye is about to be rendered. */
  void applyProjection(RenderDevice *rd, EyeProjectionType whichEye);

  CoordinateFrame getHeadFrame()      { return headFrame; }
  CoordinateFrame getLeftEyeFrame();
  CoordinateFrame getRightEyeFrame();
  Vector3         getHeadPos()        { return headFrame.translation; }
  Vector3         getCameraPos()      { return headFrame.translation; }
  Vector3         getLookVec();
  DisplayTile     getTile()           { return tile; }

  void updateHeadFrame(CoordinateFrame frame) { headFrame = frame; }
  void setDisplayTile(DisplayTile dispTile)   { tile = dispTile; }

  void   set_iod(double val) { iod = val; }
  double get_iod() const     { return iod; }

  /// Inter-ocular distance, default = 2.5 inches = 0.2083 feet
  double           iod;
  DisplayTile      tile;
  CoordinateFrame  headFrame;
};

} // end namespace

#endif
