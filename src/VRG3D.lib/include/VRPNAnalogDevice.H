/**
   \author     Daniel Keefe (dfk)
   \maintainer Daniel Keefe (dfk)
   
   \created 2004-01-28
   \edited  2004-01-28
   
   \file  VRPNAnalogDevice.H
   
   \brief A driver that connects to VRPN and creates Events
          based on analog data from VRPN.
*/


// only compile/include this if USE_VRPN is set!!
#ifdef USE_VRPN


#ifndef VRPNANALOGDEVICE_H
#define VRPNANALOGDEVICE_H

// Note: This include ordering is important, don't screw with it!
#include <G3D/G3DAll.h>
using namespace G3D;
#include <iostream>

#include "InputDevice.H"

class vrpn_Analog_Remote;

namespace VRG3D {

/**
*/
class VRPNAnalogDevice : public InputDevice
{
public:
  VRPNAnalogDevice(const std::string &vrpnAnalogDeviceName, 
                   const Array<std::string> &eventsToGenerate);

  virtual ~VRPNAnalogDevice();

  void        pollForInput(Array<EventRef> &events);
  void        sendEventIfChanged(int channelNumber, double data);
  std::string getEventName(int channelNumber);
  int         numChannels() { return _eventNames.size(); }

private:
  vrpn_Analog_Remote  *_vrpnDevice;
  Array<std::string>   _eventNames;
  Array<double>        _channelValues;
  Array<EventRef>      _pendingEvents;
};

} // end namespace

#endif

#endif // USE_VRPN defined


