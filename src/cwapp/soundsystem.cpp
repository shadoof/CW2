#include "main.h"

using std::string;
using VRG3D::SynchedSystem;

#define ERRCHECK(X) if (!error_check(X)) { return false; }

//===========================================
SoundSystem::SoundSystem()
{
  system = NULL;
  speakermode = FMOD_SPEAKERMODE_RAW;
  error_handler = NULL;
}

//===========================================
bool SoundSystem::configure_output()
{
  //FMOD_RESULT result = system->setPluginPath("/Developer/FMOD/api/plugins");
  //if (result != FMOD_OK) {
  //  return false;
  //}

  unsigned int handle;
  unsigned int index = 0;
  FMOD_RESULT result = system->loadPlugin("./CW.app/Contents/Frameworks/motu_surround.dylib", &handle, index);

  if (result != FMOD_OK) {
    return false;
  }

  result = system->setOutputByPlugin(index);

  if (result != FMOD_OK) {
    return false;
  }

  speakermode = FMOD_SPEAKERMODE_5POINT1;
  return true;
}

//===========================================
bool SoundSystem::init(ErrorHandler err)
{
  FMOD_RESULT result;

  error_handler = err;

  result = FMOD::System_Create(&system);
  if (result != FMOD_OK) {
    return false;
  }

  unsigned int version;
  result = system->getVersion(&version);
  ERRCHECK(result);

  if (version < FMOD_VERSION) {
    printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
    return false;
  }
  
  //int num_drivers;
  //result = system->getNumDrivers(&num_drivers);
  
  /*for (int i = 0; i < num_drivers; i++) {
    char buf[256];
	
	result = system->getDriverName(i, buf, 256);
	ERRCHECK(result);
  }*/

  speakermode = FMOD_SPEAKERMODE_STEREO;

  //if (use_plugin_surround) {
  //#ifdef G3D_OSX
  configure_output();
  //#endif
  //}
  
  //result = system->setDSPBufferSize(64, 2);
  //ERRCHECK(result);
    
  //int driver = num_drivers - 1;
  //FMOD_CAPS caps;
  //int minf, maxf;
  //result = system->getDriverCaps(driver, &caps, &minf, &maxf, &speakermode);
  //ERRCHECK(result);
  

  
  //result = system->setDriver(driver);
  //ERRCHECK(result);
  
  //result = system->setOutput(FMOD_OUTPUTTYPE_COREAUDIO);
  //ERRCHECK(result);  
  
  result = system->setSpeakerMode(speakermode);
  ERRCHECK(result); 
  
  //result = system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCM8, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
  //ERRCHECK(result);
  
  //result = system->setSoftwareChannels(3);
  //ERRCHECK(result); 

  //result = system->setHardwareChannels(0, 0, 0, 0);
  //ERRCHECK(result); 

  result = system->init(100, FMOD_INIT_3D_RIGHTHANDED, 0);
  ERRCHECK(result);

  result = system->set3DSettings(1.0f, 3.28f, 1.0f);
  ERRCHECK(result);

  return true;
}

//===========================================
void SoundSystem::close()
{
  unload_all();
  if (system) {
    system->release();
    system = NULL;
  }
}

//===========================================
void SoundSystem::load_sounds(const Array<SoundRequest>& sound_files)
{
  for (int i = 0; i < sound_files.size(); i++) {
    string filename = sound_files[i].filename;

    SoundData sound;
    sound.sound = NULL;
    sound.flags = sound_files[i].flags;

    FMOD_MODE mode = FMOD_SOFTWARE;
    if (sound.flags & SOUND_2D) {
      mode |= FMOD_2D;
    } else {
      mode |= FMOD_3D_WORLDRELATIVE;
    }
    if (sound.flags & SOUND_UNIQUE) {
      mode |= FMOD_UNIQUE;
    }

    FMOD_RESULT result = system->createSound(filename.c_str(), mode, NULL, &sound.sound);
    if (!error_check(result)) {
      if (sound.sound) {
        delete sound.sound;
      }
      sound.sound = NULL;
    }

    sound.sound->setLoopCount(sound_files[i].repeat);

    float freq;
    int pri;
    sound.sound->getDefaults(&freq, NULL, NULL, &pri);
    float freq_scale = sound_files[i].freq;
    if (freq_scale < 0.2f) {
      freq_scale = 0.2f;
    }
    freq *= freq_scale;
    sound.sound->setDefaults(freq, sound_files[i].vol, sound_files[i].pan, pri);

    //sound.sound->setVariations(10000.0, 1.0, 1.0);
    //sound.sound->setDefaults(10000, 0.5, 1.0, 128);

    if (sound.flags & SOUND_AUTOSTART) {
      FMOD_RESULT result = system->playSound(FMOD_CHANNEL_REUSE, sound.sound, false, &sound.channel);
      error_check(result);
    }

    sounds.append(sound);
  }
}

