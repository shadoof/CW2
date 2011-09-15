#ifndef _USER_INPUT_VR_APP_H_
#define _USER_INPUT_VR_APP_H_

#include <VRG3D.H>

typedef ReferenceCountedPointer<Manipulator> ManipulatorRef;

class UserInputVRApp : public VRG3D::VRApp
{
public:
  void init_userinput();
  void set_fp_manipulator(double rps, double mps);

  static bool eventToGEvent(const VRG3D::EventRef& ref_event, GEvent& event);

  void doUserInput(Array<VRG3D::EventRef> &events);

  void setRoomToVirtualSpaceFrame(const CoordinateFrame& frame);

protected:
  FirstPersonManipulatorRef cam_control;
  CoordinateFrame cam_frame;
  //CoordinateFrame world_frame;

  UserInput* user_input;

  double old_time;
  double curr_time;
};


#endif
