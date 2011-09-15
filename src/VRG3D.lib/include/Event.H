/**
 * \author Daniel Keefe (dfk)
 *
 * \file  Event.H
 * \brief
 *
 */

#ifndef VRG3DEVENT_H
#define VRG3DEVENT_H

#include <G3D/G3DAll.h>

namespace VRG3D {

class Event;
typedef ReferenceCountedPointer<class Event> EventRef;

/// Creates a copy of the Event pointed to by e and returns a ref
/// counted pointer to the new copy.
EventRef createCopyOfEvent(EventRef e);

/** G3DVR Event class.  To keep things simple, there are no subclasses
    of Event.  The type of data that the event carries is interpreted
    differently based on the value of the type of the event.  Button
    Events are typically sent by devices as two separate
    EVENTTYPE_STANDARD Events, the first named ButtonName_down and
    then when the button is released ButtonName_up.
*/
class Event : public ReferenceCountedObject
{
public:

  enum EventType {  
    EVENTTYPE_STANDARD = 0,        /// standard type, carries no additional info
    EVENTTYPE_1D = 1,              /// event that stores 1D data in a double
    EVENTTYPE_2D = 2,              /// stores two doubles
    EVENTTYPE_3D = 3,              /// stores three doubles
    EVENTTYPE_COORDINATEFRAME = 4, /// stores a G3D::CoordinateFrame
    EVENTTYPE_MSG = 5              /// stores a std::string
  };

  Event(const std::string &name) { 
    _name = name;
    _type = EVENTTYPE_STANDARD;
  }

  Event(const std::string &name, const double data) { 
    _name = name;
    _data1D = data;
    _type = EVENTTYPE_1D;
  }

  Event(const std::string &name, const Vector2 &data) { 
    _name = name;
    _data2D = data;
    _type = EVENTTYPE_2D;
  }
  
  Event(const std::string &name, const Vector3 &data) { 
    _name = name;
    _data3D = data;
    _type = EVENTTYPE_3D;
  }

  Event(const std::string &name, const CoordinateFrame &data) { 
    _name = name;
    _dataCF = data;
    _type = EVENTTYPE_COORDINATEFRAME;
  }
  
  Event(const std::string &name, const std::string &data) { 
    _name = name;
    _dataMsg = data;
    _type = EVENTTYPE_MSG;
  }

  virtual ~Event() {}


  std::string     getName() const { return _name; }
  EventType       getType() const { return _type; }

  double          get1DData();
  Vector2         get2DData();
  Vector3         get3DData();
  CoordinateFrame getCoordinateFrameData();
  std::string     getMsgData();

  std::string     toString();

  void            rename(const std::string &newname) { _name = newname; }

  void serialize(BinaryOutput &b) const;
  void deserialize(BinaryInput &b);

protected:
  std::string     _name;
  EventType       _type;
  double          _data1D;
  Vector2         _data2D;
  Vector3         _data3D;
  CoordinateFrame _dataCF;
  std::string     _dataMsg;
};


} // end namespace

#endif

