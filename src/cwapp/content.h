#ifndef _CONTENT_H_
#define _CONTENT_H_

#include "Parser.h"
//#include "FontMgr.h"
#include "actions.h"

enum
{
  IS_CLICK_STOP = 0x01,
  IS_SELECTED = 0x02,
  IS_ACTIVE = 0x04,
  IS_REMAIN_ACTIVE = 0x08,
  IS_USE_LIGHTING = 0x10,
  IS_AROUND_SELF_AXIS = 0x20,

  LINK_RESET = 0x80000000,
};

//===========================================
class Content : public CWBase
{
public:
  Content() : flags(0), scale(1.f), valid(false), link_counter(0) {}

  virtual void init(const TiXmlElement* root, const string& content_name);
  virtual void init_actions(const TiXmlElement* root);

  virtual void init(const TiXmlElement* root) = 0;

  void select(bool onoff);
  void set_link_active(bool active);

  bool is_selected() const    { return (flags & IS_SELECTED) ? true : false; }
  bool is_active_link() const { return ((flags & IS_ACTIVE) && actions.size()) ? true : false; }
  bool is_click_stop() const  { return (is_active_link() || (flags & IS_CLICK_STOP)); }
  bool use_lighting() const   { return (flags & IS_USE_LIGHTING) ? true : false; }

  float get_alpha() const     { return color.a; }
  float get_scale() const     { return scale; }
  Color4 get_color() const    { return color; }

  AABox get_bbox() const { return bounding_box; }
  void update_bbox();

  void play_sound();

  virtual void activate();

  void begin_interp(int type, const ActionRef& action_inst);

  virtual void interp_alpha(const float& end_alpha, float elapsed);
  virtual void interp_frame(const CoordinateFrame& end_frame, float elapsed);
  virtual void interp_color(const Color4& end_color, float elapsed);
  virtual void interp_scale(const float& end_scale, float elapsed);

  virtual void render(RenderDevice *RD);
          void render_direct(RenderDevice *RD);
  virtual void render_derived(RenderDevice *RD) = 0;

  const CoordinateFrame& get_end_frame() const { return end_frame; }

  virtual LightContent* get_light() { return NULL; }

  ShaderRef get_shader() const { return shader; }

  CoordinateFrame get_curr_frame() const { return placement.get_frame(); }
  
  
  string get_content() const { return content_type; }


protected:
  AABox local_bbox;

  AABox bounding_box;

  Array<ActionRef> actions;
  Array<unsigned int> click_nums;

  Color4 active_color, selected_color, color;
  float scale;
  Placement placement;

  SoundRef sound;
  //Vector3 move_vel;

  CoordinateFrame start_frame, end_frame;
  Color4 start_color;
  float start_alpha;
  float start_scale;

  string content_type;

  ShaderRef shader;

  int link_counter;

protected:
  bool valid;

private:
  int flags;

};

inline void Content::select(bool onoff)
{
  if (onoff) {
    flags |= IS_SELECTED;
  } else {
    flags &= ~IS_SELECTED;
  }
}

inline void Content::set_link_active(bool onoff)
{
  if (onoff) {
    flags |= IS_ACTIVE;
  } else {
    flags &= ~IS_ACTIVE;
  }
}

#endif
