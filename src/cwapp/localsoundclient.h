#ifndef _LOCAL_SOUND_CLIENT_H_
#define _LOCAL_SOUND_CLIENT_H_

class LocalSoundClient : public BaseSoundClient
{
public:
  LocalSoundClient();
  virtual ~LocalSoundClient() {}

  void close();

  bool is_succeeded() const { return success; }

  unsigned int add_sound(const SoundRequest& req);
  void         load_all();

  void system_update()
  {
    sound_sys.system_update();
  }

  bool play_sound(unsigned int index, const Vector3& pos)
  {
    return sound_sys.play_sound(index, pos);
  }

  bool set_pos_vel(unsigned int index, const Vector3& pos, const Vector3& vel)
  {
    return sound_sys.set_pos_vel(index, pos, vel);
  }

  bool start_interp(unsigned int index, const Vector3& start, const Vector3& end, float duration)
  {
    return sound_sys.start_interp(index, start, end, duration);
  }

  bool set_listener(const CoordinateFrame& frame, const Vector3& vel)
  {
    return sound_sys.set_listener(frame, vel);
  }

  bool stop(unsigned int index)
  {
    return sound_sys.stop(index);
  }

  void unload_all()
  {
    sound_sys.unload_all();
  }


private:
  SoundSystem sound_sys;
  Array<SoundRequest> requests;
  bool success;
};

#endif
