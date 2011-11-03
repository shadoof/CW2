#include "main.h"

#ifdef USE_GLUT
#  include "GlutWindow.h"
#endif

#ifdef USE_SDL
#if __APPLE__
#  include <SDL/SDL.h>
#else
#  include "SDL.h"
#endif
#endif

CWApp the_app;
CoordinateFrame wand_frame_orig;

using VRG3D::DisplayTile;
using VRG3D::InputDevice;
using VRG3D::SynchedSystem;

//===========================================
void CWApp::init(const string& file, const string& vrSetup, const string& server)
{
  filename = file;

  if (beginsWith(vrSetup, "desktop")) {
    init_custom_desktop(vrSetup);
    is_cave = false;
    is_draw_box = true;
  } else {
    is_cave = true;
    is_draw_box = false;
    init_custom_cave(vrSetup, server);
  }

  cave_box.set(Vector3(-4, -4, -4), Vector3(4, 4, 4));

  cave_vel = Vector3::zero();

  //cave_frame.translation = Vector3(0, 0, -8);
  var_static  = VARArea::create(1024 * 16 * 512, VARArea::WRITE_ONCE);

  UserInputVRApp::init_userinput();
  rps = 5;
  mps = 5;
#ifdef G3D_OSX
  rps *= 2;
#endif

  set_fp_manipulator(rps, mps);
  
#ifdef G3D_LINUX
  if (vrSetup == "front" || vrSetup == "desktopsound" || vrSetup == "cavefront-winserv") {
    sound_client = new RemoteSoundClient("cave9.ccv.brown.edu", SOUND_SERVER_PORT);
    if (!sound_client->is_succeeded()) {
      sound_client = new LocalSoundClient();
    }
    if (!sound_client->is_succeeded()) {
      sound_client = new NoSoundClient();
    }
  } else {
    sound_client = new NoSoundClient();
  }
#else
  //sound_client = new RemoteSoundClient("192.168.1.108", SOUND_SERVER_PORT);
  sound_client = new LocalSoundClient();

#endif
}

//===========================================
void CWApp::init_custom_cave (const string& vrSetup, const string& server_host)
{
  VRApp::init(vrSetup);

  //DisplayTile tile;

  //if (beginsWith(display, "front")) {
	 // tile = DisplayTile::defaultDesktopTile();
  //} else if (beginsWith(display, "left")) {
  //  tile = DisplayTile::defaultDesktopTile();
  //} else if (beginsWith(display, "right")) {
  //  tile = DisplayTile::defaultDesktopTile();
  //} else if (beginsWith(display, "floor")) {
  //  tile = DisplayTile::defaultDesktopTile();
  //}

  //GWindow::Settings settings;
  //settings.x = 0;
  //settings.y = 0;
  //settings.center = true;
  //settings.width  = 1024;
  //settings.height = 768;
  //settings.stereo = true;

  //GlutWindow *win = new GlutWindow(settings);

  //win->setMouseVisible(false);

  //RenderDevice *rd = new RenderDevice();
  //rd->init(win, NULL);
  //rd->resetState();

  //Array<InputDevice*> empty_devs;
  //CoordinateFrame start_frame;
  //start_frame.translation = Vector3(0, 0, 1);

  //VRApp::init(win, rd, tile, empty_devs, start_frame, server_host, NULL);
}

//===========================================
void CWApp::init_custom_desktop (const std::string& vrSetup)
{
  //use the config vrSetup for anything other than "desktopdefault"
  if (vrSetup != "desktopdefault")
  {
    VRApp::init(vrSetup);
    return;
  }

 GWindow::Settings settings;
 settings.x = 0;
 settings.y = 0;
 settings.center = true;
 settings.width  = 800;
 settings.height = 700;

 bool full_screen = false;

 if (full_screen) {
   settings.fullScreen = true;
   settings.framed = false;
 } else {
   settings.fullScreen = false;
   settings.framed = true;
 }

 GWindow* win = NULL;

#ifdef G3D_WIN32
 if (full_screen) {
   settings.width  = GetSystemMetrics(SM_CXSCREEN);// * 3/4;
   settings.height = GetSystemMetrics(SM_CYSCREEN);// * 3/4;
   settings.framed = false;
   settings.fullScreen = false;
 }
#else
 if (full_screen) {
   settings.width = 1280;
   settings.height = 1024;
 }
#endif

#if defined(USE_GLUT)
 win = new GlutWindow(settings);
#else
 win = OSWindow::create(settings);
#endif

 //7PORT
 win->setMouseHideCount(0);

 RenderDevice *rd = new RenderDevice();
 rd->init(win, NULL);
 rd->resetState();

 Array<InputDevice*> empty_devs;
 CoordinateFrame start_frame;
 start_frame.translation = Vector3::unitZ();

 cam_frame.translation = Vector3::unitZ();

 VRApp::init(win, rd, DisplayTile::defaultDesktopTile(), empty_devs, start_frame, "", NULL);
}

