/**
 * /author Dan Keefe (dfk)
 *
 * /file  vrg3d-server.cpp
 * /brief 
 *
 */

#include <VRG3D.H>

using namespace VRG3D;

#define DEVICE_THREAD_SLEEP 1
#define WAIT_FOR_EVENTREQ_SLEEP 0.5 //0.0
#define WAIT_FOR_SWAPREQ_SLEEP 0.5 //0.0
#define INPUT_WINDOW_SLEEP 10

// Displays a window and grabs mouse and kbd input events
GThreadRef gServerThread;

// Continuously polls input devices for input
GThreadRef gDeviceThread;

// Global event buffer shared between device thread and main server program
Array<VRG3D::EventRef> gDeviceEventBuffer;

// Mutex on the global event buffer that is shared between the device thread
// and the main server program
GMutex  gDeviceEventBufferMutex;

// Global pointer to the input window
class InputWindow;
InputWindow *gInputWindow = NULL;

// All threads should exit when this is set to true
bool gShutdown = false;
GMutex gShutdownLock;

std::string gDeviceFile;

void showUsage() {
  cerr << "vrg3d-server - There are two ways to start up the server:" << endl;
  cerr << "1. vrg3d-server [-nogfx] known-setup-name" << endl;
  cerr << "2. vrg3d-server [-nogfx] device-filename [numclients] [port]" << endl;
  cerr << endl;
  cerr << "  -nogfx  is optional and starts the server in a shell that does not" << endl;
  cerr << "          capture kbd or mouse input." << endl;
  cerr << "  known-setup-name  is currently one of {cave, cit411}." << endl;
  cerr << "  device-filename  specifies a config file for input devices to start." << endl;
  cerr << "  numclients  is the number of clients to expect to connect to the server." << endl;
  cerr << "  port  is the port that the server and clients should communicate on." << endl;
}


/** The input window thread renders and grabs input from this window,
   while the device input thread treats the class as an input device
   and polls it for events that it generates from the mouse and kbd.
 */
class InputWindow : public VRApp, public InputDevice
{
public:
  InputWindow() : VRApp() {
    _clearColor = Color3(0.82, 0.82, 0.61);
    capturedFocus = false;
  }

  virtual ~InputWindow() {}
	
  void bringToTop() {
    #ifdef WIN32
      Win32Window *win32 = (Win32Window*)_gwindow;
      if (win32) {
        BringWindowToTop(win32->hwnd());
      }
    #endif
  }

  virtual void doUserInput(Array<VRG3D::EventRef> &events) {

    if (!capturedFocus) {
      _gwindow->incInputCaptureCount();
      capturedFocus = true;
    }
    //_gwindow->setRelativeMousePosition(_renderDevice->width()/2, _renderDevice->height()/2);
    
    for (int i=0;i<events.size();i++) {
      if (events[i]->getName() == "kbd_ESC_down") {
        /** Shutting all these threads down properly is difficult to
            do.  I'm getting error messages in Windows about memory
            that cannot be read.  I think there is a race condition
            somewhere in here if you try to shut things down one by
            one.  So, for now just calling exit(0) to avoid getting a
            popup window on Windows that prevents future versions of
            the server from starting. **/

        events.append(new Event("Shutdown")); 
        cout << "ESC key pressed.  Shutting down now." << endl;

        // Setting this global var should cause both threads to exit.
        gShutdownLock.lock();
        gShutdown = true;
        gShutdownLock.unlock();
        
        if (gDeviceThread.notNull()) {
          gDeviceThread->waitForCompletion();
          cout << "Device thread shutdown." << endl;
        }
        if (gServerThread.notNull()) {
          gServerThread->waitForCompletion();
          cout << "Server thread shutdown." << endl;
        }

        // This tells the InputWindow, which is a VRApp to shutdown.
        _endProgram = true;
      }
      
    }

    lock.lock();
    eventBuffer.append(events);
    lock.unlock();
  }

