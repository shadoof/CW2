#ifndef _ACTIONS_H_
#define _ACTIONS_H_

//===========================================
class Action : public ReferenceCountedObject
{
public:
  Action() : target(NONE), action_type(TRANSITION), link_change(LINK_NONE), trans_type(TRANS_VISIBLE),
             time_change(TIME_RESTART), exec_guard(false), is_random(false), command(0), random_val(-1) {}

  enum AppliesTo
  {
    NONE,
    ME,
    OBJECT,
    ALL,
    GROUP,
  };

  enum ActionType
  {
    CAVE_TRANSITION,
    MOVE_CAVE,
    MOVE_CAVE_REL,
    TIMELINE,
    LINK,
    TRANSITION,
    SOUND,
    RESET,
    EVENT,
  };

  enum LinkChangeType
  {
    LINK_NONE,
    LINK_ENABLE,
    LINK_DISABLE,
    LINK_ACTIVATE,
    LINK_ACTIVATE_IF_ENABLED,
  };

  enum TransitionType
  {
    TRANS_VISIBLE,
    TRANS_MOVE,
    TRANS_COLOR,
    TRANS_SCALE,
    TRANS_SOUND,
    TRANS_LINK,
    TRANS_MOVE_REL,
  };

  enum TimelineChange
  {
    TIME_START_IF_NOT_STARTED,
    TIME_PAUSE,
    TIME_RESUME,
    TIME_RESTART,
  };


  void init(const TiXmlElement* action, const ContentRef& me);
  //void init_applies_to(const TiXmlElement* root, const ContentRef& me);
  void init_link_change(const TiXmlElement* root);
  void init_time_change(const TiXmlElement* root);
  void init_transition(const TiXmlElement* root);

  void exec();
  void exec_link_change(ContentRef obj);
  void exec_transition();

  int group_begin();
  int group_end();

  GroupRef group;
  SoundRef sound;
  EventTriggerRef the_event;
  TimelineRef timeline;

  AppliesTo target;
  ActionType action_type;

  int command;
  int random_val;
  bool exec_guard;
  bool is_random;

  LinkChangeType link_change;
  TimelineChange time_change;

  TransitionType trans_type;

  float duration;

  WeakReferenceCountedPointer<class ActionInstance> weak_inst;

  CoordinateFrame start_frame, end_frame;

  Placement placement;
  Color4 color;
  float scale;
};

typedef ReferenceCountedPointer<Action> ActionRef;

//===========================================
class ActionInstance : public ReferenceCountedObject
{
public:
  ActionInstance() : start_time(0), end_time(0), duration(0) {}

  float start_time;
  float end_time;
  float duration;

  bool init(const ActionRef& action);
  bool process();

  ActionRef action;
  GroupRef group;
};

typedef ReferenceCountedPointer<ActionInstance> ActionInstRef;

#endif
