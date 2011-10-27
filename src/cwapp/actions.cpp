#include "Parser.h"
#include "main.h"

using VRG3D::SynchedSystem;

//===========================================
//Action
//===========================================
void Action::init(const TiXmlElement* root, const ContentRef& me)
{
  const TiXmlElement* action_choice = Parser::get_first_element(root);
  debugAssert(action_choice);
  string choice = action_choice->ValueStr();

  if (choice == "CaveChange") {
    action_type = CAVE_TRANSITION;
    init_transition(Parser::get_element(action_choice, "Transition"));
  } else if (choice == "ObjectChange") {
    ContentRef obj;

    string obj_name = action_choice->Attribute("name");
    obj = Story::the_story->get_object(obj_name);
  
    if (obj.isNull()) {
      obj = me;
      target = ME;
    } else {
      target = OBJECT;
    }

    if (obj.notNull()) {
      group = new Group();
      group->add(obj);
      action_type = TRANSITION;
      init_transition(Parser::get_element(action_choice, "Transition"));
    }
  } else if (choice == "GroupRef") {
    bool is_all;
    assign(is_all, action_choice->Attribute("all"));
    if (is_all) {
      target = ALL;
      group = Story::the_story;
    } else {
      target = GROUP;
      string group_name = action_choice->Attribute("name");
      group = Story::the_story->get_group(group_name);
    }
    //debugAssertM(group.notNull(), "No group for action");
    string randoms = action_choice->Attribute("random");
    if (randoms == "Select One Randomly") {
      is_random = true;
    }

    action_type = TRANSITION;
    init_transition(Parser::get_element(action_choice, "Transition"));
  } else if (choice == "MoveCave") {
    if (Parser::get_element(action_choice, "Relative")) {
      action_type = MOVE_CAVE_REL;
    } else {
      action_type = MOVE_CAVE;
    }
    placement.init(Parser::get_element(action_choice, "Placement"));
    assign(duration, action_choice->Attribute("duration"));
    placement.set_world_frame(placement.get_world_frame().inverse());

  } else if (choice == "SoundRef") {
    action_type = SOUND;
    sound = Story::the_story->get_sound(action_choice->Attribute("name"));
  } else if (choice == "TimerChange") {
    action_type = TIMELINE;
    timeline = Story::the_story->get_timeline(action_choice->Attribute("name"));
    init_time_change(Parser::get_first_element(action_choice));
  } else if (choice == "Restart") {
    action_type = RESET;
  } else if (choice == "Event") {
    action_type = EVENT;
    the_event = Story::the_story->get_event(action_choice->Attribute("name"));
    
    bool bool_cmd;
    assign(bool_cmd, action_choice->Attribute("enable"));
    command = bool_cmd;
  }
}

//===========================================
void Action::init_link_change(const TiXmlElement* root)
{
  if (!root) {
    return;
  }

  string value = root->ValueStr();

  if (value == "link_on") {
    link_change = LINK_ENABLE;
  } else if (value == "link_off") {
    link_change = LINK_DISABLE;
  } else if (value == "activate") {
    link_change = LINK_ACTIVATE;
  } else if (value == "activate_if_on") {
    link_change = LINK_ACTIVATE_IF_ENABLED;
  } else {
    alwaysAssertM(0, "Invalid");
  }
}

//===========================================
void Action::init_time_change(const TiXmlElement* root)
{
  if (!root) {
    return;
  }

  string value = root->ValueStr();

  if (value == "start") {
    time_change = TIME_RESTART;
  } else if (value == "stop") {
    time_change = TIME_PAUSE;
  } else if (value == "continue") {
    time_change = TIME_RESUME;
  } else if (value == "start_if_not_started") {
    time_change = TIME_START_IF_NOT_STARTED;
  } else {
    alwaysAssertM(0, "Invalid");
  }
}

//===========================================
void Action::init_transition(const TiXmlElement* root)
{
  const TiXmlElement* trans = Parser::get_first_element(root);
  debugAssert(trans);
  string trans_name = trans->ValueStr();

  assign(duration, root->Attribute("duration"));

  if (trans_name == "Visible") {
    trans_type = TRANS_VISIBLE;
    bool visibility;
    assign(visibility, trans->GetText());
    color.a = (visibility ? 1.f : 0.f);
  } else if (trans_name == "Movement") {
    trans_type = TRANS_MOVE;
    placement.init(Parser::get_first_element(trans));
  } else if (trans_name == "MoveRel") {
    trans_type = TRANS_MOVE_REL;
    placement.init(Parser::get_first_element(trans));
  } else if (trans_name == "Color") {
    trans_type = TRANS_COLOR;
    assign(color, trans->GetText());
  } else if (trans_name == "Scale") {
    trans_type = TRANS_SCALE;
    assign(scale, trans->GetText());
  } else if (trans_name == "Sound") {
    trans_type = TRANS_SOUND;
  } else if (trans_name == "LinkChange") {
    trans_type = TRANS_LINK;
    init_link_change(Parser::get_first_element(trans));
  } else {
    alwaysAssertM(0, "Invalid Element");
  }
}