  virtual void doGraphics(RenderDevice *rd) {
    if (font.isNull()) {
      std::string fontfile = VRApp::findVRG3DDataFile("eurostyle.fnt");
      if (fileExists(fontfile)) {
        font = GFont::fromFile(fontfile);
      }
    }
    else {
      rd->push2D();
      double size = 20.0;
      Vector2 pos = Vector2(_renderDevice->width()/2,_renderDevice->height()/2);
      font->draw2D(rd, "VRG3D Cluster Event Server", pos, size, 
                   Color3(0.1, 0.1, 0.44), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
      pos[1] += 2*size;
      font->draw2D(rd, "Press ESC to shutdown your cluster program.", pos, size, 
                   Color3(0.1, 0.1, 0.44), Color4::clear(), GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
      rd->pop2D();
    }
    vrg3dSleepMsecs(INPUT_WINDOW_SLEEP);
  }

  /// Called by the server thread to get events generated in the gfx window
  virtual void pollForInput(Array<VRG3D::EventRef> &events) {
    lock.lock();
    events.append(eventBuffer);
    eventBuffer.clear();
    lock.unlock();
  }

  virtual void shutdown() {
    _endProgram = true;
  }

private:
  Array<VRG3D::EventRef> eventBuffer;
  GFontRef               font;
  GMutex                 lock; 
  bool                   capturedFocus;
};



void device_input_thread_func(void* param) 
{
  Array<InputDevice*> inputDevices;
  if (gDeviceFile != "") {
    Log *log = new Log("server-device-thread-log.txt");
    VRApp::setupInputDevicesFromConfigFile(gDeviceFile, log, inputDevices);
  }

  if (gInputWindow != NULL) {
    inputDevices.append(gInputWindow);
  }

  Array<VRG3D::EventRef> events;
  bool done = false;
  while (!done) {
    // poll devices, put data into temp events array
    events.clear();
    for (int i=0;i<inputDevices.size();i++) {
      inputDevices[i]->pollForInput(events);
    }

    // copy over the temp array into the global array
    gDeviceEventBufferMutex.lock();
    gDeviceEventBuffer.append(events);
    gDeviceEventBufferMutex.unlock();
    
    vrg3dSleepMsecs(DEVICE_THREAD_SLEEP);
    gShutdownLock.lock();
    if (gShutdown) {
      done = true;
    }
    gShutdownLock.unlock();
  }  

  // delete input devices.. except for gInputWindow which is
  // controlled by the other thread
  for (int i=0;i<inputDevices.size();i++) {
    if (inputDevices[i] != gInputWindow) {
      delete inputDevices[i];
    }
  }
}




/// Server written with G3D networking rather than cluster-sync
class VRG3DEventServer
{
public:
  VRG3DEventServer(Log *log) { _log = log; }

  virtual ~VRG3DEventServer() {}

  virtual void waitForConnections(int numClients, int port) {
    networkDevice = NetworkDevice::instance();
    
    // doesn't look like it uses TCP_NODELAY -- does it need to?
    netListener = NetListener::create(port);

    NetAddress myAddress(networkDevice->localHostName(), port);
    cout << "Event Server running on " << networkDevice->localHostName() 
         << " on port " << port << " a.k.a. " << myAddress.toString() << endl;

    double startWaiting = System::time();
    double waitTime = 20.0;
    cout << endl << "Listening for connections, will wait a max of " << waitTime << " seconds.." << endl;

    while (netClients.size() < numClients) {
      if (netListener->clientWaiting()) {
        // this does look like it sets TCP_NODELAY
        ReliableConduitRef conduit = netListener->waitForConnection();
        alwaysAssertM(conduit.notNull(), "Could not create conduit even though client is waiting.");
        cout << "  Got connection from " << conduit->address().toString() << endl;
        netClients.append(conduit);
      }
      else {
        vrg3dSleepMsecs(0.001);
        checkConnections();
      }

      if (System::time() > startWaiting + waitTime) {
        // Took too long to connect to all the rendering nodes, something must have gone wrong.
        cout << "Did not connect to all nodes after " << waitTime << " seconds, so quiting." << endl;
        exit(0);
      }
    }
  }

  virtual void waitForEventRequest() {
    while (true) {
      checkConnections();
      for (int i=0;i<netClients.size();i++) {
        if (netClients[i]->messageWaiting()) {
          if (netClients[i]->waitingMessageType() != EVENTNETMSG_TYPE) {
            cerr << "Server: Expected Event Request msg, got something else." << endl;
            exit(0);
          }
          else {
            EventNetMsg msg;
            netClients[i]->receive(msg);
            if (msg.event->getName() == "ReqEvents") {
              return;
            }
          }
        }
      }
      vrg3dSleepMsecs(WAIT_FOR_EVENTREQ_SLEEP);
    }
  }
 
  virtual void sendEventsToClients(const Array<VRG3D::EventRef> &events) {
    checkConnections();
    EventBufferNetMsg msg(events);
    ReliableConduit::multisend(netClients, msg.type(), msg);
  }

  virtual void syncSwapBuffers() {
    // wait until everyone is ready to swap
    int numReady = 0;
    while (numReady < netClients.size()) {
      checkConnections();
      for (int i=0;i<netClients.size();i++) {
        if (netClients[i]->messageWaiting()) {
          if (netClients[i]->waitingMessageType() != EVENTNETMSG_TYPE) {
            cerr << "Server: Expected Swap msg, got something else." << endl;
            exit(0);
          }
          EventNetMsg readyToSwapMsg;
          netClients[i]->receive(readyToSwapMsg);
          // could either be a Swap msg or a ReqEvents msg that we ignored earlier
          if (readyToSwapMsg.event->getName() == "Swap") {
            numReady++;
          }
        }
      }
      vrg3dSleepMsecs(WAIT_FOR_SWAPREQ_SLEEP);
    }
    
    // send out an ok to swap msg
    EventNetMsg msg(new Event("SwapOK"));
    ReliableConduit::multisend(netClients, msg.type(), msg);
  }

  void checkConnections() {
    for (int i=0;i<netClients.size();i++) {
      if (!netClients[i]->ok()) {
        cerr << "Lost connection to " << netClients[i]->address().toString() << "." << endl; 
        cerr << "Server shutting down." << endl;
        exit(0);
      }
    }
  }

  void mainloop() {
    // wait for a request for events from one of the clients
    waitForEventRequest();
    
    // prepend an event for the synchronized time, then send the
    // current event queue to everybody, then clear it out
    gDeviceEventBufferMutex.lock();
    Array<VRG3D::EventRef> allEvents;
    double now = System::time();
    allEvents.append(new Event("SynchedTime", now));
    allEvents.append(gDeviceEventBuffer);
    sendEventsToClients(allEvents);
    gDeviceEventBuffer.clear();
    gDeviceEventBufferMutex.unlock();
    
    // wait for all clients to finish rendering then send an okToSwap signal to
    // all of them to synchronize the swap buffers call
    syncSwapBuffers();
  }

protected:
  Log                       *_log;
  NetworkDevice             *networkDevice;
  NetListenerRef             netListener;
  Array<ReliableConduitRef>  netClients;
};



VRG3DEventServer *gServer = NULL;

void server_thread_func(void* param)
{
  bool done = false;
  while (!done) {
    gServer->mainloop();
    gShutdownLock.lock();
    if (gShutdown) {
      done = true;
    }
    gShutdownLock.unlock();
  }
}



int main(int argc, char **argv)
{
  // defaults
  bool useGfx = true;
  int port = CLUSTER_NET_PORT;
  int numClients = 1;
  int bufferSize = 100000;

  if ((argc > 1) && (std::string(argv[1]) == "-h")) {
    showUsage();
    exit(0);
  }

  int i=1;
  if ((argc > 1) && (std::string(argv[1]) == "-nogfx")) {
    useGfx = false;
    i++;
  }

  if (argc <= i) {
    showUsage();
    exit(0);
  }

  // first, check for known server setups
  std::string argStr = argv[i];
  if (argStr == "cave") {
    cout << "Loading devices for Brown's CAVE." << endl;
    gDeviceFile = VRApp::findVRG3DDataFile("cave-devices.cfg");
    numClients = 4;
  }
  else if (argStr == "cave-on-audiocave") {
    cout << "Loading devices for Brown's CAVE controlled by audiocave." << endl;
    gDeviceFile = VRApp::findVRG3DDataFile("cave-devices-audiocave.cfg");
    numClients = 4;
  }
  else if (argStr == "cavefront-on-audiocave") {
    cout << "Loading devices for Brown's CAVE (front wall only) controlled by audiocave." << endl;
    gDeviceFile = VRApp::findVRG3DDataFile("cave-devices-audiocave.cfg");
    numClients = 1;
  }
  else if (argStr == "dive") {
    cout << "Loading devices for Duke's DiVE." << endl;
    gDeviceFile = VRApp::findVRG3DDataFile("dive-devices.cfg");
    numClients = 6;    
  }
  else if (argStr == "desktop") {
    cout << "Using desktop setup." << endl;
    numClients = 1;
  }

  // here, no pre-defined setup, so customize based on arguments
  else {
    gDeviceFile = argStr;
    i++;
    if (argc > i) {
      numClients = stringToInt(std::string(argv[i]));
      i++;
      if (argc > i) {
        port = stringToInt(std::string(argv[i]));
        i++;
      }
    }
  }

  Log *log = new Log("server-log.txt");
  gServer = new VRG3DEventServer(log);

  if (useGfx) {
    gInputWindow = new InputWindow();
  }

  // Don't start polling devices until everybody is connected..
  gServer->waitForConnections(numClients, port);

  gDeviceThread = GThread::create("DeviceInputThread", device_input_thread_func, 0);
  if (!gDeviceThread->start()) {
    cerr << "Problem starting DeviceInputThread." << endl;
    exit(1);
  }
  
  if (useGfx) {
    // The main program thread will be used to run the graphics
    // application, so create a separate thread here to run the
    // server.
    gServerThread = GThread::create("ServerThread", server_thread_func, 0);
    if (!gServerThread->start()) {
      cerr << "Problem starting ServerThread." << endl;
      exit(1);
    }
    Log *iwinlog = new Log("server-gfxlog.txt");
    //gInputWindow->init("desktopfullscreen", iwinlog);
    gInputWindow->init("desktop", iwinlog);
    gInputWindow->bringToTop();
    gInputWindow->run();
  }
  else {
    // In this case, the current (main program) thread is not needed
    // to run the graphics application, so don't bother creating a
    // separate thread to run the server, just run it directly.
    server_thread_func(0);
  }

  return 0;
}

