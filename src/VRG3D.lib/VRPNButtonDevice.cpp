
// only compile if USE_VRPN is defined!
#ifdef USE_VRPN

// Note: This include ordering is important, don't screw with it!
#include "VRPNButtonDevice.H"
#include "Event.H"
#include <vrpn/vrpn_Button.h>


#ifndef VRPN_CALLBACK
#define VRPN_CALLBACK
#endif

namespace VRG3D {


void  VRPN_CALLBACK
buttonHandler(void *thisPtr, const vrpn_BUTTONCB info)
{
  ((VRPNButtonDevice*)thisPtr)->sendEvent(info.button, info.state);
}



VRPNButtonDevice::VRPNButtonDevice(const std::string &vrpnButtonDeviceName,
                                   const Array<std::string> &eventsToGenerate)

{
  _eventNames = eventsToGenerate;
  
  _vrpnDevice = new vrpn_Button_Remote(vrpnButtonDeviceName.c_str());
  if (!_vrpnDevice) {
    alwaysAssertM(false, "Can't create VRPN Remote Button with name " + 
	  vrpnButtonDeviceName);
  }
  
  _vrpnDevice->register_change_handler(this, buttonHandler);
}


VRPNButtonDevice::~VRPNButtonDevice()
{
}

std::string
VRPNButtonDevice::getEventName(int buttonNumber)
{
  if (buttonNumber >= _eventNames.size())
    return std::string("VRPNButtonDevice_Unknown_Event");
  else 
    return _eventNames[buttonNumber];
}

void
VRPNButtonDevice::sendEvent(int buttonNumber, bool down)
{
  std::string ename = getEventName(buttonNumber);
  if (down)
    _pendingEvents.append(new Event(ename + "_down"));
  else
    _pendingEvents.append(new Event(ename + "_up"));
}

void
VRPNButtonDevice::pollForInput(Array<EventRef> &events)
{
  _vrpnDevice->mainloop();
  if (_pendingEvents.size()) {
    events.append(_pendingEvents);
    _pendingEvents.clear();
  }
}


} // end namespace


#endif // USE_VRPN

