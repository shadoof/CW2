#include "VRApp.H"
#include "CheckerboardStencil.H"
#include "ConfigMap.H"
#include "EventNet.H"
#include "SynchedSystem.H"

#include "PlatformDefs.h"

#ifdef USE_GLUT
#  include "GlutWindow.h"
#endif

//#ifdef USE_SDL
// for SDL_Init()
//#  include <SDL/SDL.h>
//#  include <GLG3D/SDLWindow.h>
//#endif

// devices
#include "ICubeXDevice.H"
#include "ISenseDirect.H"
//#include "SpaceNavDevice.H"
#include "TrackDClient.H"
#include "VRPNAnalogDevice.H"
#include "VRPNButtonDevice.H"
#include "VRPNTrackerDevice.H"



#include <iostream>
using namespace std;


// if the program is a client and the server is running on the same machine, then
// it needs to sleep a bit to allow the server to have some cycles
#define CLIENT_SLEEP 0.0005


namespace VRG3D {

static void gWindowLoopCallback(void *ptrToVRApp) {
  VRApp *app = (VRApp*)ptrToVRApp;
  app->oneFrame();
  if (app->getReadyToEndProgram()) {
    app->getRenderDevice()->window()->popLoopBody();
    // bug? the pop above doesn't seem to work
    exit(0);
  }
}


VRApp::VRApp() :
  _log(NULL), 
  _tile(DisplayTile::defaultDesktopTile()), 
  _gwindow(NULL), 
  _renderDevice(NULL), 
  _endProgram(false),
  _frameCounter(0),
  _isAClusterClient(false),
  _clearColor(Color4(0,0,0,1)),
  _clusterServerName("127.0.0.1")  // localhost
{
}

VRApp::~VRApp()
{
}


void
VRApp::init(const std::string &vrSetup, Log *appLog) 
{
#ifdef USE_CLUSTER
  clusterSynchIt = NULL;
  clusterSynchItBuf = NULL;
#endif

  if (_log == NULL) {
    _log = new Log("log.txt");
  }

  GWindow::Settings gwinsettings;
  bool hideMouse = false;
  double nearclip = 0.01;
  double farclip = 100.0;
  DisplayTile::TileRenderType renderType = DisplayTile::TILE_MONO;
  CoordinateFrame initialHeadFrame = CoordinateFrame(Vector3(0,0,1));
  Vector3 topLeft  = Vector3(-0.65, 0.5, 0.0);
  Vector3 topRight = Vector3( 0.65, 0.5, 0.0);
  Vector3 botLeft  = Vector3(-0.65,-0.5, 0.0);
  Vector3 botRight = Vector3( 0.65,-0.5, 0.0);
  bool tileAutoAdjustToResolution = false;

  std::string configfile;
  if (vrSetup != "") {
    configfile = findVRG3DDataFile(vrSetup + ".vrsetup");
  }
  if ((configfile != "") && (fileExists(configfile))) {
   
    ConfigMapRef map = new ConfigMap(configfile, _log);
  
    int iFalse = 0;
    _isAClusterClient = map->get("ClusterClient", iFalse);
    _clusterServerName = map->get("ClusterServerHost", "myserver.cs.brown.edu");
    
    std::string devCfgFile = map->get("InputDevicesFile", "");
    if (devCfgFile != "") {
      setupInputDevicesFromConfigFile(findVRG3DDataFile(devCfgFile), _log, _inputDevices);
    }

    if ((map->get("WindowWidth", "1280") == "max") || (map->get("WindowHeight", "1024") == "max")) {
    #ifdef WIN32
      gwinsettings.width  = GetSystemMetrics(78);
      gwinsettings.height  = GetSystemMetrics(79);
      _log->println("Max display dimensions reported by Windows: " + intToString(gwinsettings.width) 
                    + " x " + intToString(gwinsettings.height));
    #else
      /* // This is a bit hacky.. trying to figure out the max (best) screen resolution.
		   // The problem is we need to start SDL in order to query the resolution.  So, 
		   // we create a temporary SDL window here, query the resolution, and later create
		   // our rendering window using this resolution.  In recent versions of SDL asking
		   // for a window of size 0 x 0 gives us SDL's default "best" resolution.
		   gwinsettings.width = 0;
		   gwinsettings.height = 0;
		   GWindow *tmpWin = OSWindow::create(gwinsettings);
		   const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
		   gwinsettings.width = vinfo->current_w;
		   gwinsettings.height = vinfo->current_h;
		   delete tmpWin;
		   _log->println("Max display dimensions reported by SDL: " + intToString(gwinsettings.width) 
					     + " x " + intToString(gwinsettings.height));
	   */
		gwinsettings.width = 1280;
		gwinsettings.height = 1024;
    #endif
    }
    else {
      gwinsettings.width       = map->get("WindowWidth", gwinsettings.width);
      gwinsettings.height      = map->get("WindowHeight", gwinsettings.height);
    }

    gwinsettings.x             = map->get("WindowX", gwinsettings.x);
    gwinsettings.y             = map->get("WindowY", gwinsettings.y);
    gwinsettings.center        = map->get("WindowCenter", gwinsettings.center);
    gwinsettings.rgbBits       = map->get("WindowRGBBits", gwinsettings.rgbBits);
    gwinsettings.alphaBits     = map->get("WindowAlphaBits", gwinsettings.alphaBits);
    gwinsettings.depthBits     = map->get("WindowDepthBits", gwinsettings.depthBits);
    gwinsettings.stencilBits   = map->get("WindowStencilBits", gwinsettings.stencilBits);
    gwinsettings.fsaaSamples   = map->get("WindowFSAASamples", gwinsettings.fsaaSamples);
    gwinsettings.fullScreen    = map->get("WindowFullScreen", gwinsettings.fullScreen);
    gwinsettings.asynchronous   = map->get("WindowAsychronous", gwinsettings.asynchronous);
    gwinsettings.stereo        = map->get("WindowStereo", gwinsettings.stereo);
    gwinsettings.refreshRate   = map->get("WindowRefreshRate", gwinsettings.refreshRate);
    gwinsettings.resizable     = map->get("WindowResizable", gwinsettings.resizable);
    gwinsettings.allowMaximize = map->get("WindowAllowMaximize", gwinsettings.allowMaximize);
    gwinsettings.framed        = map->get("WindowFramed", gwinsettings.framed);
    gwinsettings.caption       = map->get("WindowCaption", gwinsettings.caption);

#ifdef WIN32
    if (gwinsettings.fullScreen) {
      gwinsettings.width  = GetSystemMetrics(SM_CXSCREEN);// * 3/4;
      gwinsettings.height = GetSystemMetrics(SM_CYSCREEN);// * 3/4;
      gwinsettings.framed = false;
      gwinsettings.fullScreen = false;
    }
#endif

    if (!map->get("WindowMouseVisible", 1)) {
      hideMouse = true;
    }
  
    topLeft  = map->get("TileTopLeft", Vector3(-0.65, 0.5, 0.0));
    topRight = map->get("TileTopRight", Vector3( 0.65, 0.5, 0.0));
    botLeft  = map->get("TileBotLeft", Vector3(-0.65,-0.5, 0.0));
    botRight = map->get("TileBotRight", Vector3( 0.65,-0.5, 0.0));
    nearclip = map->get("TileNearClip", 0.01);
    farclip  = map->get("TileFarClip", 100.0);

    if (gwinsettings.stereo) {
      // If we're creating a stereo window, then set the display render type to stereo
      renderType = DisplayTile::TILE_STEREO;
    }
    else {
      // Else default to mono unless any of these other special
      // rendering types were specified as the TileRenderType.
      std::string rtStr = map->get("TileRenderType", "MONO");
      if (rtStr == "MONO_LEFT") {
        renderType = DisplayTile::TILE_MONO_LEFT;
      }
      else if (rtStr == "MONO_RIGHT") {
        renderType = DisplayTile::TILE_MONO_RIGHT;
      }
      else if (rtStr == "CHECKERBOARD_STEREO") {
        renderType = DisplayTile::TILE_CHECKERBOARD_STEREO;
      }
    }
    
    _tile = DisplayTile(topLeft, topRight, botLeft, botRight, 
                        renderType, nearclip, farclip);    
    initialHeadFrame = map->get("TileInitialHeadFrame", CoordinateFrame(Vector3(0,0,1)));
    _camera = new ProjectionVRCamera(_tile, initialHeadFrame); 

    if (map->get("TileAutoAdjustToResolution", iFalse) == 1) {
      tileAutoAdjustToResolution = true;
    }
  }
  else {
    // Can't find VR setup config file
    Array<std::string> configFiles;
    std::string pattern = replaceEnvVars("../config/") + "*.vrsetup";
    getFiles(pattern, configFiles, true);
    
    if (vrSetup == "") {
      cerr << "No argument specified for a VR setup to load." << endl;
    }
    else {
      cerr << "Could not find a valid VR setup named: " << vrSetup << endl;
    }
    cerr << "Continuting with a default desktop setup." << endl;
    cerr << "-----------------------------------------------------------------" << endl;
    cerr << "Installed VR setups are listed below:" << endl;
    
    Array<std::string> name, desc;
    for (int i=0;i<configFiles.size();i++) {
      std::string drive, base, ext;
      Array<std::string> path;
      parseFilename(configFiles[i], drive, path, base, ext);
      name.append(base);
      ConfigMapRef map = new ConfigMap(configFiles[i], _log);
      desc.append(map->get("Description", "Unknown description"));      
    }
    int maxLen = 0;
    for (int i=0;i<name.size();i++) {
      if (name[i].size() > maxLen) {
        maxLen = name[i].size();
      }
    }
    for (int i=0;i<name.size();i++) {
      cerr << name[i];
      int nspaces = maxLen - name[i].size() + 2;
      for (int s=0;s<nspaces;s++) {
        cerr << " ";
      }
      cerr << desc[i] << endl;
    }
    cerr << "-----------------------------------------------------------------" << endl;

    _tile = DisplayTile::defaultDesktopTile();
    _camera = new ProjectionVRCamera(_tile, initialHeadFrame); 
  }
  

  /* Note: there is a bug currently in some nvidia drivers - they
   can't open fullscreen stereo windows via whatever method SDLWindow
   and X11Window use internally, but GlutWindow works.  Ideally we
   would simply use Win32Window on Windows and X11Window on everything
   else, but until these implementations are ready for prime time use,
   we detect the buggy situation and use GlutWindow.  USE_GLUT needs
   to be defined in the Makefiles to compile and link in this Glut
   code. 
   Brown Note: SDLWindow works on CS style linux machines, but not 
   on CASCV's CentOS machines at the Cave.
  */
#ifdef USE_GLUT
    cout << "Starting a GlutWindow -- USE_GLUT was defined when compiling VRApp." << endl;
    _gwindow = new GlutWindow(gwinsettings);
#else
	_gwindow = OSWindow::create(gwinsettings);
#endif

  _renderDevice = new RenderDevice();
  _renderDevice->init(_gwindow, _log);
  _renderDevice->resetState();
  
  // Expand the tile in room coordinates to agree with the aspect
  // ratio of the display.  For a VR display, you should really
  // measure the exact dimensions of the physical display with a ruler
  // and avoid this auto adjustment, but for a simple desktop display,
  // this is a reasonable approximation to handle viewports of a
  // variety of sizes and aspect ratios.  Assumes square pixels.
  if (tileAutoAdjustToResolution) {
    Vector3 tileCtr   = (topLeft + topRight + botLeft + botRight) / 4.0;
    Vector3 tileUp    = (topLeft - botLeft).unit();
    Vector3 tileRight = (topRight - topLeft).unit();
    double  tileW     = (topRight - topLeft).magnitude();
    double  tileH     = (topRight - botRight).magnitude();
    double  ratio     = (double)_gwindow->width() / (double)_gwindow->height();

    if (ratio >= 1.0) {
      double newTileW = ratio * tileH;
      topLeft  = tileCtr + 0.5*tileH*tileUp - 0.5*newTileW*tileRight;
      topRight = tileCtr + 0.5*tileH*tileUp + 0.5*newTileW*tileRight;
      botLeft  = tileCtr - 0.5*tileH*tileUp - 0.5*newTileW*tileRight;
      botRight = tileCtr - 0.5*tileH*tileUp + 0.5*newTileW*tileRight;
    }
    else {
      double newTileH = tileW / ratio;
      topLeft  = tileCtr + 0.5*newTileH*tileUp - 0.5*tileW*tileRight;
      topRight = tileCtr + 0.5*newTileH*tileUp + 0.5*tileW*tileRight;
      botLeft  = tileCtr - 0.5*newTileH*tileUp - 0.5*tileW*tileRight;
      botRight = tileCtr - 0.5*newTileH*tileUp + 0.5*tileW*tileRight;
    }

    _tile = DisplayTile(topLeft, topRight, botLeft, botRight, 
                        renderType, nearclip, farclip);
    _camera = new ProjectionVRCamera(_tile, initialHeadFrame); 
  }
  

  if (hideMouse) {
    _gwindow->incMouseHideCount();
  }

  _widgetManager = WidgetManager::create(_gwindow);
  _userInput = new UserInput(_gwindow);

  if (_isAClusterClient) {
    clientSetup();
  }
}


void
VRApp::init(GWindow *gwindow, RenderDevice *renderDevice, DisplayTile tile, 
            Array<InputDevice*> inputDevices, CoordinateFrame initialHeadFrame, 
            const std::string &clusterServerName, Log *log)
{
  _gwindow = gwindow; 
  _renderDevice = renderDevice; 
  _tile = tile;
  _inputDevices = inputDevices; 
  _log = log; 
  _isAClusterClient = (clusterServerName != "");
  _clusterServerName = clusterServerName;

  if (_log == NULL) {
    _log = new Log("log.txt");
  }

  _camera = new ProjectionVRCamera(_tile, initialHeadFrame); 
  _userInput = new UserInput(_gwindow);
  _widgetManager = WidgetManager::create(_gwindow);

  if (_isAClusterClient) {
    clientSetup();
  }
}


void
VRApp::doUserInput(Array<EventRef> &events)
{
  // Respond to events generated by input devices since the last frame was rendered

  /** Typical setup:
  for (int i=0;i<events.size();i++) {
    if (events[i]->getName() == "Wand_Tracker") {
      CoordinateFrame newFrame = events[i]->getCoordinateFrameData();
    }
    else if (events[i]->getName() == "kbd_SPACE_down") {
      cout << "Pressed the space key." << endl;
    }
    else if (events[i]->getName() == "Mouse_Pointer") {
      cout << "New mouse position = " << events[i]->get2DData() << endl;
    }
  }  
  **/
}

void
VRApp::guiProcessGEvents(Array<GEvent> &gevents, Array<VRG3D::EventRef> &newGuiEvents)
{
  Array<GEvent> returnEvents;
  _userInput->beginEvents();
  for (int i=0;i<gevents.size();i++) {
    
    if (gevents[i].type == GEventType::GUI_ACTION) {
      Array<GuiButton*> keys = _guiBtnToEventMap.getKeys();
      for (int k=0;k<keys.size();k++) {
        if (gevents[i].gui.control == keys[k]) {
          newGuiEvents.append(new VRG3D::Event(_guiBtnToEventMap[keys[k]]));
        }
      }
    }
    else if (gevents[i].type == GEventType::QUIT)
    {
      _endProgram = true;
      return;
    }
    
    if (!WidgetManager::onEvent(gevents[i], _widgetManager)) {
      returnEvents.append(gevents[i]);
    }
    // BUG: There is a bug in G3D::GuiButton that makes it consume all
    // mouse button up events once it has been pressed.  The "fix"
    // here is to always report mouse up events, even if they should
    // be consumed by a G3D Gui element.  This isn't a great fix
    // though, it could screw up some programs.
    else if (gevents[i].type == GEventType::MOUSE_BUTTON_UP) {
      returnEvents.append(gevents[i]);
    }

    _userInput->processEvent(gevents[i]);
  }
  _userInput->endEvents();
  _widgetManager->onUserInput(_userInput);

  gevents = returnEvents;
}


void
VRApp::doGraphics(RenderDevice *rd)
{
  // Example of drawing a simple piece of geometry using G3D::Draw
  Draw::axes(CoordinateFrame(), rd, Color3::red(), Color3::green(), Color3::blue(), 0.25);
}



void
VRApp::run()
{
  if (_gwindow->requiresMainLoop()) {
    _gwindow->pushLoopBody(gWindowLoopCallback, this);
    _gwindow->runMainLoop();
  }
  else {
    while (!_endProgram) {
      oneFrame();
    }
  }
}


void
VRApp::oneFrame()
{
  _frameCounter++;

  // Clients should request new events from the server
  if (_isAClusterClient) {
    clientRequestEvents();
    vrg3dSleepSecs(CLIENT_SLEEP);
  }

  // Collect new events from input devices
  Array<EventRef> events;
  for (int i=0;i<_inputDevices.size();i++) {
    _inputDevices[i]->pollForInput(events);
  }

  // Poll graphics window for mouse and kbd events
  Array<GEvent> gEvents;
  pollWindowForGEvents(_renderDevice, gEvents);
  // Respond to any G3D::Gui* elements registered with our
  // _widgetManager and convert registered GuiButton events to
  // VRG3D::Events.
  guiProcessGEvents(gEvents, events);
  // Convert remaining unprocessed GEvents to VRG3D::Events.
  appendGEventsToEvents(_renderDevice, _camera->tile, events, gEvents, _curMousePos);


  // Clients should check for new events from the server
  if (_isAClusterClient) {
    clientReceiveEvents(events);
  }

  // Respond here to VRG3D system-level events (head tracking, update synced clock)
  for (int i=0;i<events.size();i++) {
    if (events[i]->getName() == "Head_Tracker") {
      //cout << frameCounter << " " << events[i]->getCoordinateFrameData().translation << endl;
      _camera->updateHeadFrame(events[i]->getCoordinateFrameData());
    }
    else if (events[i]->getName() == "SynchedTime") {
      if (SynchedSystem::getTimeUpdateMethod() == SynchedSystem::USE_LOCAL_SYSTEM_TIME) {
        // We must be running in a cluster, and this is the very first
        // SynchedTime event so use it as the program start time.
        SynchedSystem::setProgramUpdatesTime(events[i]->get1DData());
      }
      SynchedSystem::updateLocalTime(events[i]->get1DData());
    }
    else if (events[i]->getName() == "Shutdown") {
      _endProgram = true;
    }
  }

  // Subclasses should respond to events that interest them by filling in this method
  doUserInput(events);  

  // Render Graphics 

  _renderDevice->setColorClearValue(_clearColor);

  if (_frameCounter == 1) {
    _renderDevice->beginFrame();
  }
    
  // Subclasses should reimplement the doGraphics routine to draw
  // their own graphics, but should NOT call RenderDevice::clear(),
  // beginFrame(), or endFrame().  In cases where additional control
  // of the gfx loop is needed, such as the ability to apply a custom
  // camera/projection matrix, the oneFrameGraphics() routine may be
  // reimplemented.
  oneFrameGraphics();

  // Clients should tell the server they are ready and wait for an an ok to
  // swap signal.
  if (_isAClusterClient) {
    glFlush();
    clientRequestAndWaitForOkToSwap();
  }

  _renderDevice->endFrame();
  // In G3D ver 6.08+ swap buffers is actually called in beginFrame(), so we want
  // this very close to the cluster swaplock.
  _renderDevice->beginFrame();
}



// The graphics rendering portion of the loop setup inside the oneFrame method.
void
VRApp::oneFrameGraphics()
{
  if (_camera->tile.renderType == DisplayTile::TILE_STEREO) {
    glDrawBuffer(GL_BACK_LEFT);
    _renderDevice->clear(true,true,true);
    _camera->applyProjection(_renderDevice, ProjectionVRCamera::LeftEye);
    doGraphics(_renderDevice);
    
    glDrawBuffer(GL_BACK_RIGHT);
    _renderDevice->clear(true,true,true);
    _camera->applyProjection(_renderDevice, ProjectionVRCamera::RightEye);
    doGraphics(_renderDevice);
  }

  else if (_camera->tile.renderType == DisplayTile::TILE_CHECKERBOARD_STEREO) {
    glDrawBuffer(GL_BACK);
    _renderDevice->clear(true,true,true);

    // Draw checkerboard into stencil buffer
    _renderDevice->pushState();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    checkerboard_stencil(_renderDevice->width(), _renderDevice->height());
    glPopAttrib();
    _renderDevice->popState();
    _renderDevice->clear(true,true,false);

    glStencilFunc(GL_NOTEQUAL, 1, 1);
    _camera->applyProjection(_renderDevice, ProjectionVRCamera::LeftEye);
    doGraphics(_renderDevice);

    glStencilFunc(GL_EQUAL, 1, 1);
    _camera->applyProjection(_renderDevice, ProjectionVRCamera::RightEye);
    doGraphics(_renderDevice);
  }

  else if (_camera->tile.renderType == DisplayTile::TILE_MONO_LEFT) {
    glDrawBuffer(GL_BACK);
    _renderDevice->clear(true,true,true);
    _camera->applyProjection(_renderDevice, ProjectionVRCamera::LeftEye);
    doGraphics(_renderDevice);
  }

  else if (_camera->tile.renderType == DisplayTile::TILE_MONO_RIGHT) {
    glDrawBuffer(GL_BACK);
    _renderDevice->clear(true,true,true);
    _camera->applyProjection(_renderDevice, ProjectionVRCamera::RightEye);
    doGraphics(_renderDevice);
  }

  else {
    glDrawBuffer(GL_BACK);
    _renderDevice->clear(true,true,true);
    _camera->applyProjection(_renderDevice, ProjectionVRCamera::Cyclops);
    doGraphics(_renderDevice);
  }
}


void
VRApp::clientSetup()
{
#ifdef USE_CLUSTER
  _clusterSynchIt = new SynchItClient();
  _clusterSynchIt->Init(VRG3D_DEFAULT_SYNCHIT_PORT, clusterServerName.c_str());
  _clusterSynchItBuf = new char[_clusterSynchIt->max_buffer_size()];
#else
  // instance() call creates a new NetworkDevice and calls init() on it
  _networkDevice = NetworkDevice::instance();
  NetAddress serverAddress(_clusterServerName, CLUSTER_NET_PORT);
  cout << "Client: Trying to connect to " << serverAddress.toString() << endl;
  // Looks like it should set TCP_NODELAY
  _conduitToServer = ReliableConduit::create(serverAddress);

  // If didn't connect to server right away, keep trying for a maximum of 10 sec.
  double timeout = System::time() + 11.0;
  while (((_conduitToServer.isNull()) || (!_conduitToServer->ok())) && 
         (System::time() < timeout)) {
    vrg3dSleepSecs(1.0);
    _conduitToServer = NULL;
    cout << "Client: trying again.." << endl;
    _conduitToServer = ReliableConduit::create(serverAddress);
  }

  if (_conduitToServer.isNull() || !_conduitToServer->ok()) {
    cout << "Client: Unable to connect to server." << endl;
    _endProgram = true;
  }
  else {
    cout << "Client: Connected ok." << endl;
  }
#endif
}

void
VRApp::clientRequestEvents()
{
#ifdef USE_CLUSTER
  // no request for cluster-sync
#else
  clientCheckConnection();
  EventNetMsg requestMsg(new Event("ReqEvents"));
  _conduitToServer->send(requestMsg.type(), requestMsg);
#endif
}

void
VRApp::clientCheckConnection()
{
  if (!_conduitToServer->ok()) {
    cerr << "Connection to server lost." << endl;
    exit(0);
  }
}

void
VRApp::clientReceiveEvents(Array<EventRef> &events)
{
#ifdef USE_CLUSTER
  int num = _clusterSynchIt->GetNewData(_clusterSynchItBuf);
  if (num > 0) {
    // Convert to BinaryInput to easily deserialize the data
    BinaryInput bin((const uint8*)_clusterSynchItBuf, num, G3D_LITTLE_ENDIAN, false, false);

    EventBufferNetMsg msg;
    msg.deserialize(bin);
    events.append(msg.eventBuffer);
  }
#else
  while (!_conduitToServer->messageWaiting()) {
    clientCheckConnection();
  }
  if (_conduitToServer->waitingMessageType() != EVENTBUFFERNETMSG_TYPE) {
    cerr << "Client: Expected event buffer msg, got something else." << endl;
    _endProgram = true;
  }
  else {
    EventBufferNetMsg msg;
    clientCheckConnection();
    _conduitToServer->receive(msg);
    events.append(msg.eventBuffer);
  }
#endif
}

void
VRApp::clientRequestAndWaitForOkToSwap()
{
#ifdef USE_CLUSTER
  // currently no software swap lock
#else
  clientCheckConnection();

  // send a request to swap
  EventNetMsg requestMsg(new Event("Swap"));
  _conduitToServer->send(requestMsg.type(), requestMsg);

  // wait for reply
  while (!_conduitToServer->messageWaiting()) {
    clientCheckConnection();
  }
  if (_conduitToServer->waitingMessageType() != EVENTNETMSG_TYPE) {
    cerr << "Client: Expected SwapOK msg, got something else." << endl;
    _endProgram = true;
  }
  else {
    clientCheckConnection();
    EventNetMsg replyMsg;
    _conduitToServer->receive(replyMsg);
    debugAssert(replyMsg.event->getName() == "SwapOK");
  }
#endif
}


std::string
VRApp::findVRG3DDataFile(const std::string &filename)
{
  if (fileExists(filename)) {
    return filename;
  }
  
  std::string filename2 = "./config/" + filename;
  if (fileExists(filename2)) {
    return filename2;
  }
  
  std::string filename3 = "../config/" + filename;
  if (fileExists(filename3)) {
    return filename3;
  }
  
  cerr << "Could not find data file as either:" << endl;
  cerr << "1. " << filename << endl;
  cerr << "2. " << filename2 << endl; 
  cerr << "3. " << filename3 << endl; 
  
  return "";
}



// ----------------------------------------------------------------------------

void
VRApp::setupInputDevicesFromConfigFile(const std::string &filename, Log *log, Array<InputDevice*> &devices)
{
  ConfigMapRef map = new ConfigMap(filename, log);

  Array<std::string> devnames = splitStringIntoArray(map->get("InputDevices",""));

  for (int i=0;i<devnames.size();i++) {
    std::string devname = devnames[i];
    std::string type = map->get(devname + "_Type","");

    InputDevice* newDevice = NULL;

    if (type == "dummy") {
    }

    else if (type == "ICubeX") {
      std::string activeChannels = map->get(devname + "_ActiveChannels",
          "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32");
      unsigned int samplingInterval = map->get(devname + "_SamplingInterval", 100);
      int midiIn  = map->get(devname + "_MidiInDeviceNum", 0);
      int midiOut = map->get(devname + "_MidiOutDeviceNum", 0);
      std::string modeStr = map->get(devname + "_Mode", "STAND_ALONE_MODE");
      ICubeXDevice::ICubeXMode mode = ICubeXDevice::STAND_ALONE_MODE;
      if (modeStr == "HOST_MODE") {
        mode = ICubeXDevice::HOST_MODE;
      }
      bool debug = map->get(devname + "_Debug", 0);
      newDevice =  new ICubeXDevice(activeChannels, samplingInterval, midiIn, midiOut, mode, debug);
    }

#ifdef USE_VRPN
    else if (type == "VRPNAnalogDevice") {
      std::string vrpnname = map->get(devname + "_VRPNAnalogDeviceName","");
      std::string events = map->get(devname + "_EventsToGenerate","");
      log->println("Creating new VRPNAnalogDevice (" + vrpnname + ")");
      newDevice = new VRPNAnalogDevice(vrpnname, splitStringIntoArray(events));
    }
    else if (type == "VRPNButtonDevice") {
      std::string vrpnname = map->get(devname + "_VRPNButtonDeviceName","");
      std::string events = map->get(devname + "_EventsToGenerate","");
      log->println("Creating new VRPNButtonDevice (" + vrpnname + ")");
      newDevice = new VRPNButtonDevice(vrpnname, splitStringIntoArray(events));
    }
    else if (type == "VRPNTrackerDevice") {
      std::string vrpnname = map->get(devname + "_VRPNTrackerDeviceName","");
      std::string eventsStr = map->get(devname + "_EventsToGenerate","");
      Array<std::string> events = splitStringIntoArray(eventsStr);
  
      double scale = map->get(devname + "_TrackerUnitsToRoomUnitsScale", 1.0);
      CoordinateFrame d2r = map->get(devname + "_DeviceToRoom", CoordinateFrame());
      d2r.rotation.orthonormalize();
  
      Array<CoordinateFrame> p2t;
      Array<CoordinateFrame> fo;
      for (int i=0;i<events.size();i++) {
        CoordinateFrame cf = map->get(events[i] + "_PropToTracker", CoordinateFrame());
        cf.rotation.orthonormalize();
        p2t.append(cf);
        CoordinateFrame cf2 = map->get(events[i] + "_FinalOffset", CoordinateFrame());
        cf2.rotation.orthonormalize();
        fo.append(cf2);
      }

      bool wait = map->get(devname + "_WaitForNewReportInPoll", false);
      bool convertLHtoRH = map->get(devname + "_ConvertLHtoRH", false);

      log->println("Creating new VRPNTrackerDevice (" + vrpnname + ")");
      newDevice = new VRPNTrackerDevice(vrpnname, events, scale, d2r, p2t, fo, wait, convertLHtoRH);
    }
#endif

#ifdef USE_TRACKD
    else if (type == "TrackD") {
      int trackerKey = map->get(devname + "_TrackerKey", 4126);
      int wandKey = map->get(devname + "_WandKey", 5126);

      std::string bEventsStr = map->get(devname + "_ButtonEventsToGenerate","");
      Array<std::string> bEvents = splitStringIntoArray(bEventsStr);

      std::string vEventsStr = map->get(devname + "_ValuatorEventsToGenerate","");
      Array<std::string> vEvents = splitStringIntoArray(vEventsStr);

      std::string eventsStr = map->get(devname + "_EventsToGenerate","");
      Array<std::string> events = splitStringIntoArray(eventsStr);
  
      double scale = map->get(devname + "_TrackerUnitsToRoomUnitsScale", 1.0);
      CoordinateFrame d2r = map->get(devname + "_DeviceToRoom", CoordinateFrame());
      d2r.rotation.orthonormalize();
  
      Array<CoordinateFrame> p2t;
      Array<CoordinateFrame> fo;
      for (int i=0;i<events.size();i++) {
        CoordinateFrame cf = map->get(events[i] + "_PropToTracker", CoordinateFrame());
        cf.rotation.orthonormalize();
        p2t.append(cf);
        CoordinateFrame cf2 = map->get(events[i] + "_FinalOffset", CoordinateFrame());
        cf2.rotation.orthonormalize();
        fo.append(cf2);
      }

      cerr << "Creating new trackd client.." << endl;
      newDevice = new TrackDClient(trackerKey, wandKey, events, scale, d2r, p2t, fo, bEvents, vEvents);
      cerr << "done." << endl;
    }
#endif

#ifdef USE_ISENSE

    else if (type == "ISense") {

      Array< Array<std::string> > bevents;
      for (int i=0;i<ISD_MAX_STATIONS;i++) {
        std::string bEventsStr = map->get(devname + "_ButtonEventsToGenerate" + intToString(i),"");
        Array<std::string> bea = splitStringIntoArray(bEventsStr);
        bevents.append(bea);
      }
 
      std::string eventsStr = map->get(devname + "_EventsToGenerate","");
      Array<std::string> events = splitStringIntoArray(eventsStr);
  
      double scale = map->get(devname + "_TrackerUnitsToRoomUnitsScale", 1.0);
      CoordinateFrame d2r = map->get(devname + "_DeviceToRoom", CoordinateFrame());
      d2r.rotation.orthonormalize();
  
      Array<CoordinateFrame> p2t;
      Array<CoordinateFrame> fo;
      for (int i=0;i<events.size();i++) {
        CoordinateFrame cf = map->get(events[i] + "_PropToTracker", CoordinateFrame());
        cf.rotation.orthonormalize();
        p2t.append(cf);
        CoordinateFrame cf2 = map->get(events[i] + "_FinalOffset", CoordinateFrame());
        cf2.rotation.orthonormalize();
        fo.append(cf2);
      }

      newDevice = new ISenseDirect(events, scale, d2r, p2t, fo, bevents);
    }

#endif

#ifdef USE_SPACENAV
    else if (type == "SpaceNavDevice") {
      // If we compiled with SpaceNav support, then load it automatically.
      log->println("Creating new SpaceNavDevice");
      newDevice = new SpaceNavDevice();
    }
#endif
    
    else {
      alwaysAssertM(false, format("Unknown device type: %s", type.c_str()));
    }

    debugAssert(newDevice != NULL);
    devices.append(newDevice);
  }
}



void 
VRApp::pollWindowForGEvents(RenderDevice *rd, Array<GEvent> &gEvents)
{
  GWindow *gwindow = rd->window();
  if (!gwindow) {
    return;
  }
  
  GEvent event;
  while (gwindow->pollEvent(event)) {
    gEvents.append(event);
  }
}


/// Converts GEvents to VRG3D::Events and appends them to the events array
void
VRApp::appendGEventsToEvents(RenderDevice *rd, DisplayTile &tile, Array<EventRef> &events, 
                             Array<GEvent> &gEvents, Vector2 &mousePos)
{
  bool gotMouseMotion = false;
  for (int i=0;i<gEvents.size();i++) {
    GEvent event = gEvents[i];
    switch(event.type) {
    case GEventType::QUIT:
      exit(0);
      break;

    case GEventType::VIDEO_RESIZE:
      {
        double oldw = rd->width();
        double oldh = rd->height();  

        rd->notifyResize(event.resize.w, event.resize.h);
        Rect2D full = Rect2D::xywh(0, 0, rd->width(), rd->height());
        rd->setViewport(full);

        double w = rd->width();
        double h = rd->height();  

        /** recompute room coordinates of the tile display
	          if width changes more than height, then width controls what appears
	          to be zoom and height is adjusted so that what was in the center
	          of the screen before remains in the center of the screen.  if
	          height changes more, then we do the opposite. 
        */
	
        // scale factors
        double scalew = w / oldw;
        double scaleh = h / oldh;
          
        if (fabs(scalew-1.0) >= fabs(scaleh-1.0)) {
          double newh = scalew*oldh;
          double frach = h / newh;
          double newy = tile.topLeft[1] + frach*(tile.botLeft[1] - tile.topLeft[1]);
          double diff_y_div2 = (newy-tile.botLeft[1]) / 2.0;
          tile.topLeft[1] -= diff_y_div2;
          tile.topRight[1] -= diff_y_div2;
          tile.botLeft[1] = newy - diff_y_div2;
          tile.botRight[1] = newy - diff_y_div2;
        }
        else {
          double neww = scaleh*oldw;
          double fracw = w / neww;
          double newx = tile.topLeft[0] + fracw*(tile.topRight[0] - tile.topLeft[0]);
          double diff_x_div2 = (newx-tile.topRight[0]) / 2.0;
          tile.topLeft[0] -= diff_x_div2;
          tile.topRight[0] = newx - diff_x_div2;
          tile.botLeft[0] -= diff_x_div2;
          tile.botRight[0] = newx - diff_x_div2;	  
        }

        // recompute room2tile matrix
        Vector3 center = (tile.topLeft + tile.topRight + tile.botLeft + tile.botRight) / 4.0;
        Vector3 x = (tile.topRight - tile.topLeft).unit();
        Vector3 y = (tile.topLeft - tile.botLeft).unit();
        Vector3 z = x.cross(y).unit();
        Matrix3 rot(x[0],y[0],z[0],x[1],y[1],z[1],x[2],y[2],z[2]);
        CoordinateFrame tile2room(rot,center);
        tile.room2tile  = tile2room.inverse();
      }
      break;
      
    case GEventType::KEY_DOWN: 
      {
        std::string keyname = getKeyName(event.key.keysym.sym, event.key.keysym.mod);
        events.append(new Event("kbd_" + keyname + "_down"));
      }              
      break;
      
    case GEventType::KEY_UP:
      {
        std::string keyname = getKeyName(event.key.keysym.sym, event.key.keysym.mod);
        events.append(new Event("kbd_" + keyname + "_up"));
      }
      break;
      
    case GEventType::MOUSE_MOTION:
      {
      // Mouse Coordiante System Convention
       /**                (1,1)
         +-------------+
         |             |
         |    (0,0)    |
         |             |
         +-------------+
      (-1,-1)               **/

      gotMouseMotion = true;
      double fracx = (double)event.motion.x / (double)rd->width();
      double fracy = (double)event.motion.y / (double)rd->height();
      Vector2 vmouse((fracx * 2.0) - 1.0, -((fracy * 2.0) - 1.0));
      events.append(new Event("Mouse_Pointer",vmouse));
      mousePos = vmouse;
      }
      break;
      
    case GEventType::MOUSE_BUTTON_DOWN:
      {
      switch (event.button.button) {
      case 0: //SDL_BUTTON_LEFT:
        events.append(new Event("Mouse_Left_Btn_down", mousePos));
        break;
      case 1: //SDL_BUTTON_MIDDLE:
        events.append(new Event("Mouse_Middle_Btn_down", mousePos));
        break;
      case 2: //SDL_BUTTON_RIGHT:
        events.append(new Event("Mouse_Right_Btn_down", mousePos));
        break;
      case 3:
        events.append(new Event("Mouse_WheelUp_Btn_down", mousePos));
        break;
      case 4:
        events.append(new Event("Mouse_WheelDown_Btn_down", mousePos));
        break;
      default:
        events.append(new Event("Mouse_" + intToString(event.button.button) + "_Btn_down", mousePos));
        break;
      }
      }
      break;

    case GEventType::MOUSE_BUTTON_UP:
      {
      switch (event.button.button) {
      case 0: //SDL_BUTTON_LEFT:
        events.append(new Event("Mouse_Left_Btn_up", mousePos));
        break;
      case 1: //SDL_BUTTON_MIDDLE:
        events.append(new Event("Mouse_Middle_Btn_up", mousePos));
        break;
      case 2: //SDL_BUTTON_RIGHT:
        events.append(new Event("Mouse_Right_Btn_up", mousePos));
        break;
      case 3:
        events.append(new Event("Mouse_WheelUp_Btn_up", mousePos));
        break;
      case 4:
        events.append(new Event("Mouse_WheelDown_Btn_up", mousePos));
        break;
      default:
        events.append(new Event("Mouse_" + intToString(event.button.button) + "_Btn_up", mousePos));
        break;
      }
      }
      break;
    }
  }  

  if (!gotMouseMotion) {
    // Events may not be coming through as events for this window
    // type, try polling
    int x,y;
    G3D::uint8 btns;
    rd->window()->getRelativeMouseState(x, y, btns);
    if (Vector2(x,y) != mousePos) {
      double fracx = (double)x / (double)rd->width();
      double fracy = (double)y / (double)rd->height();
      Vector2 vmouse((fracx * 2.0) - 1.0, -((fracy * 2.0) - 1.0));
      events.append(new Event("Mouse_Pointer",vmouse));
      mousePos = vmouse;
    }
  }

}


std::string
VRApp::getKeyName(GKey::Value key, GKeyMod mod)
{
  std::string name;

  switch (key) {
  case GKey::F1:
    name =  std::string("F1");
    break;
  case GKey::F2: 
    name =  std::string("F2");
    break;
  case GKey::F3:
    name =  std::string("F3"); 
    break;
  case GKey::F4:
    name =  std::string("F4");
    break;
  case GKey::F5:
    name =  std::string("F5");
    break;
  case GKey::F6:
    name =  std::string("F6");
    break;
  case GKey::F7:
    name =  std::string("F7");
    break;
  case GKey::F8: 
    name =  std::string("F8");
    break;
  case GKey::F9:
    name =  std::string("F9"); 
    break;
  case GKey::F10:
    name =  std::string("F10");
    break;
  case GKey::F11:
    name =  std::string("F11");
    break;
  case GKey::F12:
    name =  std::string("F12");
    break;
  case GKey::LEFT:
    name =  std::string("LEFT");
    break;
  case GKey::RIGHT: 
    name =  std::string("RIGHT");
    break;
  case GKey::UP:
    name =  std::string("UP"); 
    break;
  case GKey::DOWN:
    name =  std::string("DOWN");
    break;
  case GKey::PAGEUP:
    name =  std::string("PAGEUP");
    break;
  case GKey::PAGEDOWN:
    name =  std::string("PAGEDOWN");
    break;
  case GKey::HOME:
    name =  std::string("HOME");
    break;
  case GKey::END: 
    name =  std::string("END");
    break;
  case GKey::INSERT:
    name =  std::string("INSERT"); 
    break;
  case GKey::BACKSPACE:
    name =  std::string("BACKSPACE"); 
    break;
  case GKey::TAB:
    name =  std::string("TAB"); 
    break;
  case GKey::RETURN:
    name =  std::string("ENTER"); 
    break;
  case GKey::ESCAPE:
    name =  std::string("ESC"); 
    break;
  case GKey::SPACE:
    name =  std::string("SPACE"); 
    break;
  case 48:
    name =  std::string("0"); 
    break;
  case 49:
    name =  std::string("1"); 
    break;
  case 50:
    name =  std::string("2"); 
    break;
  case 51:
    name =  std::string("3"); 
    break;
  case 52:
    name =  std::string("4"); 
    break;
  case 53:
    name =  std::string("5"); 
    break;
  case 54:
    name =  std::string("6"); 
    break;
  case 55:
    name =  std::string("7"); 
    break;
  case 56:
    name =  std::string("8"); 
    break;
  case 57:
    name =  std::string("9"); 
    break;
  case 97:
    name =  std::string("A"); 
    break;
  case 98:
    name =  std::string("B"); 
    break;
  case 99:
    name =  std::string("C"); 
    break;
  case 100:
    name =  std::string("D"); 
    break;
  case 101:
    name =  std::string("E"); 
    break;
  case 102:
    name =  std::string("F"); 
    break;
  case 103:
    name =  std::string("G"); 
    break;
  case 104:
    name =  std::string("H"); 
    break;
  case 105:
    name =  std::string("I"); 
    break;
  case 106:
    name =  std::string("J"); 
    break;
  case 107:
    name =  std::string("K"); 
    break;
  case 108:
    name =  std::string("L"); 
    break;
  case 109:
    name =  std::string("M"); 
    break;
  case 110:
    name =  std::string("N"); 
    break;
  case 111:
    name =  std::string("O"); 
    break;
  case 112:
    name =  std::string("P"); 
    break;
  case 113:
    name =  std::string("Q"); 
    break;
  case 114:
    name =  std::string("R"); 
    break;
  case 115:
    name =  std::string("S"); 
    break;
  case 116:
    name =  std::string("T"); 
    break;
  case 117:
    name =  std::string("U"); 
    break;
  case 118:
    name =  std::string("V"); 
    break;
  case 119:
    name =  std::string("W"); 
    break;
  case 120:
    name =  std::string("X"); 
    break;
  case 121:
    name =  std::string("Y"); 
    break;
  case 122:
    name =  std::string("Z"); 
    break;
  case GKey::RSHIFT:
  case GKey::LSHIFT:
    name =  std::string("SHIFT");
    return name;
    break;
  case GKey::RCTRL:
  case GKey::LCTRL:
    name =  std::string("CTRL");
    return name;
    break;
  case GKey::RALT:
  case GKey::LALT:
    name =  std::string("ALT");
    return name;
    break;
  case GKey::PERIOD:
    name = ".";
    break;
  case GKey::COMMA:
    name = ",";
    break;
  case GKey::LEFTBRACKET:
    name = "[";
    break;
  case GKey::RIGHTBRACKET:
    name = "]";
    break;
  default:
    cerr << "Unknown keypress: " << (int)key << endl;
    name =  std::string("UNKNOWN");
    break;
  }
  
  if (mod & GKEYMOD_SHIFT)
    name = "SHIFT_" + name;
  if (mod & GKEYMOD_ALT)
    name = "ALT_" + name;
  if (mod & GKEYMOD_CTRL)
    name = "CTRL_" + name;
  if (mod & GKEYMOD_META)
    name = "META_" + name;

  return name;
}



void vrg3dSleepMsecs(double msecs)
{
  // Routine grabbed from VRPN's vrpn_SleepMsecs routine
#ifdef WIN32
  Sleep((DWORD)msecs);
#else
  timeval timeout;
  // Convert milliseconds to seconds
  timeout.tv_sec = (int)(msecs / 1000);
  // Subtract of whole number of seconds
  msecs -= timeout.tv_sec * 1000;
  // Convert remaining milliseconds to microsec
  timeout.tv_usec = (int)(msecs * 1000);
  // A select() with NULL file descriptors acts like a microsecond timer.
  select(0, 0, 0, 0, &timeout);
#endif
}

GuiButton* 
VRApp::addGuiButtonAndTrapEvent(GuiPane *pane,
                                const std::string &eventToGenerate,
                                const GuiCaption &text, 
                                GuiTheme::ButtonStyle style)
{
  GuiButton *b = pane->addButton(text, style);
  _guiBtnToEventMap.set(b,eventToGenerate);
  return b;
}
  





void
vrg3dSleepSecs(double secs)
{
  vrg3dSleepMsecs(secs*1000.0);
}



} // end namespace

