
// only compile if USE_VRPN is defined!
#ifdef USE_VRPN

// Note: This include ordering is important, don't screw with it!
#include "VRPNAnalogDevice.H"
#include "Event.H"
#include <vrpn/vrpn_Analog.h>


#ifndef VRPN_CALLBACK
#define VRPN_CALLBACK
#endif

namespace VRG3D {


void VRPN_CALLBACK
analogHandler(void *thisPtr, const vrpn_ANALOGCB info)
{
  int lastchannel = (int)min(info.num_channel, ((VRPNAnalogDevice*)thisPtr)->numChannels());
  for (int i=0;i<lastchannel;i++) {
    ((VRPNAnalogDevice*)thisPtr)->sendEventIfChanged(i, info.channel[i]);
  }
}



VRPNAnalogDevice::VRPNAnalogDevice(const std::string &vrpnAnalogDeviceName,
                                   const Array<std::string> &eventsToGenerate)

{
  _eventNames = eventsToGenerate;
  for (int i=0;i<_eventNames.size();i++)
    _channelValues.append(0.0);
  
  _vrpnDevice = new vrpn_Analog_Remote(vrpnAnalogDeviceName.c_str());
  if (!_vrpnDevice) {
    alwaysAssertM(false, "Can't create VRPN Remote Analog with name" + 
	  vrpnAnalogDeviceName);
  }
  
  _vrpnDevice->register_change_handler(this, analogHandler);
}


VRPNAnalogDevice::~VRPNAnalogDevice()
{
}

std::string
VRPNAnalogDevice::getEventName(int channelNumber)
{
  if (channelNumber >= _eventNames.size())
    return std::string("VRPNAnalogDevice_Unknown_Event");
  else 
    return _eventNames[channelNumber];
}

void
VRPNAnalogDevice::sendEventIfChanged(int channelNumber, double data)
{
  if (_channelValues[channelNumber] != data) {
    _pendingEvents.append(new Event(_eventNames[channelNumber], data));
    _channelValues[channelNumber] = data;
  }
}

void
VRPNAnalogDevice::pollForInput(Array<EventRef> &events)
{
  _vrpnDevice->mainloop();
  if (_pendingEvents.size()) {
    events.append(_pendingEvents);
    _pendingEvents.clear();
  }
}


} // end namespace


#endif // USE_VRPN