//===========================================
void Action::exec()
{
  if (exec_guard) {
    return;
  }

  exec_guard = true;
  
  if (is_random) {
     random_val = G3D::iRandom(0, group->get_size() - 1);
  }

  CoordinateFrame rel_frame;

  switch (action_type) {
    case TRANSITION:
      if (trans_type == TRANS_SOUND) {
        for (int i = group_begin(); i < group_end(); i++) {
          ContentRef obj = group->get_object(i);
          obj->play_sound();
        }
      //} else if (trans_type == TRANS_LINK) {
      //  exec_link_change();
      } else {
        exec_transition();
      }
      break;

    case SOUND:
      if (sound.notNull()) {
        sound->play();
      }
      break;

    case MOVE_CAVE:
      //start_frame = the_app.cave_frame;
      start_frame = the_app.cave_frame;
      end_frame = placement.get_world_frame();
      the_app.cave_vel = Placement::compute_vel(end_frame, start_frame, duration);
      exec_transition();
      break;

    case MOVE_CAVE_REL:
      start_frame = the_app.cave_frame;
      end_frame = placement.get_world_frame() * start_frame;
      end_frame.rotation.orthonormalize();
      //start_frame = the_app.cave_frame;
      //end_frame = start_frame * placement.get_world_frame();
      the_app.cave_vel = Placement::compute_vel(end_frame, start_frame, duration);
      exec_transition();
      break;

    case CAVE_TRANSITION:
      exec_transition();
      break;

    case TIMELINE:
      if (timeline.notNull()) {
        switch (time_change) {
          case TIME_RESTART:
            timeline->run(true);
            break;

          case TIME_START_IF_NOT_STARTED:
            if (!timeline->is_started()) {
              timeline->run(true);
            }
            break;

          case TIME_PAUSE:
            timeline->pause();
            break;

          case TIME_RESUME:
            timeline->run(false);
            break;
        }
      }
      break;

    case RESET:
      the_app.reload_exit = true;
      break;

    case EVENT:
      if (the_event.notNull()) {
        the_event->set_enabled(command);
      }
      break;
  }

  exec_guard = false;
}

//===========================================
void Action::exec_link_change(ContentRef obj)
{
  switch (link_change) {
    case LINK_ACTIVATE:
      obj->activate();
      break;

    case LINK_ACTIVATE_IF_ENABLED:
      if (obj->is_active_link()) {
        obj->activate();
      }
      break;

    case LINK_ENABLE:
      obj->set_link_active(true);
      break;

    case LINK_DISABLE:
      obj->set_link_active(false);
      break;
  }
}

//===========================================
void Action::exec_transition()
{
  ActionInstRef instance = weak_inst.createStrongPtr();

  if (instance.notNull()) {
    return;
  }

  instance = new ActionInstance();

  if (!instance->init(this)) {
    return;
  }

  switch (action_type) {
    case TRANSITION:
      if (group.isNull()) {
        return;
      }
      for (int i = group_begin(); i < group_end(); i++) {
        ContentRef obj = group->get_object(i);
        obj->begin_interp(trans_type, this);

        if (trans_type == Action::TRANS_LINK) {
          exec_link_change(obj);
        }
      }
      break;
    case CAVE_TRANSITION:
      the_app.begin_interp(trans_type, this);
      break;
  }

  weak_inst = instance;

  Story::the_story->add_action(instance);
}

//===========================================
int Action::group_begin()
{
  if (is_random) {
    return random_val;
  } else {
    return 0;
  }
}

//===========================================
int Action::group_end()
{
  if (is_random) {
    return random_val + 1;
  } else {
    return group->get_size();
  }
}

//===========================================
//ActionInstance
//===========================================
bool ActionInstance::init(const ActionRef& act_ref)
{
  action = act_ref;
  start_time = SynchedSystem::getAppRunningTime();
  end_time = start_time + action->duration;
  group = action->group;
  return true;
}

//===========================================
bool ActionInstance::process()
{
  float curr_time = SynchedSystem::getAppRunningTime();
  
  bool done;
  float elapsed;

  if ((curr_time >= end_time) || !action->duration) {
    done = true;
    elapsed = 1.0f;
  } else {
    done = false;
    elapsed = (curr_time - start_time) / action->duration;
  }

  if (action->action_type == Action::TRANSITION) {
    for (int i = action->group_begin(); i < action->group_end(); i++) {
      ContentRef obj = group->get_object(i);
      switch (action->trans_type) {
        case Action::TRANS_VISIBLE:
          obj->interp_alpha(action->color.a, elapsed);
          break;

        case Action::TRANS_MOVE:
          obj->interp_frame(action->placement.get_world_frame(), elapsed);
          break;

        case Action::TRANS_MOVE_REL:
          obj->interp_frame(obj->get_end_frame(), elapsed);
          break;

        case Action::TRANS_COLOR:
          obj->interp_color(action->color, elapsed);
          break;

        case Action::TRANS_SCALE:
          obj->interp_scale(action->scale, elapsed);
          break;

        case Action::TRANS_LINK:
          if (elapsed == 1.f) {
            obj->select(false);
          }

      }
    }
  } else if (action->action_type == Action::CAVE_TRANSITION) {
  	  switch (action->trans_type) {
        /*case Action::TRANS_VISIBLE:
          the_app.interp_alpha(action->color.a, elapsed);
          break;*/

        // TODO: get cave move working here too
        /*case Action::TRANS_MOVE:
          the_app.interp_frame(action->placement.get_world_frame(), elapsed);
          break;

        case Action::TRANS_MOVE_REL:
          the_app.interp_frame(obj->get_end_frame(), elapsed);
          break;*/

        case Action::TRANS_COLOR:
          the_app.interp_color(action->color, elapsed);
          break;

        /*case Action::TRANS_SCALE:
          the_app.interp_scale(action->scale, elapsed);
          break;*/

        /*case Action::TRANS_LINK:
          if (elapsed == 1.f) {
            obj->select(false);
          }*/
      }
  } else if ((action->action_type == Action::MOVE_CAVE) || (action->action_type == Action::MOVE_CAVE_REL)) {
    the_app.cave_frame = (action->start_frame.lerp(action->end_frame, elapsed));
    if (done) {
      the_app.cave_vel = Vector3::zero();
    }
    //cout << elapsed << " " << the_app.cave_frame.toXML() << endl;
  }

  return done;
}





