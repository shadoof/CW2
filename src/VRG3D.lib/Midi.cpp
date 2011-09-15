
#include "Midi.H"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#  include <unistd.h>
#  include <fcntl.h>
#endif


namespace VRG3D {

#ifdef WIN32  // Win32 versions

  void reportMidiInError(unsigned int error) {
    char errorMsg[256];
    midiInGetErrorText(error, errorMsg, 256);
    printf("MidiIn Error %s\n", errorMsg);
    exit(1);
  }

  void reportMidiOutError(unsigned int error) {
    char errorMsg[256];
    midiOutGetErrorText(error, errorMsg, 256);
    printf("MidiOut Error %s\n", errorMsg);
    exit(1);
  }
  
#else  // Linux versions

  void reportMidiInError(char *errorMsg) {
    printf("MidiIn Error %s\n", errorMsg);
    exit(1);
  }
  
  void reportMidiOutError(char *errorMsg) {
    printf("MidiOut Error %s\n", errorMsg);
    exit(1);
  }

#endif



#ifdef WIN32

  // Windows uses a callback mechanism, linux needs to be polled
  void CALLBACK
  midiInWin32Callback(HMIDIIN handle, WORD midiMessage, DWORD dwInstance,
		      DWORD data, DWORD timestamp)
  {
    MidiInDevice *thisDevice = (MidiInDevice*)dwInstance;
    
    switch (midiMessage) {
    case MIM_OPEN:
      break;
    case MIM_CLOSE:
      printf("MidiIn Callback: Closing Midi.\n");
      break;
    case MIM_ERROR: {
      printf("MidiIn Callback: Midi Error.\n");
      break;
    }
    case MIM_LONGERROR: {
      printf("MidiIn Callback: Error receiving system-exclusive message.\n");
      // You'll get here on the first message from the ICubeX device if
      // it's already running and reporting data.  Do an addBuffer anyway
      // because otherwise, you won't get any more callbacks.
      if (thisDevice->continueCallback) {
        LPMIDIHDR lpMIDIHeader = (LPMIDIHDR)data;
        midiInAddBuffer(handle, lpMIDIHeader, sizeof(MIDIHDR)); 
      }
      break;
    }
    case MIM_MOREDATA:
      printf("MidiIn Callback: Application is not processing "
             "midi events fast enough.\n");
      break;
    case MIM_DATA: {
      thisDevice->addData(LOBYTE(LOWORD(data)), HIBYTE(LOWORD(data)), LOBYTE(HIWORD(data)));

      if (thisDevice->continueCallback) {
        // Queue the buffer for additional input
        LPMIDIHDR lpMIDIHeader = (LPMIDIHDR)data;
        midiInAddBuffer(handle, lpMIDIHeader, sizeof(MIDIHDR)); 
      }
      break;
    }
    case MIM_LONGDATA: {
      // MIM_LONGDATA indicates a system-exclusive buffer is being sent
      
      LPMIDIHDR lpMIDIHeader = (LPMIDIHDR)data;
      // a pointer to the bytes received in the midi buffer
      unsigned char *ptr = (unsigned char *)(lpMIDIHeader->lpData);
      unsigned char lastbyte = *(ptr + lpMIDIHeader->dwBytesRecorded - 1);
      
      // For system-exclusive midi messages:
      //   0xF0 denotes the start of the message
      //   0xF7 denotes the end of the message
      if (lastbyte == 0xF7) {
	// End of message reached, process the message
	// The first byte of the message will always be 0xF0, indicating
	// start-of-sysex-message, so skip it and also skip the last
	// byte 0xF7 = end-of-sysex-message
	int size = lpMIDIHeader->dwBytesRecorded - 2;
	thisDevice->setMessage(ptr+1, size);
      }
      
      if (thisDevice->continueCallback) {
        // Queue the buffer for additional input
        midiInAddBuffer(handle, lpMIDIHeader, sizeof(MIDIHDR)); 
      }
      break;
    }
    default:
      printf("MidiIn Callback: Unknown midi message type.\n");
      break;
    }
  }

#endif  // Windows Callback
  



MidiInDevice*
MidiInDevice::fromMidiDeviceName(const char *name)
{
  int numDevs = getNumAvailableDevices();
  int i=0;
  bool gotmatch = false;
  while ((i<numDevs) && (!gotmatch)) {
    char devname[1024];
    getAvailableDeviceName(i, devname);
    if (strcmp(devname, name) == 0)
      gotmatch = true;
    else
      i++;
  }

  if (gotmatch)
    return MidiInDevice::fromMidiDeviceNumber(i);
  else
    return NULL;
}

MidiInDevice*
MidiInDevice::fromMidiDeviceNumber(int number)
{
# ifdef WIN32
    int max = midiInGetNumDevs();
# else
    int max = 4;
# endif

  if ((number >= 0) && (number < max))
    return new MidiInDevice(number);
  else
    return NULL;
}


MidiInDevice::MidiInDevice(int number)
{
  _msgSize = 0;
  _newMsgFlag = false;
  _dataSize = 0;
  _newDataFlag = false;

# ifdef WIN32
    continueCallback = true;

    UINT error = midiInOpen(&_handle, number, (DWORD)midiInWin32Callback,
			    (DWORD)this, CALLBACK_FUNCTION);
    if (error) {
      reportMidiInError(error);
      return;
    }
    
    error = midiInReset(_handle);
    if (error) {
      reportMidiInError(error);
    return;
    }  
    
    _header.lpData = (LPSTR)_sysExBuffer;
    _header.dwBufferLength = sizeof(_sysExBuffer);
    _header.dwFlags = 0;
    
    error = midiInPrepareHeader(_handle, &_header, sizeof(MIDIHDR));
    if (error) {
      reportMidiInError(error);
      return;
    }
    
    error = midiInAddBuffer(_handle, &_header, sizeof(MIDIHDR));
    if (error) {
      reportMidiInError(error);
      return;
    }
    
    error = midiInStart(_handle);
    if (error) {
      reportMidiInError(error);
      return;
    }


# else  // Linux, etc..

    _sysExMsgSize = 0;
    char devName[1024];
    MidiInDevice::getAvailableDeviceName(number, devName);

    //cout << "about to open MidiIn" << endl;
    _handle = open(devName, O_RDONLY, 0);
    if (_handle < 0) {
      printf("MidiIn cannot open %s\n", devName);
      exit(1);
      return;
    }
    
# endif
}

MidiInDevice::~MidiInDevice()
{
  shutdown();
}

void
MidiInDevice::shutdown()
{
  //cout << "Closing MidiIn." << endl;
# ifdef WIN32
    continueCallback = false;
    midiInStop(_handle);
# endif
}

int
MidiInDevice::getNumAvailableDevices()
{
# ifdef WIN32
    return midiInGetNumDevs();
# else
    return 4;
# endif
}


void
MidiInDevice::getAvailableDeviceName(int number, char *name)
{
  strcpy(name, "");
  int numDevs = getNumAvailableDevices();
  if ((number < 0) || (number >= numDevs))
    return;

# ifdef WIN32
  
    MIDIINCAPS mic;
    UINT error = midiInGetDevCaps(number, &mic, sizeof(MIDIINCAPS));
    if (error != 0)
      reportMidiInError(error);
    else {
      strcpy(name, mic.szPname);
    }

# else

    if (number == 0)
      strcpy(name, "/dev/midi00");
    else if (number == 1)
      strcpy(name, "/dev/midi01");
    else if (number == 2)
      strcpy(name, "/dev/midi02");
    else if (number == 3)
      strcpy(name, "/dev/midi03");
    else
      strcpy(name, "");
    
# endif
}

bool
MidiInDevice::hasMessageWaiting()
{
  return _newMsgFlag;
}

bool
MidiInDevice::readMessage(unsigned char **msgPtr, int *size)
{
  if (!_newMsgFlag)
    return false;
  else {
    *msgPtr = _msg;
    *size = _msgSize;
    _newMsgFlag = false;
    _msgSize = 0;
    return true;
  }
}

void
MidiInDevice::setMessage(unsigned char *msg, int size)
{
  unsigned char *ptr = _msg;
  int free = MIDI_IN_BUFFER_SIZE;
  // if still haven't checked the old one, then append this message
  if (_newMsgFlag) {
    ptr  += _msgSize;
    free -= _msgSize;
  }
  if (size >= free) {
    printf("Error Midi-In message buffer overflow!\n");
  }
  else {
    // copy message into the message buffer
    unsigned char* ptr2 = msg;
    for (int i=0;i<size;i++) {
      *ptr = *ptr2;
      ptr++;
      ptr2++;
    }
    _msgSize += size;
    _newMsgFlag = true;
  }
}


bool
MidiInDevice::hasDataWaiting()
{
  return _newDataFlag;
}

int
MidiInDevice::readData(unsigned char* &statusBytes, unsigned char* &data1Bytes, unsigned char* &data2Bytes)
{
  if (!_newDataFlag)
    return 0;
  else {
    int size = _dataSize;
    statusBytes = _dataStatus;
    data1Bytes = _data1;
    data2Bytes = _data2;
    _dataSize = 0;
    _newDataFlag = false;
    return size;
  }
}

void
MidiInDevice::addData(unsigned char status, unsigned char data1, unsigned char data2)
{
  if (_dataSize >= MIDI_IN_DATA_BUFFER_SIZE) {
    printf("Error Midi-In data buffer overflow!\n");
  }
  else {
    _dataStatus[_dataSize] = status;
    _data1[_dataSize] = data1;
    _data2[_dataSize] = data2;
    _newDataFlag = true;
    _dataSize++;
  }
}



void
MidiInDevice::poll()
{
# ifndef WIN32
  if (_handle >= 0) {

    unsigned char *newMsgStart = _sysExBuffer + _sysExMsgSize;
    int count = read(_handle, (void*)newMsgStart,
		     MIDI_IN_BUFFER_SIZE - _sysExMsgSize);
    _sysExMsgSize += count;

    if (_sysExMsgSize >= MIDI_IN_BUFFER_SIZE) {
      printf("Error: MidiIn Buffer Overflow.\n");
      _sysExMsgSize = 0;
    }
    
    if (count > 0) {

      unsigned char lastbyte = _sysExBuffer[_sysExMsgSize-1];
      
      //cout << "first byte = " << (int)(_sysExBuffer[0]) << endl;
      //cout << "last byte = " << (int)lastbyte << endl;
      
      //for (int i=0;i<count;i++) {
      //  printf("%d: %X\n", i, (int)newMsgStart[i]);
      //}
      
      if (lastbyte == 0xF7) {
        // got end of message signal, skip first byte and last and set
        // the rest as the data message
        setMessage(_sysExBuffer+1, _sysExMsgSize-2);
        _sysExMsgSize = 0;
      }
    }
  }
# endif
}


// ----------------------------------------------------------------------------


MidiOutDevice*
MidiOutDevice::fromMidiDeviceName(const char *name)
{
  int numDevs = getNumAvailableDevices();
  int i=0;
  bool gotmatch = false;
  while ((i<numDevs) && (!gotmatch)) {
    char devname[1024];
    getAvailableDeviceName(i, devname);
    if (strcmp(devname, name) == 0)
      gotmatch = true;
    else
      i++;
  }

  if (gotmatch)
    return MidiOutDevice::fromMidiDeviceNumber(i);
  else
    return NULL;
}

MidiOutDevice*
MidiOutDevice::fromMidiDeviceNumber(int number)
{
# ifdef WIN32
    int max = midiOutGetNumDevs();
# else
    int max = 4;
# endif

  if ((number >= 0) && (number < max))
    return new MidiOutDevice(number);
  else
    return NULL;
}

MidiOutDevice::MidiOutDevice(int number)
{
# ifdef WIN32
    UINT error = midiOutOpen(&_handle, number, 0, 0, CALLBACK_NULL);
    if (error) {
      reportMidiOutError(error);
      return;
    }  
    
    error = midiOutReset(_handle);
    if (error) {
      reportMidiOutError(error);
      return;
    }
    
# else

    char devName[1024];
    MidiOutDevice::getAvailableDeviceName(number, devName);

    //cout << "about to open MidiOut" << endl;
    _handle = open(devName, O_WRONLY, 0);
    if (_handle < 0) {
      printf("MidiOut cannot open %s\n", devName);
      exit(1);
      return;
    }

# endif
}

MidiOutDevice::~MidiOutDevice()
{
# ifdef WIN32
    midiOutClose(_handle);
# endif
}


int
MidiOutDevice::getNumAvailableDevices()
{
# ifdef WIN32
    return midiOutGetNumDevs();
# else
    return 4;
# endif
}

void
MidiOutDevice::getAvailableDeviceName(int number, char *name)
{
  strcpy(name, "");
  int numDevs = getNumAvailableDevices();
  if ((number < 0) || (number >= numDevs))
    return;

# ifdef WIN32

    MIDIOUTCAPS moc;
    UINT error = midiOutGetDevCaps(number, &moc, sizeof(MIDIOUTCAPS));
    if (error != 0)
      reportMidiOutError(error);
    else {
      strcpy(name, moc.szPname);
    }
    
# else

    if (number == 0)
      strcpy(name, "/dev/midi00");
    else if (number == 1)
      strcpy(name, "/dev/midi01");
    else if (number == 2)
      strcpy(name, "/dev/midi02");
    else if (number == 3)
      strcpy(name, "/dev/midi03");
    else
      strcpy(name, "");
    
# endif
}

void
MidiOutDevice::sendMessage(unsigned char* message, int size)
{
# ifdef WIN32
  MIDIHDR header;
  header.lpData = (LPSTR)message;
  header.dwBufferLength = size;
  header.dwFlags = 0;

  UINT error = midiOutPrepareHeader(_handle, &header, sizeof(MIDIHDR));
  if (error) {
    reportMidiOutError(error);
    return;
  }

  // Send the system-exclusive message
  error = midiOutLongMsg(_handle, &header, sizeof(MIDIHDR));
  if (error) {
    reportMidiOutError(error);
    return;
  }

  // Wait until buffer can be unprepared.
  while (MIDIERR_STILLPLAYING == midiOutUnprepareHeader(_handle, &header, sizeof(MIDIHDR))) {}

# else
  
  write(_handle, message, size);

# endif
}



} // end namespace



