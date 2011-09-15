#ifndef _PLACEMENT_H_
#define _PLACEMENT_H_

//===========================================
class CWBase : public ReferenceCountedObject
{
public:
  CWBase() {}

  virtual void init(const TiXmlElement* elem) = 0;
  string get_name() const { return name; }

protected:
  string name;
};

//===========================================
class Placement : public CWBase
{
public:
  Placement() : is_look_at(false) {}

  void init(const TiXmlElement* elem);

  CoordinateFrame apply(const CoordinateFrame& other)
  {
    return other * (world_frame/* * local_frame*/);
  }

  void set_world_frame(const CoordinateFrame& w) { world_frame = w; }
  CoordinateFrame get_world_frame() const        { return world_frame; }
  CoordinateFrame get_frame() const              { return (world_frame /** local_frame*/); }
  Vector3         get_pos() const                { return get_frame().translation; }

  void            invert_look_at();

  CoordinateFrame get_relative(const Placement& rel_place);

  static Vector3  compute_vel(const CoordinateFrame& end, const CoordinateFrame& start, float duration);


private:
  //CoordinateFrame local_frame;
  CoordinateFrame world_frame;

  string relative_to;
  bool is_look_at;
};

#endif
