
// Note: This is untested at this point!!!  dfk 03/06/06


#ifdef USE_ISENSE

#include "ISenseDirect.H"

// ISense sets this up so you compile this rather than link with a
// .lib file to pick up the hooks for the .dll
#include <isense.c>

namespace VRG3D {

ISenseDirect::ISenseDirect(
        const Array<std::string>     &trackerEventNames,
        const double                 &trackerUnitsToRoomUnitsScale,
        const CoordinateFrame        &deviceToRoom,
        const Array<CoordinateFrame> &propToTracker,
        const Array<CoordinateFrame> &finalOffset,
        const Array< Array<std::string> > &buttonEventNames
        )
{
  _tEventNames = trackerEventNames;
  _bEventNames = buttonEventNames;
  _trackerUnitsToRoomUnitsScale = trackerUnitsToRoomUnitsScale;
  _deviceToRoom = deviceToRoom;
  _propToTracker = propToTracker;
  _finalOffset = finalOffset;

  for (int i=0;i<ISD_MAX_STATIONS;i++) {
    for (int j=0;j<ISD_MAX_BUTTONS;j++) {
      _btnStatus[i][j] = 0;
    }
  }


  // Detect first tracker. If you have more than one InterSense device and
  // would like to have a specific tracker, connected to a known port, 
  // initialized first, then enter the port number instead of 0. Otherwise, 
  // tracker connected to the rs232 port with lower number is found first 
  _handle = ISD_OpenTracker((Hwnd)NULL, 0, FALSE, TRUE);
    
  // Check value of handle to see if tracker was located 
  if (_handle < 1) {
    printf("Failed to detect InterSense tracking device\n");
    exit(1);
  }


  // Get tracker configuration info 
  ISD_TRACKER_INFO_TYPE           Tracker;
  ISD_GetTrackerConfig( _handle, &Tracker, TRUE );
                
  ISD_HARDWARE_INFO_TYPE          hwInfo;
  memset((void *) &hwInfo, 0, sizeof(hwInfo));
  
  if ( ISD_GetSystemHardwareInfo( _handle, &hwInfo ) ) {
    if( hwInfo.Valid ) {
      _maxStations = hwInfo.Capability.MaxStations;
    }
  }

  // Clear station configuration info to make sure GetAnalogData and other flags are FALSE 
  memset((void *) _stationInfo, 0, sizeof(_stationInfo));

  // General procedure for changing any setting is to first retrieve current 
  // configuration, make the change, and then apply it. Calling 
  // ISD_GetStationConfig is important because you only want to change 
  // some of the settings, leaving the rest unchanged.   
  if ( Tracker.TrackerType == ISD_PRECISION_SERIES ) {
    for (int station = 1; station <= _maxStations; station++ ) {         
      // fill ISD_STATION_INFO_TYPE structure with current station configuration 
      if( !ISD_GetStationConfig( _handle, &_stationInfo[station-1], station, TRUE )) 
        break;
    }
  }
}

ISenseDirect::~ISenseDirect()
{
  ISD_CloseTracker(_handle);
}

std::string
ISenseDirect::getTrackerName(int trackerNumber)
{
  if (trackerNumber >= _tEventNames.size())
    return std::string("ISenseUnknown_Tracker");
  else
    return _tEventNames[trackerNumber];
}

std::string
ISenseDirect::getButtonName(int stationNumber, int buttonNumber)
{
  if (stationNumber >= _bEventNames.size())
    return std::string("ISenseUnknown_Btn");
  else if (buttonNumber >= _bEventNames[stationNumber].size())
    return std::string("ISenseUnknown_Btn");
  else
    return _bEventNames[stationNumber][buttonNumber];
}

void
ISenseDirect::pollForInput(Array<EventRef> &events)
{
  ISD_TRACKER_DATA_TYPE trackerData;
  ISD_GetData( _handle, &trackerData );    

  for (int s=0;s<_maxStations;s++) {
    ISD_STATION_STATE_TYPE *data = &trackerData.Station[s];

    Vector3 trans = Vector3(data->Position[0], data->Position[1], data->Position[2]);
    //printf("%6.2f %6.2f %6.2f ",
    //       data->Position[0], data->Position[1], data->Position[2]);
    
    CoordinateFrame trackerToDevice;
    if ( _stationInfo[s].AngleFormat == ISD_QUATERNION ) {
      //printf("%5.2f %5.2f %5.2f %5.2f ", data->Orientation[0], 
      //       data->Orientation[1], data->Orientation[2], data->Orientation[3]);

      Matrix3 rot = Matrix3(Quat(data->Orientation[0], data->Orientation[1],
                                 data->Orientation[2], data->Orientation[3]));

      trackerToDevice = CoordinateFrame(rot, trans);
    }
    else { // Euler angles
      //printf("%7.2f %7.2f %7.2f ",
      //       data->Orientation[0], data->Orientation[1], data->Orientation[2]);
      Matrix3 rotY = Matrix3::fromAxisAngle(Vector3(0,1,0), toRadians(data->Orientation[1]));
      Matrix3 rotX = Matrix3::fromAxisAngle(Vector3(1,0,0), toRadians(data->Orientation[0]));
      Matrix3 rotZ = Matrix3::fromAxisAngle(Vector3(0,0,1), toRadians(data->Orientation[2]));
      
      trackerToDevice = trans * rotY * rotX * rotZ;
    }

    trackerToDevice.translation *= _trackerUnitsToRoomUnitsScale;
    CoordinateFrame eventRoom = _finalOffset[s] * _deviceToRoom * 
      trackerToDevice * _propToTracker[s];

    events.append(new Event(getTrackerName(s), eventRoom));  


    /*printf("%d%d%d%d%d ", 
           (int) data->ButtonState[0], 
           (int) data->ButtonState[1], 
           (int) data->ButtonState[2], 
           (int) data->ButtonState[3], 
           (int) data->ButtonState[4]);
    */

    for (int i=0;i<ISD_MAX_BUTTONS;i++) {
      if (data->ButtonState[i] != _btnStatus[s][i]) {
        _btnStatus[s][i] = !_btnStatus[s][i];
        if (_btnStatus[s][i]) {
          events.append(new Event(getButtonName(s, i) + "_down"));
        }
        else {
          events.append(new Event(getButtonName(s, i) + "_up"));
        }
      }
    }
      
    // TODO: deal with analogs..
    //printf("%d %d ", data->AnalogData[0], data->AnalogData[1]); 
  }
}


} // end namespace


#endif // USE_ISENSE
