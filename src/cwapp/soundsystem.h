#ifndef _SOUND_SYSTEM_H_
#define _SOUND_SYSTEM_H_

#include <fmod.hpp>
#include <fmod_errors.h>

enum SoundEnums
{
  SOUND_2D = 0x01,
  SOUND_MULTIPLE = 0x02,
  SOUND_UNIQUE =  0x04,
  SOUND_AUTOSTART = 0x08,
};

struct SoundRequest
{
  SoundRequest() : flags(0), freq(0), vol(0), pan(0), repeat(0) {}

  void serialize(BinaryOutput& out) const
  {
    out.writeString(filename);
    out.writeInt32(flags);
    out.writeFloat32(freq);
    out.writeFloat32(vol);
    out.writeFloat32(pan);
    out.writeInt32(repeat);
  }

  void deserialize(BinaryInput& in)
  {
    filename = in.readString();
    flags = in.readInt32();
    freq = in.readFloat32();
    vol = in.readFloat32();
    pan = in.readFloat32();
    repeat = in.readInt32();
  }

  string filename;
  int flags;
  float freq, vol, pan;
  int repeat;
};

typedef void (*ErrorHandler)(unsigned int, const char* err);

class SoundSystem
{
public:
  SoundSystem();
  bool init(ErrorHandler handler);
  bool configure_output();
  void close();
  void load_sounds(const Array<SoundRequest>& sound_files);

  bool play_sound(unsigned int index, const Vector3& pos);
  bool set_pos_vel(unsigned int index, const Vector3& pos, const Vector3& vel);
  bool start_interp(unsigned int index, const Vector3& start, const Vector3& end, float duration);
  bool set_listener(const CoordinateFrame& frame, const Vector3& vel);
  bool stop(unsigned int index);
  void unload_all();

  void system_update();

private:
  struct SoundData
  {
    SoundData() : flags(0), sound(NULL), channel(NULL) {}

    string filename;
    int flags;
    FMOD::Sound* sound;
    //Array<Channel*> channels;
    FMOD::Channel* channel;
  };

  struct SoundMove
  {
    Vector3 start_pos;
    Vector3 end_pos;
    Vector3 move_vel;
    float duration;
    float start_time;
    unsigned int index;
  };


  bool error_check(FMOD_RESULT result);
  inline FMOD_VECTOR vec3_to_fmod(const Vector3& vec)
  {
    FMOD_VECTOR fmod;
    fmod.x = vec.x;
    fmod.y = vec.y;
    fmod.z = vec.z;
    return fmod;
  }

  Array<SoundData> sounds;
  Array<SoundMove> sound_moves;

  ErrorHandler error_handler;

  FMOD::System *system;
  FMOD_SPEAKERMODE speakermode;
};


#endif
