#include "main.h"

//===========================================
LocalSoundClient::LocalSoundClient()
{
  success = sound_sys.init(&BaseSoundClient::error_handler);
}

//===========================================
void LocalSoundClient::close()
{
  sound_sys.close();
}

//===========================================
unsigned int LocalSoundClient::add_sound(const SoundRequest& req)
{
  unsigned int len = requests.length();
  requests.append(req);
  return len;
}

//===========================================
void LocalSoundClient::load_all()
{
  sound_sys.load_sounds(requests);
}

//===========================================
void BaseSoundClient::error_handler(unsigned int code, const char* msg)
{
  string err_msg = "Sound Error: ";
  err_msg += msg;
#ifndef G3D_LINUX
  G3D::msgBox(err_msg, "Sound Error");
#endif
}
