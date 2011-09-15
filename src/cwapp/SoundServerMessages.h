/*
 *  SoundServerMessages.H
 *  OpenALServer
 *
 *  Created by Nicholas Musurca on 12/16/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef SOUNDSERVERMESSAGES_H
#define SOUNDSERVERMESSAGES_H

#define SOUND_SERVER_PORT 6969

typedef enum SoundServerMessages 
{
  SM_SOUND_LOAD = 6901,
  SM_SOUND_LOAD_ACK,
  SM_SOUND_UNLOAD_ALL,
  SM_SOUND_SETPOSVEL,
  SM_SOUND_PLAY,
  SM_SOUND_STOP,
  SM_SOUND_SETLISTENER,
  SM_SOUND_INTERP,
  SM_SOUND_ERROR,
};

#define uint32 unsigned int


//==============================================================
//==============================================================
class SoundServerNetMessageLoad
{
public:
	SoundServerNetMessageLoad() { }
	
	SoundServerNetMessageLoad(const Array<SoundRequest>& soundfiles, const string& log)
	{
    _login = log;
		_soundfiles = soundfiles;
	}
	
	virtual ~SoundServerNetMessageLoad() { }
	
	uint32 type() const { return SM_SOUND_LOAD; }
	
	void serialize(BinaryOutput &b) const 
	{
    b.writeString(_login);
		b.writeInt32(_soundfiles.size());

    for (int i = 0; i < _soundfiles.size(); i++) {
      _soundfiles[i].serialize(b);
    }
	}
  
	void deserialize(BinaryInput &b) 
	{
    _login = b.readString();

		int size  = b.readInt32();

    _soundfiles.resize(size);

    for (int i = 0; i < size; i++) {
      _soundfiles[i].deserialize(b);
    }
	}

	const Array<SoundRequest>& getSoundArray()     { return _soundfiles; }
                      string getLogin()          { return _login; }

private:
	Array<SoundRequest> _soundfiles;
  string _login;
};


//==============================================================
//==============================================================
class SoundServerNetMessageLoadResponse
{
public:
  SoundServerNetMessageLoadResponse() { }

  SoundServerNetMessageLoadResponse(unsigned int sid) { session = sid; }
	
	virtual ~SoundServerNetMessageLoadResponse() { }
	
	uint32 type() const { return SM_SOUND_LOAD_ACK; }
	
	void serialize(BinaryOutput &b) const
  {
    b.writeUInt32(session);
  }

  void deserialize(BinaryInput &b)
  {
    session = b.readUInt32();
  }

private:
  unsigned int session;
};

//==============================================================
//==============================================================
class SoundServerNetMessageUnloadAll
{
public:
	SoundServerNetMessageUnloadAll() { }
	
	virtual ~SoundServerNetMessageUnloadAll() { }
	
	uint32 type() const { return SM_SOUND_UNLOAD_ALL; }
	
  void serialize(BinaryOutput &b) const {}
  void deserialize(BinaryInput &b) {}
};

//==============================================================
//==============================================================
class SoundServerNetMessageSetPosVel
{
public:
	SoundServerNetMessageSetPosVel() { }
	
	SoundServerNetMessageSetPosVel(unsigned int sindex, const Vector3 &spos, const Vector3& svel)
	{
		index = sindex;
		pos = spos;
    vel = svel;
	}

	virtual ~SoundServerNetMessageSetPosVel() { }
	
	uint32 type() const { return SM_SOUND_SETPOSVEL; }
	
	void serialize(BinaryOutput &b) const 
	{
		b.writeUInt32(index);
		pos.serialize(b);
    vel.serialize(b);
	}
  
	void deserialize(BinaryInput &b) 
	{
		index = b.readUInt32();
		pos.deserialize(b);
    vel.deserialize(b);
	}
	
	unsigned int get_index() { return index; }
	Vector3 get_pos()        { return pos; }
  Vector3 get_vel()        { return vel; }

private:
	unsigned int index;
	Vector3 pos;
  Vector3 vel;
};

//==============================================================
//==============================================================
class SoundServerNetMessagePlay
{
public:
	SoundServerNetMessagePlay() {}

	SoundServerNetMessagePlay(unsigned int sindex, const Vector3& spos)
	{ 
		index = sindex;
    pos = spos;
	}

	virtual ~SoundServerNetMessagePlay() {}

	uint32 type() const { return SM_SOUND_PLAY; }  

	void serialize(BinaryOutput &b) const 
	{
		b.writeUInt32(index);
    pos.serialize(b);
	}
  
	void deserialize(BinaryInput &b) 
	{
    index = b.readUInt32();
    pos.deserialize(b);
	}

  unsigned int get_index() { return index; }
  Vector3 get_pos()        { return pos; }

private:
  unsigned int index;
  Vector3 pos;
};

//==============================================================
//==============================================================
class SoundServerNetMessageStop
{
public:
	SoundServerNetMessageStop() {}

  SoundServerNetMessageStop(unsigned int sindex)
  { 
    index = sindex;
  }

  virtual ~SoundServerNetMessageStop() {}

  uint32 type() const { return SM_SOUND_STOP; }  

  void serialize(BinaryOutput &b) const 
  {
    b.writeUInt32(index);
  }

  void deserialize(BinaryInput &b) 
  {
    index = b.readUInt32();
  }

  unsigned int get_index() { return index; }

private:
  unsigned int index;
};

//==============================================================
//==============================================================
class SoundServerNetMessageSetListener
{
public:
	SoundServerNetMessageSetListener() { }
	
	SoundServerNetMessageSetListener(const CoordinateFrame &loc_frame, const Vector3& svel)
	{
    frame = loc_frame;
    vel = svel;
	}

	virtual ~SoundServerNetMessageSetListener() { }
	
	uint32 type() const { return SM_SOUND_SETLISTENER; }
	
	void serialize(BinaryOutput &b) const 
	{
    frame.serialize(b);
    vel.serialize(b);
	}
  
	void deserialize(BinaryInput &b) 
	{
    frame.deserialize(b);
    vel.deserialize(b);
	}

	CoordinateFrame get_frame() const { return frame; }
  Vector3 get_vel() const           { return vel; }

private:
  CoordinateFrame frame;
  Vector3 vel;
};

//==============================================================
//==============================================================
class SoundServerNetMessageInterp
{
public:
  SoundServerNetMessageInterp() { }

  SoundServerNetMessageInterp(unsigned int sindex, const Vector3& start, const Vector3& end, float dur)
  {
    index = sindex;
    start_pos = start;
    end_pos = end;
    duration = dur;
  }

  virtual ~SoundServerNetMessageInterp() { }

  uint32 type() const { return SM_SOUND_INTERP; }

  void serialize(BinaryOutput &b) const 
  {
    start_pos.serialize(b);
    end_pos.serialize(b);
    b.writeUInt32(index);
    b.writeFloat32(duration);
  }

  void deserialize(BinaryInput &b) 
  {
    start_pos.deserialize(b);
    end_pos.deserialize(b);
    index = b.readUInt32();
    duration = b.readFloat32();
  }

  Vector3 get_start_pos()  const { return start_pos; }
  Vector3 get_end_pos()    const { return end_pos; }
  float   get_duration()   const { return duration; }
  unsigned int get_index() const { return index; }


private:
  Vector3 start_pos, end_pos;
  unsigned int index;
  float duration;
};

//==============================================================
//==============================================================
class SoundServerNetMessageError
{
public:
  SoundServerNetMessageError() {}

  SoundServerNetMessageError(const string& err)
  { 
    error = err;
  }

  virtual ~SoundServerNetMessageError() {}

  uint32 type() const { return SM_SOUND_ERROR; }  

  void serialize(BinaryOutput &b) const 
  {
    b.writeString(error);
  }

  void deserialize(BinaryInput &b) 
  {
    error = b.readString();
  }

  string get_error() const { return error; }

private:
  string error;
};

#undef uint32

#endif
