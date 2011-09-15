#include "main.h"

//unsigned int SoundObject::_curHandle = 0;

//===========================================
//SoundObject
//===========================================
void 
SoundObject::init(const TiXmlElement* root)
{
  name = root->Attribute("name");

  filename = root->Attribute("filename");
  filename = Story::resolve_rel_path(filename);

  freq = 0;
  vol = 1.0f;
  pan = 0.0f;
  repeat = 0;
  sound_index = -1;

  SoundRequest req;
  req.filename = filename;

  bool autostart = false;
  assign(autostart, root->Attribute("autostart"));
  if (autostart) {
    req.flags |= SOUND_AUTOSTART;
  }

  const TiXmlElement *mode_elem = Parser::get_element(root, "Mode");
  if (mode_elem) {
    const TiXmlElement *child = Parser::get_first_element(mode_elem);
    if (child) {
      string name = child->ValueStr();
      if (name == "Fixed") {
        req.flags |= SOUND_2D;
      }
    }
  }

  const TiXmlElement *settings_elem = Parser::get_element(root, "Settings");
  if (settings_elem) {
    assign(req.freq, settings_elem->Attribute("freq"));
    assign(req.vol, settings_elem->Attribute("volume"));
    assign(req.pan, settings_elem->Attribute("pan"));
  }

  const TiXmlElement *repeat_elem = Parser::get_element(root, "Repeat");
  if (repeat_elem) {
    const TiXmlElement *child = Parser::get_first_element(repeat_elem);
    if (child) {
      string repeat_name = child->ValueStr();
      if (repeat_name == "RepeatForever") {
        repeat = -1;
      } else if (repeat_name == "RepeatNum") {
        assign(repeat, child->GetText());
      }
    }
  }
  req.repeat = repeat;
 
  sound_index = SOUNDCLIENT->add_sound(req);
}

void 
SoundObject::play(const Vector3& position)
{
//	if (!_activated) {
//		// Initialize the source
//		SOUNDCLIENT->makeSource(_soundNum, _handleNum);
//    _activated = true;
//	}
//	
//  set_pos(position);
//  SOUNDCLIENT->setLooping(_handleNum, _looping);
//	SOUNDCLIENT->play(_handleNum);
  SOUNDCLIENT->play_sound(sound_index, position);
}

void 
SoundObject::play()
{
//  if (!_activated) {
//    // Initialize the source
//    SOUNDCLIENT->makeSource(_soundNum, _handleNum);
//    _activated = true;
//  }
//  set_pos(the_app.get_cam_pos());
//  SOUNDCLIENT->setLooping(_handleNum, _looping);
//  //SOUNDCLIENT->setListenerLoc(CoordinateFrame());
//  SOUNDCLIENT->play(_handleNum);
  //SOUNDCLIENT->setListenerLoc(the_app.get_cam_frame());
  SOUNDCLIENT->play_sound(sound_index, the_app.get_actual_frame().translation);

}

void 
SoundObject::set_pos_vel(const Vector3& position, const Vector3& vel)
{
	//Content::interp_frame(end_place, elapsed);
	
	SOUNDCLIENT->set_pos_vel(sound_index, position, vel);
//	if(_doppler)
//	{
//		// TODO: Doppler
//		// SOUNDCLIENT->setSourceVel(_handleNum, velocity );
//	}
}