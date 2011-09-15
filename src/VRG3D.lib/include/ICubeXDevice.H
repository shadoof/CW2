/**
   \author Daniel Keefe (dfk)
   \file   ICubeXDevice.H   
   \brief  Driver for ICubeX midi-based input device.

*/

#ifndef ICUBEXDEVICE_H
#define ICUBEXDEVICE_H

#include <G3D/G3DAll.h>
#include <iostream>
using namespace std;

#include "Midi.H"
#include "InputDevice.H"


namespace VRG3D {

class ICubeXDevice : public InputDevice
{
public:

  enum ICubeXMode {
    HOST_MODE = 0,
    STAND_ALONE_MODE = 1
  };

  /** This constructor opens a midi-in and midi-out device by their
   number.  You can get the available midi device names with the
   calls: MidiInDevice::getAvailableDeviceNames() and also
   MidiOutDevice::getAvailableDeviceNames().  The device number is the
   index of the device in the array returned by these calls.

   The activeChannels string is a space separated list of channel
   numbers that are active.  The numbering starts at 1 and corresponds
   to the numbers printed on the front of the device.

   The sampling interval is specified in ms.  The factory default for
   the harware is 100ms.

   The device can be run in 2 modes: stand-alone and host.
   Stand-alone is the preferred method because it allows you to setup
   signal processing on the device itself.  The manufacturer has made
   signal processing presets for the input sensors they manufacture
   which set the min/max voltages and can average and/or threshold
   data to make it more reliable.  To setup the individualized signal
   processing, use the ICubeX editor program downloadable from
   www.infusionsystems.com In the editor program, it is best to set
   the midi message type for each sensor that you add to the
   configuration to "Pitch-Bend".  This captures the signal with
   14-bit accuracy as opposed to 7-bit for the other message types.
   Also, set the midi channel number to be equal to the sensor number
   on the front of the icubex device.

   This driver also supports host mode, and it automatically sets host
   mode data to be reported in hi-res (12-bit) mode.  The advantage of
   host mode is that the device does not need to be setup using the
   icubex editor program, but the disadvantage is that you don't get
   the built-in signal processing, so you'll typically have to
   renormalize, smooth, or otherwise process the sensor data that is
   output inside your own program.

   Note: This driver was written for ICubeX firmware version 4.0 but
   using the midi implementation manual for version 4.1, since no
   manual exists for 4.0.  It has been tested for version 4.0, and it
   is likely to work for version 4.1.  Beyond that, it should work
   unless ICubeX changes their data format.
  **/
  ICubeXDevice(const std::string &activeChannels, unsigned int samplingInterval,
	       int midiInDeviceNum, int midiOutDeviceNum, 
               ICubeXMode mode=STAND_ALONE_MODE, bool debug=false);

  static void printMidiDevices();

  virtual ~ICubeXDevice();

  void pollForInput(Array<EventRef> &events);

protected:
  
  void init();
  EventRef reportChannel(int channel, double data);
  
  std::string    _activeChannelsStr;
  Array<int>     _activeChannels;
  unsigned int   _samplingInterval;
  bool           _debug;
  int            _inDevNum;
  int            _outDevNum;
  MidiInDevice  *_in;
  MidiOutDevice *_out;
  double         _channelData[32];
  ICubeXMode     _mode;
};

} // end namespace

#endif