//===========================================
void CWApp::close()
{
  font_mgr.close();
}

//===========================================
void CWApp::doUserInput(Array<VRG3D::EventRef>& events)
{
  UserInputVRApp::doUserInput(events);

  double current_time = SynchedSystem::getLocalTime();
  double delta = current_time - last_time;

  if (!randomized) {
    ::srand(current_time);
    randomized = true;
  }

  bool last_left_down = left_down;
  bool last_middle_down = middle_down;

  //bool cam_updated = false;

  //float x_acc = 0, y_acc = 0;

  for (int i = 0; i < events.length(); i++) {
    string name = events[i]->getName();

    if (name == "Mouse_Pointer") {
      continue;
    }

//    cout << name << endl;

    if (name == "Mouse_Left_Btn_down") {
      mouse_down = left_down = true;
    } else if (name == "Mouse_Left_Btn_up") {
      mouse_down = left_down = false;
    } else if (name == "Mouse_Middle_Btn_down") {
      mouse_down = middle_down = true;
    } else if (name == "Mouse_Middle_Btn_up") {
      mouse_down = middle_down = false;
    }

      else if (G3D::beginsWith(name, "Wand_")){
      //cout << events[i]->getCoordinateFrameData().toXML() << endl;
      if (G3D::endsWith(name, "_Tracker")) {
      	wand_frame_orig = events[i]->getCoordinateFrameData();
        wand_frame = cam_frame * wand_frame_orig;
      } else if (G3D::endsWith(name, "Y")) {
        //y_acc = events[i]->get1DData() * .025;
      	//cam_frame.translation += wand_frame.lookVector() * events[i]->get1DData() * -.5;
		y_delta = events[i]->get1DData();
        //Story::the_story->do_collision_move(cam_frame, wand_frame.lookVector() * events[i]->get1DData() * .05);
      } else if (G3D::endsWith(name, "X")) {
        //x_acc = events[i]->get1DData() * .025;
      	//cam_frame.translation += wand_frame.rightVector() * events[i]->get1DData() * -.5;
		x_delta = events[i]->get1DData();
        //Story::the_story->do_collision_move(cam_frame, wand_frame.rightVector() * events[i]->get1DData() * .05);
	      //cout << "X Data " << events[i]->get1DData() << endl;
      } else if (name == "Wand_Left_Btn_down") {
        left_down = true;
      } else if (name == "Wand_Left_Btn_up") {
        left_down = false;
      } else if (name == "Wand_Right_Btn_down") {
        right_down = true;
      } else if (name == "Wand_Right_Btn_up") {
        right_down = false;
      } else if (name == "Wand_Middle_Btn_down") {
        middle_down = true;
      } else if (name == "Wand_Middle_Btn_up") {
        middle_down = false;
      }
    } else if (name == "kbd_B_down") {
      is_draw_box = !is_draw_box;
    } else if (name == "kbd_CTRL_B_down") {
      draw_bbox = !draw_bbox;
    } else if (name == "kbd_N_down") {
      if (Story::the_story.notNull()) {
        Story::the_story->draw_events = !Story::the_story->draw_events;
      }
    } else if (name == "kbd_L_down") {
      if (Story::the_story.notNull()) {
        Story::the_story->draw_lights = !Story::the_story->draw_lights;
      }
    } else if (name == "kbd_[_down") {
      rps /= 1.20;
      cam_control->setTurnRate(rps);
    } else if (name == "kbd_]_down") {
      rps *= 1.20;
      cam_control->setTurnRate(rps);
    } else if (name == "kbd_F_down") {
      is_draw_frp = !is_draw_frp;
    }
  }


  CoordinateFrame last_actual = cam_frame;
  actual = cam_frame * cave_frame;
  head_actual = actual * getHeadFrame();

  
  if (mouse_down) {
    Vector3 target = screen_point_to_camera_plane(_curMousePos);
    wand_frame.translation = cam_full.translation;//getCamera()->getCameraPos();
    wand_frame.lookAt(target);
  }

  //Tracker movement
  if (Story::the_story->allow_movement) {
    /*if (!x_acc) {
      x_delta *= .75;
    } else {
      x_delta += x_acc;
    }

    if (!y_acc) {
      y_delta *= .75;
    } else {
      y_delta += y_acc;
    }*/

    if (!fuzzyEq(y_delta, 0.f) || !fuzzyEq(x_delta, 0.f)) {
      //Vector3 offset = cam_frame.lookVector() * y_delta + cam_frame.rightVector() * x_delta;
	  Vector3 offset = delta * (wand_frame.lookVector() * y_delta + (wand_frame.lookVector().cross(cam_frame.upVector())) * x_delta);
      Story::the_story->do_collision_move(head_actual.translation, cam_frame.translation, offset);
    }
  }

  if (!last_actual.fuzzyEq(cam_frame)) {
    SOUNDCLIENT->set_listener(head_actual, cave_vel);
  }

  if (left_down) {
    CoordinateFrame world_wand = cave_frame.inverse() * wand_frame;
    Story::the_story->select_new_obj_at_pointer(world_wand.translation, world_wand.lookVector());
  } else if (!left_down && last_left_down) {
    Story::the_story->activate_selected();
  }

  //Tracker rotation
  if (Story::the_story->allow_rotation) {
    if (middle_down && !last_middle_down) {
      //cam_frame = cam_frame * Matrix3::fromAxisAngle(Vector3::unitY(), G3D::pi() / 180.0);
      seed_vector = wand_frame.lookVector();
      seed_vector.y = 0;
      seed_vector.unitize();
    } else if (middle_down) {
      Vector3 curr_vec = wand_frame.lookVector();
      curr_vec.y = 0;
      curr_vec.unitize();

      float dot = curr_vec.dot(seed_vector);

      if ((dot > 0.f) && (dot < 1.f)) {
        float angle = acos(dot);

        //2d cross
        if ((curr_vec.z * seed_vector.x) > (curr_vec.x* seed_vector.z)) {
          angle = -angle;
        }

  //      cout << "dot: " << dot << "angle: " << angle << endl;

        cam_frame = cam_frame * Matrix3::fromAxisAngle(Vector3::unitY(), angle);
        seed_vector = curr_vec * Matrix3::fromAxisAngle(Vector3::unitY(), -angle);      
      }
    }
  }

  Story::the_story->process_events();
  Story::the_story->process_actions();
  Story::the_story->process_timelines();

  if (reload_exit) {
    SOUNDCLIENT->unload_all();
    cave_frame = CoordinateFrame();
    cave_vel = Vector3::zero();
    Story::the_story = new Story();
    Story::the_story->init(filename);
    SOUNDCLIENT->set_listener(cam_frame, Vector3::zero());
    reload_exit = false;
  }

  last_time = current_time;
  SOUNDCLIENT->system_update();
}

