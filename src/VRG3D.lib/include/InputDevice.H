/**
 * \author Daniel Keefe (dfk)
 *
 * \file  InputDevice.H
 *
 */


#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#include <G3D/G3DAll.h>
#include "Event.H"

namespace VRG3D {

/** Base class for InputDevices.  Input Devices should be polled once
    per frame and append Events that were generated since the last
    call to poll to the array passed into the poll routine.
*/
class InputDevice
{
public:
  InputDevice() {}
  virtual ~InputDevice() {}

  virtual void pollForInput(Array<EventRef> &events) {}
};

} // end namespace


#endif