//===========================================
bool SoundSystem::play_sound(unsigned int index, const Vector3& pos)
{
  if (index >= sounds.length()) {
    return false;
  }

  if (!sounds[index].sound) {
    return false;
  }

  if (sounds[index].channel) {
    bool playing;
    sounds[index].channel->isPlaying(&playing);
    if (playing) {
      //sounds[index].channel->stop();
      sounds[index].channel = NULL;
    }
  }

  FMOD_RESULT result = system->playSound(FMOD_CHANNEL_REUSE, sounds[index].sound, true, &sounds[index].channel);
  ERRCHECK(result);
  
  //cout << "Sound: " << pos.toString() << endl;
  
  //result = sounds[index].channel->setSpeakerMix(1.0, 1.0, 1.0, 1.0, 100.0, 100.0, 1.0, 1.0);
  //ERRCHECK(result);
  
  /*float  frontleft, 
  float  frontright, 
  float  center, 
  float  lfe, 
  float  backleft, 
  float  backright, 
  float  sideleft, 
  float  sideright 
);*/ 

//  bool is_play, is_virt;
//  sounds[index].channel->isPlaying(&is_play);
//  sounds[index].channel->isVirtual(&is_virt);
////  std::cout << string("Play ") << is_play string("Virt ") << is_virt << std::endl;
//  printf("Play: %d Virt: %d\n", is_play, is_virt);


  FMOD_VECTOR fmod_pos = vec3_to_fmod(pos);
  result = sounds[index].channel->set3DAttributes(&fmod_pos, NULL);
  result = sounds[index].channel->setPaused(false);
  return true;
}

//===========================================
bool SoundSystem::set_pos_vel(unsigned int index, const Vector3& pos, const Vector3& vel)
{
  if (index >= sounds.length()) {
    return false;
  }

  if (!sounds[index].channel) {
    return false;
  }

  //cout << vel.toString() << endl;

  FMOD_VECTOR fmod_pos = vec3_to_fmod(pos);
  FMOD_VECTOR fmod_vel = vec3_to_fmod(vel);

  FMOD_RESULT result = sounds[index].channel->set3DAttributes(&fmod_pos, &fmod_vel);
  return (result == FMOD_OK);
}

//===========================================
bool SoundSystem::start_interp(unsigned int index, const Vector3& start, const Vector3& end, float duration)
{
  if (index >= sounds.length()) {
    return false;
  }

  if (!sounds[index].channel) {
    return false;
  }

  sound_moves.resize(sound_moves.length() + 1);

  SoundMove *curr;
  curr = &sound_moves[sound_moves.length() - 1];
  curr->index = index;
  curr->start_pos = start;
  curr->end_pos = end;
  curr->duration = duration;
  curr->move_vel = (!duration ? Vector3::zero() : (end - start) / duration);
  curr->start_time = SynchedSystem::getAppRunningTime();
  return true;
}

//===========================================
bool SoundSystem::set_listener(const CoordinateFrame& frame, const Vector3& vel_vec)
{
  FMOD_VECTOR pos = vec3_to_fmod(frame.translation);
  FMOD_VECTOR vel = vec3_to_fmod(vel_vec);
  FMOD_VECTOR forward = vec3_to_fmod(frame.lookVector());
  FMOD_VECTOR up = vec3_to_fmod(frame.upVector());
  
  //cout << "Listener: " << frame.translation.toString() << endl; 

  FMOD_RESULT result = system->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
  return !error_check(result);
}

//===========================================
bool SoundSystem::stop(unsigned int index)
{
  if (index >= sounds.length()) {
    return false;
  }

  if (!sounds[index].channel) {
    return false;
  }

  FMOD_RESULT result = sounds[index].channel->stop();
  return !error_check(result);
}

//===========================================
void SoundSystem::unload_all()
{
  for (int i = 0; i < sounds.length(); i++) {
    if (sounds[i].channel) {
      sounds[i].channel->stop();
    }
    sounds[i].channel = NULL;

    if (sounds[i].sound) {
      sounds[i].sound->release();
      sounds[i].sound = NULL;
    }
  }
  sounds.clear();
}

//===========================================
bool SoundSystem::error_check(FMOD_RESULT result)
{
  if (result != FMOD_OK) {
    printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    if (error_handler) {
      error_handler(result, FMOD_ErrorString(result));
    }
    return false;
  }

  return true;
}

//===========================================
void SoundSystem::system_update()
{
  float curr_time = SynchedSystem::getAppRunningTime();

  for (int i = 0; i < sound_moves.length(); i++) {

    if ((curr_time >= (sound_moves[i].start_time + sound_moves[i].duration)) || !sound_moves[i].duration) {
      set_pos_vel(sound_moves[i].index, sound_moves[i].end_pos, Vector3::zero());
      sound_moves.fastRemove(i);
    } else {
      float elapsed = (curr_time - sound_moves[i].start_time) / sound_moves[i].duration;
      Vector3 curr = sound_moves[i].start_pos.lerp(sound_moves[i].end_pos, elapsed);
      set_pos_vel(sound_moves[i].index, curr, sound_moves[i].move_vel);
    }
  }

  system->update();
}
