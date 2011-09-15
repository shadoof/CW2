#ifndef _TIMELINE_H_
#define _TIMELINE_H_

class Timeline : public CWBase
{
public:
  enum TimeState
  {
    NOT_RUNNING,
    PAUSED,
    RUNNING,
    DONE,
  };

  Timeline() : state(NOT_RUNNING), timeline_time(0), last_time(0), just_reset(false) {}

  void init(const TiXmlElement* root);
  void init_actions(const TiXmlElement* root);

  void run(bool restart);
  void pause();

  void process();

  bool is_done() const     { return (state == DONE); }
  bool is_running() const  { return (state == RUNNING); }
  bool is_paused() const   { return (state == PAUSED); }

  bool is_started() const  { return (is_running() || is_paused()); }

private:
  float last_time;
  float timeline_time;

  bool just_reset;

  TimeState state;

  multimap<float, ActionRef> actions;
  multimap<float, ActionRef>::iterator next_action;
};

typedef ReferenceCountedPointer<class Timeline> TimelineRef;

#endif
