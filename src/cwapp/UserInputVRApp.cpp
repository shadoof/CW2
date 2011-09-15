#include "main.h"

using VRG3D::SynchedSystem;

//===========================================
void UserInputVRApp::init_userinput()
{
  user_input = new UserInput(_gwindow);
}

//===========================================
void UserInputVRApp::set_fp_manipulator(double rps, double mps)
{
  cam_control = FirstPersonManipulator::create();
  cam_control->setTurnRate(rps);
  cam_control->setMoveRate(mps);

  cam_control->onUserInput(user_input);
}

//===========================================
bool UserInputVRApp::eventToGEvent(const VRG3D::EventRef& ref_event, GEvent& event)
{
  std::string name = ref_event->getName();

  if ((ref_event->getType() == VRG3D::Event::EVENTTYPE_STANDARD) && !name.compare(0, 4, "kbd_")) {
    std::string keyname;

    if (endsWith(name, "_down")) {
      event.key.type = GEventType::KEY_DOWN;
    } else if (endsWith(name, "_up")) {
      event.key.type = GEventType::KEY_UP;
    } else {
      return false; //improperly named event, exit
    }

    event.key.state = GButtonState::PRESSED;

    event.key.which = 0;

    std::string newStr;
    newStr += name[4];

    GKey gkey = GKey::fromString(newStr);

    event.key.keysym.sym = *(GKey::Value*)&gkey;
    event.key.keysym.scancode = 0;
    event.key.keysym.unicode = 0;

    //Does not support modifiers for now (default UserInput controls don't use them)
    event.key.keysym.mod = GKeyMod::NONE;

    //char c = name[4];

    //if ((c >= 'A') && (c <= 'Z')) {
    //  // Make key codes lower case canonically
    //    event.key.keysym.sym = (GKey::Value)(c - 'A' + 'a');
    //} else {
    //  event.key.keysym.sym = (GKey::Value)c;
    //}
    return true;
  }

  if ((ref_event->getType() == VRG3D::Event::EVENTTYPE_2D) && beginsWith(name, "Mouse_")) {
    if (name == "Mouse_Left_Btn_down") {
      event.button.type = GEventType::MOUSE_BUTTON_DOWN;
      event.button.button = 0;
      return true;
    } else if (name == "Mouse_Left_Btn_up") {
      event.type = GEventType::MOUSE_BUTTON_UP;
      event.button.button = 0;
      return true;
    } else if (name == "Mouse_Right_Btn_down") {
      event.type = GEventType::MOUSE_BUTTON_DOWN;
      event.button.button = 1;
      return true;
    } else if (name == "Mouse_Right_Btn_up") {
      event.type = GEventType::MOUSE_BUTTON_UP;
      event.button.button = 1;
      return true;
    }
  }

  return false;
}

//===========================================
void UserInputVRApp::doUserInput (Array<VRG3D::EventRef> &events)
{
  if (!user_input || cam_control.isNull()) {
    return;
  }

  old_time = curr_time;
  curr_time = SynchedSystem::getLocalTime();

  cam_control->onSimulation(curr_time - old_time, 0, 0);

  user_input->beginEvents();

  for (int i = 0; i < events.length(); i++) {
    GEvent event;
    if (cam_control->active() && eventToGEvent(events[i], event)) {
      user_input->processEvent(event);
    }

    if (events[i]->getName() == "kbd_TAB_down") {
      if (cam_control->active()) {
        cam_control->setActive(false);
        setRoomToVirtualSpaceFrame(cam_control->frame());
      } else {
        cam_control->setActive(true);
        CoordinateFrame frame = cam_frame;
        //frame.translation -= Vector3::unitZ();
        cam_control->setFrame(frame);
      }
      continue;
    }

    if (events[i]->getName() == "kbd_ESC_down") {
      _endProgram = true;
    }
  }

  user_input->endEvents();

  if (cam_control->active()) {
    setRoomToVirtualSpaceFrame(cam_control->frame());
  }
}

//===========================================
void UserInputVRApp::setRoomToVirtualSpaceFrame(const CoordinateFrame& frame)
{ 
  cam_frame = frame;
  //cam_frame = frame.inverse();
  //cam_frame.translation += Vector3::unitZ();
}
