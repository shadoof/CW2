#include "main.h"

//===========================================
//Placement
//===========================================
void Placement::init(const TiXmlElement* elem)
{
  debugAssert(elem->ValueStr() == "Placement");

  name = elem->Attribute("name");

  relative_to = Parser::get_element_value(elem, "RelativeTo");

  Vector3 origin;
  assign(origin, Parser::get_element_value(elem, "Position"));
  world_frame.translation = origin;

  if (relative_to.length()) {
    PlaceRef place = Story::the_story->get_place(relative_to);
    if (place.notNull()) {
      world_frame = world_frame * place->get_frame();
      is_look_at = is_look_at || place->is_look_at;
    }
  }

  const TiXmlElement* axis;
  const TiXmlElement* lookAt;
  float angle = 0;

  if (axis = Parser::get_element(elem, "Axis")) {
    Vector3 rot;
    assign(rot, axis->Attribute("rotation"));
    assign(angle, axis->Attribute("angle"));
    if (angle) {
      angle *= G3D::pi() / 180.0;
    }
    world_frame.rotation = world_frame.rotation * Matrix3::fromAxisAngle(rot, angle);

  } else if (lookAt = Parser::get_element(elem, "LookAt")) {
    //Vector3 pos_real = frame.translation;
    //frame.translation.unitize();
    Vector3 point, up;
    assign(point, lookAt->Attribute("target"));
    assign(up, lookAt->Attribute("up"));
    if (point.fuzzyEq(world_frame.translation)) {
      point.z += 1.f;
    }
    //world_frame.translation = origin;
    world_frame.lookAt(point, up);
    world_frame = world_frame * Matrix3::fromAxisAngle(Vector3::unitY(), G3D::pi());

    //world_frame.translation = Vector3::zero();
    //frame.translation = pos_real;
    is_look_at = true;
  } else if (lookAt = Parser::get_element(elem, "Normal")) {
    Vector3 normal;
    assign(normal, lookAt->Attribute("normal"));
    normal.unitize();

    world_frame.lookAt(world_frame.translation + normal);

    assign(angle, lookAt->Attribute("angle"));
    if (angle) {
      angle *= G3D::pi() / 180.0;
      world_frame.rotation = world_frame.rotation * Matrix3::fromAxisAngle(normal, angle);
    }
  }
}

//===========================================
void Placement::invert_look_at()
{
  if (is_look_at) {
    //world_frame = world_frame * Matrix3::fromAxisAngle(Vector3::unitY(), G3D::pi());
  }
}

//===========================================
CoordinateFrame Placement::get_relative(const Placement& rel_place)
{
  CoordinateFrame frame;
  frame = rel_place.world_frame * world_frame;
  frame.rotation.orthonormalize();
  return frame;
}

//===========================================
Vector3  Placement::compute_vel(const CoordinateFrame& end, const CoordinateFrame& start, float duration)
{
  if (!duration) {
    return Vector3::zero();
  }

  Vector3 dist = end.translation - start.translation;
  dist /= duration;
  return dist;
}
