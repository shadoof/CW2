#ifndef SOUNDCONTENT_H
#define SOUNDCONTENT_H

//===========================================
class SoundObject : public CWBase
{
public:
  //static unsigned int _curHandle;

  virtual void init(const TiXmlElement* root);
  virtual void play(const Vector3& position);
  virtual void play();
  virtual void set_pos_vel(const Vector3& position, const Vector3& vel);

  unsigned int get_index() const { return sound_index; }


  
private:
	string filename;
  float freq, vol, pan;
  int repeat;
  unsigned int sound_index;
};

typedef ReferenceCountedPointer<SoundObject> SoundRef;

#endif
