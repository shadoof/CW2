

#ifndef MIDI_H
#define MIDI_H

#include <G3D/G3DAll.h>
#include <iostream>
using namespace std;

#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

namespace VRG3D {

#define MIDI_IN_BUFFER_SIZE      1024
#define MIDI_IN_DATA_BUFFER_SIZE  256

class MidiInDevice
{
public:
  static MidiInDevice* fromMidiDeviceName(const char *name);
  static MidiInDevice* fromMidiDeviceNumber(int number);
  static int           getNumAvailableDevices();
  static void          getAvailableDeviceName(int number, char *name);

  MidiInDevice(int number);
  virtual  ~MidiInDevice();


  /// Returns true if midi messages have come in since the last
  /// call to readMessage.
  bool  hasMessageWaiting();

  /**
     To avoid copy arrays around, this returns a pointer to the
     message buffer.  But, the next time the device is polled, any new
     messages that came in will overwrite data in the message buffer.
     So, process the message immediately after this call, or copy it
     to another buffer on your own.  Calling this function resets the
     hasMessageWaiting flag to false.  returns true if
     haveMessageWaiting is true when the function is called.
  */
  bool  readMessage(unsigned char **msgPtr, int *size);


  /// Returns true if a call to readData will return some data
  bool hasDataWaiting();

  /// Returns size of the data arrays
  int readData(unsigned char* &statusBytes, unsigned char* &data1Bytes, unsigned char* &data2Bytes);



  /// poll is only used in linux, a callback is used in Win32 midi
  void  poll();

  /// for internal use only
  void setMessage(unsigned char *msg, int size);
  void addData(unsigned char status, unsigned char data1, unsigned char data2);  

  void shutdown();

#ifdef WIN32
  bool continueCallback;
#endif

private:

#ifdef WIN32
  HMIDIIN        _handle;
  MIDIHDR        _header;
#else
  // a file descriptor
  int            _handle;
  int            _sysExMsgSize;
#endif

  // buffer filled up by the midi callback in windows and the poll 
  // routine in linux
  unsigned char  _sysExBuffer[MIDI_IN_BUFFER_SIZE];

  // buffer filled up by this class with data extracted from the
  // bytes received from the device
  unsigned char  _msg[MIDI_IN_BUFFER_SIZE];
  int            _msgSize;
  bool           _newMsgFlag;

  unsigned char  _dataStatus[MIDI_IN_DATA_BUFFER_SIZE];
  unsigned char  _data1[MIDI_IN_DATA_BUFFER_SIZE];
  unsigned char  _data2[MIDI_IN_DATA_BUFFER_SIZE];
  int            _dataSize;
  bool           _newDataFlag;
};




class MidiOutDevice
{
public:
  static MidiOutDevice*  fromMidiDeviceName(const char *name);
  static MidiOutDevice*  fromMidiDeviceNumber(int number);
  static int             getNumAvailableDevices();
  static void            getAvailableDeviceName(int number, char *name);
  
  MidiOutDevice(int number);
  virtual ~MidiOutDevice();

  void sendMessage(unsigned char* message, int size);

private:

#ifdef WIN32
  HMIDIOUT  _handle;
#else
  int _handle;
#endif
};

} // end namespace

#endif


