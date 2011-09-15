#include "main.h"

using VRG3D::SynchedSystem;

//===========================================
void Timeline::init(const TiXmlElement* root)
{
  name = root->Attribute("name");
}

//===========================================
void Timeline::init_actions(const TiXmlElement* root)
{
  Array<const TiXmlNode*> nodes;
  Parser::get_elements(nodes, root, "TimedActions");

  for (int i = 0; i < nodes.size(); i++) {
    const TiXmlElement *action_elem = (const TiXmlElement*)nodes[i];

    float start_time;
    assign(start_time, action_elem->Attribute("seconds-time"));

    ActionRef action = new Action();
    action->init(action_elem, ContentRef());

    //actions[start_time] = action;
    actions.insert(std::pair<float, ActionRef>(start_time, action));
  }

  bool start_immed;
  assign(start_immed, root->Attribute("start-immediately"));

  if (start_immed && actions.size()) {
    run(true);
    just_reset = false;
  }
}

//===========================================
void Timeline::run(bool restart)
{
  if (restart) {
    timeline_time = 0;
    next_action = actions.begin();
  }

  last_time = SynchedSystem::getAppRunningTime();

  state = RUNNING;

  just_reset = true;
}

//===========================================
void Timeline::pause()
{
  if (state == RUNNING) {
    state = PAUSED;
  }
}

//===========================================
void Timeline::process()
{
  if (state != RUNNING) {
    return;
  }

  float curr_time = (float)SynchedSystem::getAppRunningTime();
  timeline_time += (curr_time - last_time);
  last_time = curr_time;
 
  while ((next_action != actions.end()) && ((*next_action).first < timeline_time) && !the_app.reload_exit) {
    (*next_action).second->exec();
    if (just_reset) {
      just_reset = false;
    } else {
      next_action++;
    }
  }

  if (next_action == actions.end()) {
    state = DONE;
  }
}
