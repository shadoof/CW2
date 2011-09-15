

#include "ICubeXDevice.H"
#include "G3DOperators.H"


namespace VRG3D {

void
ICubeXDevice::printMidiDevices()
{
  cout << "MIDI IN Devices:" << endl;
  for (int i=0;i<MidiInDevice::getNumAvailableDevices();i++) {
    char name[1024];
    MidiInDevice::getAvailableDeviceName(i, name);
    cout << i << ": " << name << endl;
  }

  cout << "MIDI OUT Devices:" << endl;
  for (int i=0;i<MidiOutDevice::getNumAvailableDevices();i++) {
    char name[1024];
    MidiOutDevice::getAvailableDeviceName(i, name);
    cout << i << ": " << name << endl;
  }
}


ICubeXDevice::ICubeXDevice(const std::string &activeChannels, unsigned int samplingInterval, 
                           int midiInDeviceNum, int midiOutDeviceNum, ICubeXMode mode, bool debug) :
  InputDevice()
{
  _activeChannelsStr = activeChannels;
  _samplingInterval = samplingInterval;
  _inDevNum  = midiInDeviceNum;
  _outDevNum = midiOutDeviceNum;
  _mode = mode;
  _debug = debug;
  init();
}


void
ICubeXDevice::init()
{
  for (int i=0;i<32;i++)
    _channelData[i] = 0.0;
  
  Array<std::string> channelList = splitStringIntoArray(_activeChannelsStr);
  for (int i=0;i<channelList.size();i++) {
    int channel = stringToInt(channelList[i]);
    debugAssert(channel <= 32);
    debugAssert(channel >= 1);
    _activeChannels.append(channel-1);
  }

  if (_debug) {
    printMidiDevices();
    cout << "ICubeX:: Active Channels = [ ";
    for (int i=0;i<_activeChannels.size();i++)
      cout << intToString(_activeChannels[i]+1) << " ";
    cout << "]" << endl;
  }

  // Open Midi Out device
  _out = MidiOutDevice::fromMidiDeviceNumber(_outDevNum);
  


  // Send config messages to Midi-Out

  // Reset the device each time to avoid bad initial data
  unsigned char resetMsg[] = {0xF0, 0x7D, 0x00, 0x22, 0xF7};
  _out->sendMessage(resetMsg, sizeof(resetMsg));

  if (_mode == STAND_ALONE_MODE) {
    // Set device to stand-alone mode
    unsigned char setStandAloneModeMsg[] = {0xF0, 0x7D, 0x00, 0x5A, 0x01, 0xF7};
    _out->sendMessage(setStandAloneModeMsg, sizeof(setStandAloneModeMsg));
  }
  else {
    // Set device to host mode
    unsigned char setHostModeMsg[] = {0xF0, 0x7D, 0x00, 0x5A, 0x00, 0xF7};
    _out->sendMessage(setHostModeMsg, sizeof(setHostModeMsg));
    
    // Set the resolution for each channel to 12-bit rather than the 7-bit default
    unsigned char hiResMsg[] = {0xF0, 0x7D, 0x00, 0x02, 0x40, 0xF7};
    for (int i=0;i<32;i++) {
      hiResMsg[4] = 0x40 + i;
      _out->sendMessage(hiResMsg, sizeof(hiResMsg));
    }
  }

  // Set the sampling interval, hardware default is 100ms
  unsigned char msb = _samplingInterval / 128;
  unsigned char lsb = _samplingInterval % 128;  
  unsigned char setIntervalMsg[] = {0xF0, 0x7D, 0x00, 0x03, 0x00, 0x64, 0xF7};
  setIntervalMsg[4] = msb;
  setIntervalMsg[5] = lsb;
  _out->sendMessage(setIntervalMsg, sizeof(setIntervalMsg));
  
  // Send the STREAM command to each active channel, turn off streaming for non-active channels
  unsigned char enableChannelMsg[] = {0xF0, 0x7D, 0x00, 0x01, 0x40, 0xF7};
  for (int i=0;i<32;i++) {
    if (_activeChannels.contains(i)) {
      enableChannelMsg[4] = 0x40 + i;
    }
    else {
      enableChannelMsg[4] = 0x00 + i;
    }
    _out->sendMessage(enableChannelMsg, sizeof(enableChannelMsg));
  }


  // Open MidiIn for listening
  _in  = MidiInDevice::fromMidiDeviceNumber(_inDevNum);
}

ICubeXDevice::~ICubeXDevice()
{
  if (_debug) {
    cout << "Deleting ICubeX Device." << endl;
  }
  _in->shutdown();
  delete _in;
  delete _out;
}

EventRef
ICubeXDevice::reportChannel(int channel, double data)
{
  debugAssert(channel < 32);
  debugAssert(channel >= 0);

  if (data != _channelData[channel]) {
    _channelData[channel] = data;
    
    std::string channelStr = intToString(channel+1);
    if (channel+1 < 10)
      channelStr = std::string("0") + channelStr;

    return new Event("ICubeX_" + channelStr, data);
  }
  return NULL;
}


void
ICubeXDevice::pollForInput(Array<EventRef> &events)
{
#ifndef WIN32
  _in->poll();
#endif

  if (_mode == STAND_ALONE_MODE) {
  
    // In Stand-Alone mode, data is reported in Midi Data messages  
    if (_in->hasDataWaiting()) {
      unsigned char *statusBuf;
      unsigned char *data1Buf;
      unsigned char *data2Buf;
      int dsize = _in->readData(statusBuf, data1Buf, data2Buf);
      
      for (int i=0;i<dsize;i++) {
        unsigned char midiMsgType = (unsigned char)(statusBuf[i] & (unsigned char)0xf0);
        unsigned char channel = (unsigned char)(statusBuf[i] & (unsigned char)0x0f);

        if (midiMsgType == 0xE0) {
          // Got pitch-bend message, 14-bit data, [0..16383]
          int data = data1Buf[i] + 128*data2Buf[i];
          EventRef e = reportChannel(channel, (double)data / 16383.0);
          if (e.notNull()) {
            events.append(e);
          }
        }
        else {
          // Got other message, 7-bit data, [0..127]
          int data = data2Buf[i];
          EventRef e = reportChannel(channel, (double)data / 127.0);
          if (e.notNull()) {
            events.append(e);
          }
        }
      }
    }
  }

  else {

    // In host mode, data is reported in SysEx messages.
    int size;
    unsigned char* msg;
    if (_in->readMessage(&msg, &size)) {
      /** ICubeX streaming data message format for host mode:
          0x7D :  Manufacturer ID
          0x00 :  Device ID (zero unless multiple devices are chained)
          0x00 :  Message type 0 (STREAM)
          
          for 7-bit lo-res mode 1 byte [0..127] for each active sensor
          OR
          for 12-bit hi-res mode 2 bytes for each active sensor in this format:
          0yyyyyyy, 000zzzzz
          yyyyyyy = [0..127]; the most significant bits of data
          zzzzz = [0..31]; the 5 least significant bits

          We set all the channels to be hi-res in the init() function.
      **/

      int expectedSize = 2*_activeChannels.size() + 3;
      if (size % expectedSize != 0) {
        if (_debug) {
          cerr << "Unexpected midi message size (" << size 
               << ") from ICubeX device - ignoring message." << endl;
          for (int i=0;i<size;i++) {
            printf("BYTE %d: %X\n", i, (int)msg[i]);
          }
        }
      }
      else {

        if (_debug) {
          for (int i=0;i<size;i++) {
            printf("BYTE %d: %X\n", i, (int)msg[i]);
          }
        }
        
        int nMessages = size / expectedSize;
        for (int n=0;n<nMessages;n++) {
          int msgOffset = n*expectedSize;
          for (int i=0;i<_activeChannels.size();i++) {
            int iMSB = 2*i + 3 + msgOffset;
            int iLSB = 2*i + 4 + msgOffset;
            int dataMSB = (int)msg[iMSB];
            int dataLSB = (int)msg[iLSB];
            int data = dataLSB + 32*dataMSB;
            
            if (_debug) {
              cout << "Raw data [" << i << "] = " << data << endl;
            }
            
            // for 12-bit data range is [0..4095], renormalize to [0..1]
            EventRef e = reportChannel(i, (double)data / 4095.0);
            if (e.notNull()) {
              events.append(e);
            }
          }
        }
      }
    }
  }
}


} // end namespace


