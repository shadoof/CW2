/**
   \author Daniel Keefe (dfk)
   \file   VRPNTrackerDevice.H
   \brief  A driver that connects to VRPN and creates CoordinateFrame Events
           based on tracker data from VRPN.
*/


// only compile/include this if USE_VRPN is defined
#ifdef USE_VRPN

#ifndef VRPNTRACKERDEVICE_H
#define VRPNTRACKERDEVICE_H

// Note: This include ordering is important, don't screw with it!
#include <G3D/G3DAll.h>
#include "InputDevice.H"


class vrpn_Tracker_Remote;

namespace VRG3D {

/** 
    Calibration Procedure:

    1. Set TrackerUnitsToRoomUnitsScale to get everything in feet.
    
    2. Adjust DeviceToRoom by printing out the position of sensor 0
    until it reports the origin of RoomSpace in the correct place and
    +X, +Y, and +Z all point in the correct direction.  You can print
    out the value of sensor 0 by calling printSensor0(true).

    3. Use the TestTrackers function of the IS3D/programTemplate demo
    to draw the CoordinateFrame for each tracker.  Adjust each
    tracker's PropToTracker transformation until it aligns itself with
    the RoomSpace frame when the tracker is held at the origin of
    RoomSpace.

    Here's an easy way to think about it: Hold up each prop at the
    origin of RoomSpace in the orientation that you want to be
    considered "normal".  Then, look at the drawing of its frame.  For
    each of the vectors in its frame (X,Y,Z) that are shown, ask
    yourself, what is the RoomSpace vector that this arrow depicts?
    Then enter that value in the appropriate X,Y, or Z column in the
    PropToTracker frame.

    4. For some of the props you may want the origin for the prop to
    be somewhere on the prop other than where the tracker is
    positioned.  For these, measure the distance from the tracker to
    where you want the origin to be, call this vector V.  Then, put
    this vector into the translation part of the PropToTracker frame.
    Be careful, the translation is applied after the rotation
    specified in the frame, so for example, if the first column of the
    frame is (0,1,0,0), you should put the Y component of V in that
    column, so it would become (0,1,0,Y[1]).  If it were (0,-1,0,0),
    you would need to put (0,-1,0,-Y[1]).  
*/
class VRPNTrackerDevice : public InputDevice
{
public:

  VRPNTrackerDevice(
        const std::string            &vrpnTrackerDeviceName, 
        const Array<std::string>     &eventsToGenerate,
        const double                 &trackerUnitsToRoomUnitsScale,
        const CoordinateFrame        &deviceToRoom,
        const Array<CoordinateFrame> &propToTracker,
        const Array<CoordinateFrame> &finalOffset,
        const bool                   &waitForNewReportInPoll,
        const bool                   &convertLHtoRH = false);

  virtual ~VRPNTrackerDevice();

  void processEvent(const CoordinateFrame &vrpnEvent, int sensorNum);

  std::string getEventName(int trackerNumber);

  void pollForInput(Array<EventRef> &events);

  void setPrintSensor0(bool b) { _printSensor0 = b; }

private:
  vrpn_Tracker_Remote    *_vrpnDevice;
  Array<std::string>      _eventNames;
  double                  _trackerUnitsToRoomUnitsScale;
  CoordinateFrame         _deviceToRoom;
  Array<CoordinateFrame>  _propToTracker;
  Array<CoordinateFrame>  _finalOffset;
  bool                    _printSensor0;
  bool                    _waitForNewReport;
  bool                    _convertLHtoRH;
  bool                    _newReportFlag;
  Array<EventRef>         _pendingEvents;
};

} // end namespace

#endif


#endif // USE_VRPN

