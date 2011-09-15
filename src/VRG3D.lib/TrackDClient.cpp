#include "TrackDClient.H"


#ifdef USE_TRACKD
using namespace std;
#include <trackdAPI.h>

#include <iostream>

namespace VRG3D {

TrackDClient::TrackDClient(
        int                          trackerShMemKey,
        int                          wandShMemKey,
        const Array<std::string>     &trackerEventsToGenerate,
        const double                 &trackerUnitsToRoomUnitsScale,
        const CoordinateFrame        &deviceToRoom,
        const Array<CoordinateFrame> &propToTracker,
        const Array<CoordinateFrame> &finalOffset,
        const Array<std::string>     &buttonEventsToGenerate,
        const Array<std::string>     &valuatorEventsToGenerate)
{
  _tEventNames = trackerEventsToGenerate;
  _trackerUnitsToRoomUnitsScale = trackerUnitsToRoomUnitsScale;
  _deviceToRoom = deviceToRoom;
  _propToTracker = propToTracker;
  _finalOffset = finalOffset;
  _bEventNames = buttonEventsToGenerate;
  _vEventNames = valuatorEventsToGenerate;
  
  _trackerMemory = trackdInitTrackerReader(trackerShMemKey);
  if (_trackerMemory == NULL) {
    cerr << "TrackDClient:: Could not initialize tracker reader." << endl;
    exit(1);
  }
  
  _wandMemory = trackdInitControllerReader(wandShMemKey);
  if (_wandMemory == NULL) {
    cerr << "TrackDClient:: Could not initialize wand reader." << endl;
    exit(1);
  }
  
  _numSensors = trackdGetNumberOfSensors(_trackerMemory);
  _numButtons = trackdGetNumberOfButtons(_wandMemory);
  for (int i=0;i<_numButtons;i++) {
    _buttonStates.append(0);
  }
  _numValuators = trackdGetNumberOfValuators(_wandMemory);
  for (int i=0;i<_numValuators;i++) {
    _valuatorStates.append(0.0);
  }
}

TrackDClient::~TrackDClient()
{
}

std::string
TrackDClient::getTrackerName(int trackerNumber)
{
  if (trackerNumber >= _tEventNames.size())
    return std::string("TrackDClientUnknown_Tracker");
  else
    return _tEventNames[trackerNumber];
}

std::string
TrackDClient::getButtonName(int buttonNumber)
{
  if (buttonNumber >= _bEventNames.size())
    return std::string("TrackDClientUnknown_Btn");
  else
    return _bEventNames[buttonNumber];
}

std::string
TrackDClient::getValuatorName(int valuatorNumber)
{
  if (valuatorNumber >= _vEventNames.size())
    return std::string("TrackDClientUnknown_Valuator");
  else
    return _vEventNames[valuatorNumber];
}

void
TrackDClient::pollForInput(Array<EventRef> &events) 
{
  // Update trackers
  for (int i=0;i<iMin(_numSensors, _tEventNames.size());i++) {
    float tracker[6];
    trackdGetPosition(_trackerMemory, i, &(tracker[0]));
    trackdGetEulerAngles(_trackerMemory, i, &(tracker[3]));
  
    Vector3 trans(tracker[0], tracker[1], tracker[2]);
    Matrix3 rotY = Matrix3::fromAxisAngle(Vector3(0,1,0), toRadians(tracker[4]));
    Matrix3 rotX = Matrix3::fromAxisAngle(Vector3(1,0,0), toRadians(tracker[3]));
    Matrix3 rotZ = Matrix3::fromAxisAngle(Vector3(0,0,1), toRadians(tracker[5]));
    
    CoordinateFrame trackerToDevice = CoordinateFrame(trans) * 
		CoordinateFrame(rotY) *
		CoordinateFrame(rotX) * 
		CoordinateFrame(rotZ);
    trackerToDevice.translation *= _trackerUnitsToRoomUnitsScale;

    /** This is probably more direct, but can't test it right now..
    float mat[4][4];
    trackdGetMatrix(_trackerMemory, i, mat);
    CoordinateFrame trackerToDevice = CoordinateFrame(Matrix3(mat[0][0], mat[1][1], mat[2][2],
                                                              mat[0][1], mat[1][1], mat[2][1],
                                                              mat[0][2], mat[1][2], mat[2][2]),
                                                      Vector3(mat[0][3], mat[1][3], mat[2][3]));
    **/
    
    CoordinateFrame eventRoom = _finalOffset[i] * _deviceToRoom * 
      trackerToDevice * _propToTracker[i];

    events.append(new Event(getTrackerName(i), eventRoom));  
  }

  // Update buttons
  for (int i=0;i<iMin(_numButtons, _bEventNames.size());i++) {
    int b = trackdGetButton(_wandMemory, i);
    if (b != _buttonStates[i]) {
      _buttonStates[i] = b;
      std::string name = getButtonName(i);
      if (b) {
        name = name + "_down";
      }
      else {
        name = name + "_up";
      }
      events.append(new Event(name));
    }
  }

  /**
  // Update valuators
  for (int i=0;i<_numValuators;i++) {
    float v = trackdGetValuator(_wandMemory, i);
    if (v != _valuatorStates[i]) {
      _valuatorStates[i] = v;
      events.append(new Event(getValuatorName(i), v));
    }
  }
  **/
}


} // end namespace

#endif // USE_TRACKD