//===========================================
void CWApp::doGraphics(RenderDevice *rd)
{
  //rd->setObjectToWorldMatrix(_roomToVirtual);

  rd->pushState();

  CoordinateFrame old = rd->getCameraToWorldMatrix();
  cam_full = cam_frame * old;
  rd->setCameraToWorldMatrix(cam_full);
//  cout << cam_frame.translation << endl;

//  CoordinateFrame frame = cam_frame;
//  frame.translation -= Vector3::unitZ()
  if (is_draw_box) {
//    rd->setObjectToWorldMatrix(cam_full);
    Draw::box(cave_box, rd, Color4::clear(), Color3::gray());
  }

  Story::the_story->do_lighting(rd);

  rd->setObjectToWorldMatrix(cave_frame);
//  cam_full = rd->getCameraToWorldMatrix() * cave_frame;

//  rd->setObjectToWorldMatrix(cave_frame);


  //Draw::arrow(world_wand.translation, world_wand.lookVector(), rd, Color3::red());

//  Draw::arrow(screen_x.translation, cam_full.rightVector(), rd, Color3::green());
//  Draw::arrow(screen_x.translation, cam_full.upVector(), rd, Color3::green());

  Story::the_story->render(rd);

  rd->popState();
  
  if (is_cave && left_down) {
 //   rd->setCameraToWorldMatrix(old);
    draw_laser(rd);
  }

  if (is_draw_frp) {
    draw_framerate(rd);
  }
}

//===========================================
void CWApp::draw_laser(RenderDevice *RD)
{
  float laserWidth = 0.02f;

  RD->pushState();

  RD->setCullFace(RenderDevice::CULL_NONE);
  RD->disableLighting();
  RD->setDepthClearValue(1.0);
  RD->setDepthWrite(false);
  RD->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE);
  RD->setObjectToWorldMatrix(wand_frame_orig);
  RD->setColor(Color3(1, 0, 0));
  RD->beginPrimitive(RenderDevice::TRIANGLES /*QUADS*/);
  RD->sendVertex(Vector4(-laserWidth, 0,  0, 1));
  RD->sendVertex(Vector4(           0, 0, -1, 0));
  RD->sendVertex(Vector4( laserWidth, 0,  0, 1));

  RD->endPrimitive();
  RD->enableLighting();
  RD->popState();
}

