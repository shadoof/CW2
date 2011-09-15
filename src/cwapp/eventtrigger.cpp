#include "main.h"

using VRG3D::SynchedSystem;

//===========================================
void EventTrigger::init(const TiXmlElement* root)
{
  name = root->Attribute("name");

  bool enabled;
  assign(enabled, root->Attribute("enabled"));
  if (enabled) {
    flags |= IS_ACTIVE;
  }

  assign(enabled, root->Attribute("remain-enabled"));
  if (enabled) {
    flags |= IS_STAY_ACTIVE;
  }

  assign(duration, root->Attribute("duration"));

  init_tracker(Parser::get_first_element(root));
}

//===========================================
void EventTrigger::process()
{
  if (!(flags & IS_ACTIVE)) {
    return;
  }

  if (!eval()) {
    flags &= ~IS_COUNTING;
    return;
  }

  double curr_time = SynchedSystem::getAppRunningTime();

  if (!(flags & IS_COUNTING)) {
    flags |= IS_COUNTING;
    start_time = curr_time;
  } 
  
  if ((curr_time - start_time) >= duration) {
    activate();
    if (!(flags & IS_STAY_ACTIVE)) {
      flags &= ~IS_ACTIVE;
    }
    flags &= ~IS_COUNTING;
  }
}

//===========================================
void EventTrigger::activate()
{
  for (int i = 0; i < actions.size(); i++) {
    actions[i]->exec();
    if (the_app.reload_exit) {
      break;
    }
  }
}

//===========================================
void EventTrigger::init_actions(const TiXmlElement* root)
{
  Array<const TiXmlNode*> nodes;
  Parser::get_elements(nodes, root, "Actions");

  for (int i = 0; i < nodes.size(); i++) {
    const TiXmlElement* element = (const TiXmlElement*)nodes[i];

    ActionRef action = new Action();
    action->init(element, ContentRef());
    actions.append(action);
  }
}

//===========================================
EventTriggerRef EventTrigger::create_new(const TiXmlElement* root)
{
  const TiXmlElement *child = Parser::get_first_element(root);
  EventTriggerRef trigger;

  if (!child) {
    return trigger;
  }

  string name = child->ValueStr();

  if (name == "HeadTrack") {
    trigger = new HeadTrack();
  } else if (name == "MoveTrack") {
    trigger = new MoveTrack();
  }

  return trigger;
}

//===========================================
//HeadTrack
//===========================================
void HeadTrack::init_tracker(const TiXmlElement* root)
{
  const TiXmlElement* pos_elem = Parser::get_element(root, "Position");
  const TiXmlElement* child = Parser::get_first_element(pos_elem);

  string track_name = child->ValueStr();

  if (track_name == "Box") {
    Parser::get_box(child, loc_box, is_inside);
    box_used = true;
  }

  const TiXmlElement* dir_elem = Parser::get_element(root, "Direction");
  child = Parser::get_first_element(dir_elem);
  
  track_name = child->ValueStr();

  if (track_name == "PointTarget") {
    type = HEAD_POINT;
    assign(point, child->Attribute("point"));
    assign(angle, child->Attribute("angle"));
  } else if (track_name == "DirectionTarget") {
    type = HEAD_DIR;
    assign(dir, child->Attribute("direction"));
    dir.unitize();
    assign(angle, child->Attribute("angle"));
  } else if (track_name == "ObjectTarget") {
    type = HEAD_OBJ;
    string obj_name = child->Attribute("name");
    object = Story::the_story->get_object(obj_name);
  } else if (track_name == "None") {
    type = HEAD_NONE;
  } else {
    debugAssertM(0, "Invalid Tracker");
  }

  angle *= G3D::pi() / 180.f;
}

//===========================================
bool HeadTrack::eval()
{
  CoordinateFrame world_head = the_app.get_world_head_frame();

  if (box_used && (loc_box.contains(world_head.translation) != is_inside)) {
    return false;
  }

  switch (type) {
    case HEAD_NONE:
      return true;
      break;

    case HEAD_POINT:
      {
        dir = normalize(point - world_head.translation);
        float thedot = dir.dot(world_head.lookVector());
        float dir_angle = acos(thedot);
        return (dir_angle <= angle);
      }

    case HEAD_DIR:
      {
        float thedot = dir.dot(world_head.lookVector());
        float dir_angle = acos(thedot);
        return (dir_angle <= angle);
      }

    case HEAD_OBJ:
      {
        if (object.isNull()) {
          return false;
        }
        AABox box = object->get_bbox();
        Ray ray = world_head.lookRay();
        float t = ray.intersectionTime(box);
        return (isFinite(t));
      }
  }

  return false;
}

//===========================================
void HeadTrack::render(RenderDevice *rd)
{
  if (!(flags & IS_ACTIVE)) {
    return;
  }

  if (box_used) {
    Draw::box(loc_box, rd, Color4(0, 0, 0, 0), Color3::red());
  }

  CoordinateFrame world_head = the_app.get_world_head_frame();

  switch (type) {
    case HEAD_POINT:
//      rd->setColor(Color3::orange());
//      rd->setPointSize(50.0);
//      rd->beginPrimitive(RenderDevice::POINTS);
//      rd->sendVertex(point);
//      rd->endPrimitive();
      Draw::sphere(Sphere(point, 0.1), rd, Color3::orange(), Color4::clear());
      break;

    case HEAD_DIR:
      Draw::arrow(world_head.translation, dir, rd, Color3::orange(), 1.0);
      break;

    case HEAD_OBJ:
      if (object.notNull()) {
        Draw::box(object->get_bbox(), rd, Color4(0, 0, 0, 0), Color3::orange());
      }
      break;
  }
}

//===========================================
//MoveTrack
//===========================================

//===========================================
void MoveTrack::init_tracker(const TiXmlElement* root)
{
//  TiXmlElement *inside_elem = Parser::get_element(root, "Movement");
//  TiXmlElement *in_out = Parser::get_first_element(inside_elem);
//  string inside = in_out->ValueStr();
//  is_inside = (inside == "Inside");

  const TiXmlElement *source_elem = Parser::get_element(root, "Source");
  source_elem = Parser::get_first_element(source_elem);
  string source_name = source_elem->ValueStr();

  if (source_name == "ObjectRef") {
    string obj_name = source_elem->Attribute("name");
    ContentRef obj = Story::the_story->get_object(obj_name);
    group = new Group();
    if (obj.notNull()) {
      group->add(obj);
    }
  } else if (source_name == "GroupObj") {
    string group_name = source_elem->Attribute("name");
    group = Story::the_story->get_group(group_name);
    assign(is_all, source_elem->Attribute("objects"));
  }

  const TiXmlElement *box_elem = Parser::get_element(root, "Box");
  Parser::get_box(box_elem, the_box, is_inside);
}

//===========================================
bool MoveTrack::eval()
{
  if (group.isNull()) {
    return false;
  }

  for (int i = 0; i < group->get_size(); i++) {
    ContentRef obj = group->get_object(i);
    AABox bbox = obj->get_bbox();
    bool c1 = the_box.contains(bbox.low());
    bool c2 = the_box.contains(bbox.high());
    bool obj_val = ((c1 == c2) && (c1 == is_inside));
    if (!is_all || !obj_val) {
      return obj_val;
    }
  }

  return true;
}

//===========================================
void MoveTrack::render(RenderDevice *rd)
{
  if (!(flags & IS_ACTIVE)) {
    return;
  }

  Draw::box(the_box, rd, Color4(0, 0, 0, 0), Color3::green());
}
