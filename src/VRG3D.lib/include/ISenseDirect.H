/**
   \author Daniel Keefe (dfk)
   \file   ISenseDirect.H
   \brief  This is an untested driver for connecting directly to an 
           Intersense tracking device using the .dll that can be
           downloaded from intersense.
*/


// only compile/include this if USE_ISENSE is defined
#ifdef USE_ISENSE

#ifndef ISENSEDIRECT_H
#define ISENSEDIRECT_H

#include <G3D/G3DAll.h>
#include "InputDevice.H"

#include <isense.h>

namespace VRG3D {
/**
*/
class ISenseDirect : public InputDevice
{
public:
  ISenseDirect(
        const Array<std::string>     &trackerEventNames,
        const double                 &trackerUnitsToRoomUnitsScale,
        const CoordinateFrame        &deviceToRoom,
        const Array<CoordinateFrame> &propToTracker,
        const Array<CoordinateFrame> &finalOffset,
        const Array< Array<std::string> > &buttonEventNames
        );

  virtual ~ISenseDirect();

  void pollForInput(Array<EventRef> &events);

  std::string getTrackerName(int trackerNumber);
  std::string getButtonName(int stationNumber, int buttonNumber);

private:
  Array<std::string>          _tEventNames;
  Array< Array<std::string> > _bEventNames;
  double                  _trackerUnitsToRoomUnitsScale;
  CoordinateFrame         _deviceToRoom;
  Array<CoordinateFrame>  _propToTracker;
  Array<CoordinateFrame>  _finalOffset;

  Bool                    _btnStatus[ISD_MAX_STATIONS][ISD_MAX_BUTTONS];
  ISD_TRACKER_HANDLE      _handle;
  ISD_STATION_INFO_TYPE   _stationInfo[ISD_MAX_STATIONS];
  int                     _maxStations;
};

} // end namespace

#endif


#endif // USE_ISENSE