//===========================================
void CWApp::draw_framerate(RenderDevice *rd)
{
  rd->push2D();
  CoordinateFrame frame_2d;
  frame_2d.translation = Vector3(10, 20, 0);
  frame_2d.rotation[1][1] = -1;
  frame_2d.rotation = frame_2d.rotation * .25f;
  rd->setColor(Color3::purple());
  rd->setObjectToWorldMatrix(frame_2d);
  std::string msg = format("%3d fps", iRound(rd->frameRate()));
  FontEntry entry = font_mgr.get_font("");
  //_font->draw2D(rd, msg, Vector2(25,25), 12, Color3(0.61, 0.72, 0.92));
  entry.the_font->Render(msg.c_str());
  rd->pop2D();
}

//===========================================
void CWApp::set_background_color(Color3 color)
{
	background_color = color;
	setClearColor(color);
}

//===========================================
void CWApp::update_far_clip(double far_clip)
{
  if ((far_clip <= 0) || fuzzyEq(far_clip, _tile.farClip)) {
    return;
  }
  _tile.farClip = far_clip;
  getCamera()->setDisplayTile(_tile);
}

//===========================================
Vector3 CWApp::screen_point_to_camera_plane(const Vector2 &v)
{
  double x = (v[0] / 2.0) + 0.5;
  double y = (v[1] / 2.0) + 0.5;

  DisplayTile tile = getCamera()->getTile();

  Vector3 xvec = tile.topRight - tile.topLeft;
  Vector3 yvec = tile.topRight - tile.botRight;

  float ratio = xvec.length() / yvec.length();

  Vector3 cam_bot_left = cam_full.pointToWorldSpace(tile.botLeft) + cam_full.lookVector();
  Vector3 p = cam_bot_left + (x * cam_full.rightVector() * ratio) + (y * cam_full.upVector());
  return p;
}

//===========================================
void CWApp::begin_interp(int type, const ActionRef& action)
{
  switch (type) {
    case Action::TRANS_COLOR:
      start_color = background_color;
      break;
  }
}

//===========================================
void CWApp::interp_color(const Color4& end_color, float elapsed)
{
  float inv_elapse = (1.f - elapsed);
  background_color.r = (inv_elapse * start_color.r) + (elapsed * end_color.r);
  background_color.g = (inv_elapse * start_color.g) + (elapsed * end_color.g);
  background_color.b = (inv_elapse * start_color.b) + (elapsed * end_color.b);
  setClearColor(background_color);
}

#ifdef USE_GLUT

#undef main 

#endif
 
// Sound server
//===========================================
void run_audio_server()
{
	//std::cout << "Spelunker Sound Server (OpenAL)\n\n";
  std::cout << "CaveWriting FMOD Sound Server\n" << endl;

  SoundServer server(SOUND_SERVER_PORT);
  server.connectionLoop();
	
//#ifndef __linux
//	SoundServerOpenAL *soundServer = new SoundServerOpenAL(SOUND_SERVER_PORT);
//	soundServer->connectionLoop();
//#endif
  std::cout << "Done!\n";
}

//===========================================
int main(int argc, char** argv)
{
  string tile, file, server;

  if (argc > 3) {
    server = argv[3];
  };

  if (argc > 2) {
    tile = argv[1];
    file = argv[2];
  } else if (argc > 1) {
    file = argv[1];
    if(file == "-audioserver") {
		  //Run an audio server
		  run_audio_server();
		  return 0;
	  } else {
		  tile = "desktop";
		  file = argv[1];
	  }
  } else {
    msgBox("The Story XML to load was not specified.\n"
           "Usage:\n"
           "CW <story.xml> - load story.xml on the desktop\n"
           "CW [desktopdefault] <story.xml> - load story.xml in default desktop mode\n"
           "CW configfile <story.xml> - load story.xml using custom VR setup read from config/configfile.vrsetup\n"
           "                            All settings, including server hostname for cluster, are read from the setup file\n"
           "CW -audioserver - start command line audio server\n",
           "CaveWriting Error");
    return 0;
  }

  the_app.init(file, tile, server);

  Story::the_story = new Story();
  Story::the_story->init(file);

  the_app.run();

  the_app.close();
  
  return 0;
}

