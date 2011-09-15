/**
   \author Daniel Keefe (dfk)
   \file   TrackDClient.H
   \brief  
*/


// only compile/include this if USE_TRACKD is defined
#ifdef USE_TRACKD

#ifndef TRACKDCLIENT_H
#define TRACKDCLIENT_H

#include <G3D/G3DAll.h>
#include "InputDevice.H"

namespace VRG3D {
/**
*/
class TrackDClient : public InputDevice
{
public:

  TrackDClient(
        int                          trackerShMemKey,
        int                          wandShMemKey,
        const Array<std::string>     &trackerEventsToGenerate,
        const double                 &trackerUnitsToRoomUnitsScale,
        const CoordinateFrame        &deviceToRoom,
        const Array<CoordinateFrame> &propToTracker,
        const Array<CoordinateFrame> &finalOffset,
        const Array<std::string>     &buttonEventsToGenerate,
        const Array<std::string>     &valuatorEventsToGenerate
        );

  virtual ~TrackDClient();

  std::string getTrackerName(int trackerNumber);
  std::string getButtonName(int buttonNumber);
  std::string getValuatorName(int valuatorNumber);

  void pollForInput(Array<EventRef> &events);

private:
  Array<std::string>      _tEventNames;
  double                  _trackerUnitsToRoomUnitsScale;
  CoordinateFrame         _deviceToRoom;
  Array<CoordinateFrame>  _propToTracker;
  Array<CoordinateFrame>  _finalOffset;

  Array<std::string>      _bEventNames;
  Array<int>              _buttonStates;

  Array<std::string>      _vEventNames;
  Array<double>           _valuatorStates;

  void *_trackerMemory;
  void *_wandMemory;
  int   _numSensors;
  int   _numButtons;
  int   _numValuators;
};

} // end namespace

#endif


#endif // USE_TRACKD

