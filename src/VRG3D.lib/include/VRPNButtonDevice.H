/**
   \author     Daniel Keefe (dfk)
   \maintainer Daniel Keefe (dfk)
   
   \created 2004-01-28
   \edited  2004-01-28
   
   \file  VRPNButtonDevice.H
   
   \brief A driver that connects to VRPN and creates Events
          based on button data from VRPN.
*/


// only compile/include this if USE_VRPN is set!!
#ifdef USE_VRPN


#ifndef VRPNBUTTONDEVICE_H
#define VRPNBUTTONDEVICE_H

// Note: This include ordering is important, don't screw with it!
#include <G3D/G3DAll.h>
using namespace G3D;
#include <iostream>

#include "InputDevice.H"

class vrpn_Button_Remote;

namespace VRG3D {

/**
*/
class VRPNButtonDevice : public InputDevice
{
public:
  VRPNButtonDevice(const std::string &vrpnButtonDeviceName, 
                   const Array<std::string> &eventsToGenerate);

  virtual ~VRPNButtonDevice();

  void pollForInput(Array<EventRef> &events);

  std::string getEventName(int buttonNumber);
  void sendEvent(int buttonNumber, bool down);

private:
  vrpn_Button_Remote  *_vrpnDevice;
  Array<std::string>   _eventNames;
  Array<EventRef>      _pendingEvents;
};

} // end namespace

#endif

#endif // USE_VRPN defined


