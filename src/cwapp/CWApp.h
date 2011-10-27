#ifndef _CWAPP_H_
#define _CWAPP_H_

#include "main.h"
#include "UserInputVRApp.h"
#include "TextureManager.h"
#include "FontMgr.H"

class BaseSoundClient;

#define SOUNDCLIENT the_app.sound_client

class CWApp : public UserInputVRApp
{
public:
  CWApp() : is_cave(false), left_down(false), mouse_down(false), randomized(false), reload_exit(false), draw_bbox(false),
            middle_down(false), right_down(false), x_delta(false), y_delta(false), is_draw_frp(false) {}

  void init(const string& filename, const string& vrSetup, const string& server);
  void init_custom_desktop (const string& vrSetup);
  void init_custom_cave (const string& vrSetup, const string& server_host);
  
  void close();

  void doUserInput(Array<VRG3D::EventRef>& events);
  void doGraphics(RenderDevice *rd);

  void draw_laser(RenderDevice *rd);

  void draw_framerate(RenderDevice *rd);

  void update_far_clip(double far_clip);

  void set_background_color(Color3 color);

  Vector3 screen_point_to_camera_plane(const Vector2 &v);

  Vector3         get_cam_pos() const   { return cam_frame.translation; }
  CoordinateFrame get_cam_frame() const { return cam_frame; }

  void set_cam_frame (const CoordinateFrame& frame) { cam_frame = frame; }

  CoordinateFrame get_actual_frame (void) const { return actual; }

  CoordinateFrame get_world_head_frame (void) const { return head_actual; }

  bool is_caved_in() const { return is_cave; }
  
  bool is_draw_bbox() const { return draw_bbox; }
  
  void begin_interp(int type, const ActionRef& action);
  
  void interp_color(const Color4& end_color, float elapsed);

public:
  CoordinateFrame cave_frame;
  Vector3 cave_vel;

  TextureManager tex_mgr;

  FontMgr font_mgr;

  VARAreaRef var_static;

  BaseSoundClient *sound_client;

  bool reload_exit;
  float x_delta, y_delta;

protected:
  CoordinateFrame wand_frame;
  CoordinateFrame combined, actual, head_actual;
  CoordinateFrame cam_full;

  Color4 background_color;
  Color4 start_color;

  Vector3 seed_vector;
  
  AABox cave_box;
  bool is_cave, is_draw_box, draw_bbox, is_draw_frp;
  bool left_down, mouse_down;
  bool right_down, middle_down;
  bool randomized;

  string filename;

  float rps, mps;
  double last_time;
};

extern CWApp the_app;

#endif
