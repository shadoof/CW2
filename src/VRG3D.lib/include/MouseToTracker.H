/**
 * \author Daniel Keefe (dfk)
 *
 * \file  MouseToTracker.H
 * \brief Transforms mouse movement and keypresses into 6D tracker events.
 *        For use with testing 6D interaction techniques on a desktop.
 *
 */

#ifndef MOUSETOTRACKER_H
#define MOUSETOTRACKER_H

#include <G3D/G3DAll.h>
#include "Event.H"
#include "ProjectionVRCamera.H"

namespace VRG3D {


typedef ReferenceCountedPointer<class MouseToTracker> MouseToTrackerRef;
/** Converts mouse movement and keypresses into CoordinateFrame events
    mimicking 6DOF Tracker input.  Useful primarily for debugging VR
    interactions at a desktop with a mouse.  If you hold down the
    X,Y,or Z key while moving the mouse you'll rotate the tracker
    around the corresponding local axis.  If you specify something >1
    for the numTrackers parameter, then you can press the TAB key to
    cycle through controlling several trackers.  The Event names for
    the trackers are Mouse1_Tracker, Mouse2_Tracker, etc..
*/
class MouseToTracker : public ReferenceCountedObject
{
public:

  MouseToTracker(ProjectionVRCameraRef camera, int numTrackers=1);

  virtual ~MouseToTracker();

  /// Call this method each frame to respond to input events that
  /// should include Mouse_Pointer events and kbd events.  It returns
  /// an array of tracker events generated based on the input.
  void doUserInput(const Array<EventRef> &events, Array<EventRef> &generatedEvents);

protected:

  std::string intToString(int i);
  void cycleTracker();
  
  EventRef mouseMoveInPlane(EventRef e);
  EventRef mouseMoveInOut(EventRef e);
  EventRef mouseMoveRotX(EventRef e);
  EventRef mouseMoveRotY(EventRef e);
  EventRef mouseMoveRotZ(EventRef e);


  enum FsaStates {
    START,
    XROT,
    YROT,
    ZROT,
    DOLLY
  };

  ProjectionVRCameraRef _camera;
  FsaStates     _fsaState;
  Matrix3       _rot;
  Vector3       _trans;
  Vector2       _lastPos;
  int           _numTrackers;
  int           _curTracker;
};

} // end namespace

#endif
