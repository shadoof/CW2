#ifndef _BASE_SOUND_CLIENT_H_
#define _BASE_SOUND_CLIENT_H_

#include "main.h"
#include "soundsystem.h"
#include "SoundServerMessages.h"

class BaseSoundClient
{
public:
  virtual ~BaseSoundClient() {}

  virtual void close() = 0;
  virtual unsigned int add_sound(const SoundRequest& req) = 0;

  virtual bool is_succeeded() const = 0;

  virtual void load_all() = 0;
  virtual void system_update() = 0;
  virtual bool play_sound(unsigned int index, const Vector3& pos) = 0;
  virtual bool start_interp(unsigned int index, const Vector3& start, const Vector3& end, float duration) = 0;
  virtual bool set_pos_vel(unsigned int index, const Vector3& pos, const Vector3& vel) = 0;
  virtual bool set_listener(const CoordinateFrame& frame, const Vector3& vel) = 0;
  virtual bool stop(unsigned int index) = 0;
  virtual void unload_all() = 0;

  static void error_handler(unsigned int code, const char* msg);

private:
};


class NoSoundClient : public BaseSoundClient
{
  virtual void close() { return; }
  virtual unsigned int add_sound(const SoundRequest& req) { return 0; }

  virtual bool is_succeeded() const { return true; }

  virtual void load_all() { return; }
  virtual void system_update() { return; }
  virtual bool play_sound(unsigned int index, const Vector3& pos) { return false; }
  virtual bool start_interp(unsigned int index, const Vector3& start, const Vector3& end, float duration) { return false; }
  virtual bool set_pos_vel(unsigned int index, const Vector3& pos, const Vector3& vel) { return false; }
  virtual bool set_listener(const CoordinateFrame& frame, const Vector3& vel) { return false; }
  virtual bool stop(unsigned int index) { return false; }
  virtual void unload_all() { return; }

};

#endif
