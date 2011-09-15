#ifndef _EVENTTRIGGER_H_
#define _EVENTTRIGGER_H_

#include "Parser.h"

typedef ReferenceCountedPointer<class EventTrigger> EventTriggerRef;

//===========================================
class EventTrigger : public CWBase
{
public:

  enum TriggerTypes
  {
    EVENT_HEAD_TRACK,
  };

  enum Flags
  {
    IS_ACTIVE = 0x01,
    IS_COUNTING = 0x02,
    IS_STAY_ACTIVE = 0x04,    
  };

  EventTrigger(void) : flags(0) {}
  virtual ~EventTrigger(void)  {}

  virtual void init(const TiXmlElement* root);
  virtual void init_tracker(const TiXmlElement* root) = 0;

  virtual void process();
  virtual bool eval() = 0;

  virtual void activate();
          void init_actions(const TiXmlElement* root);

  virtual void render(RenderDevice *rd) = 0;

   inline void set_enabled(bool onoff)
   {
     if (onoff) {
       flags |= IS_ACTIVE;
     } else {
       flags &= ~IS_ACTIVE;
     }
   }

  static EventTriggerRef create_new(const TiXmlElement* root);


protected:
  Array<ActionRef> actions;

  int flags;
  float start_time, curr_time;
  float duration;
};


//===========================================
class HeadTrack : public EventTrigger
{
public:
  HeadTrack() : type(0), angle(0), is_inside(true), box_used(false) {}

  virtual void init_tracker(const TiXmlElement* elem);
  virtual bool eval();

  virtual void render(RenderDevice *rd);

private:
  enum Track {
    HEAD_NONE,
    HEAD_POINT,
    HEAD_DIR,
    HEAD_OBJ,
  };

  int type;
  bool box_used;
  bool is_inside;

  Vector3 point;
  Vector3 dir;
  ContentRef object;
  float angle;
  AABox loc_box;
};

//===========================================
class MoveTrack : public EventTrigger
{
public:
  MoveTrack() : is_inside(false), is_all(false) {}

  virtual void init_tracker(const TiXmlElement* elem);
  virtual bool eval();

  virtual void render(RenderDevice *rd);

private:
  bool is_inside;
  bool is_all;
  AABox the_box;
  GroupRef group;
};

#endif
