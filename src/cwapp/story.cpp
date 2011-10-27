#include "main.h"

//===========================================
//Story
//===========================================

StoryRef Story::the_story;

/*SoundClientOpenAL *Story::_defaultClient;*/

//===========================================
void Story::init(const string& file)
{
  filename = file;

  // Make sure all slashes are forward slashes
  for (string::iterator it = filename.begin(); it != filename.end(); it++)
  {
    if (*it == '\\')
    {
      *it = '/';
    }
  }

  parser = new Parser();
  parser->init(filename);

  init(parser->get_root());

  delete parser;
}

//===========================================
void Story::init(const TiXmlElement* root)
{
  if (!root) {
    return;
  }

  init_global_settings(parser->get_element(root, "Global"));

  Array<const TiXmlNode*> nodes;
  const TiXmlElement *new_root = NULL;

  new_root = parser->get_element(root, "PlacementRoot");
  parser->get_elements(nodes, new_root, "Placement");

  for (int i = 0; i < nodes.size(); i++) {
    PlaceRef obj;
    obj = new Placement();
    obj->init((TiXmlElement*)nodes[i]);
    placements.append(obj);
  }

  new_root = parser->get_element(root, "SoundRoot");
  parser->get_elements(nodes, new_root, "Sound");

  for (int i = 0; i < nodes.size(); i++) {
    SoundRef obj;
    obj = new SoundObject();
    obj->init((TiXmlElement*)nodes[i]);
    sounds.append(obj);
  }

  Array<const TiXmlNode*> particle_nodes;
  new_root = parser->get_element(root, "ParticleActionRoot");
  parser->get_elements(particle_nodes, new_root, "ParticleActionList");

  ParticleActions::create_lists(particle_nodes.length());

  for (int i = 0; i < particle_nodes.length(); i++) {
    ParticleActionsRef obj;
    obj = new ParticleActions();
    obj->init((const TiXmlElement*)particle_nodes[i]);
    particle_actions.append(obj);
  }
  
  //ParticleSystem::create_systems(0);

  Array<const TiXmlNode*> obj_nodes;
  new_root = parser->get_element(root, "ObjectRoot");
  parser->get_elements(obj_nodes, new_root, "Object");

  for (int i = 0; i < obj_nodes.size(); i++) {
    ContentRef obj;
    obj = get_new_content((const TiXmlElement*)obj_nodes[i]);
    objects.append(obj);

    if (obj->get_light()) {
      lights.append(obj->get_light());
    }
  }

  new_root = parser->get_element(root, "GroupRoot");
  parser->get_elements(nodes, new_root, "Group");

  for (int i = 0; i < nodes.size(); i++) {
    GroupRef obj;
    obj = new Group();
    obj->init((const TiXmlElement*)nodes[i]);
    groups.append(obj);
  }

  for (int i = 0; i < groups.size(); i++) {
    groups[i]->resolve_group_refs((const TiXmlElement*)nodes[i]);
  }

  Array<const TiXmlNode*> time_nodes;
  new_root = parser->get_element(root, "TimelineRoot");
  parser->get_elements(time_nodes, new_root, "Timeline");

  for (int i = 0; i < time_nodes.size(); i++) {
    TimelineRef obj;
    obj = new Timeline();
    obj->init((const TiXmlElement*)time_nodes[i]);
    timelines.append(obj);
  }

  Array<const TiXmlNode*> event_nodes;
  new_root = parser->get_element(root, "EventRoot");
  parser->get_elements(event_nodes, new_root);

  for (int i = 0; i < event_nodes.size(); i++) {
    EventTriggerRef obj;
    obj = EventTrigger::create_new((const TiXmlElement*)event_nodes[i]);
    obj->init((const TiXmlElement*)event_nodes[i]);
    events.append(obj);
  }

  //Second Pass Init

  //Objects
  for (int i = 0; i < obj_nodes.size(); i++) {
    objects[i]->init_actions((TiXmlElement*)obj_nodes[i]);
  }

  //Timelines
  for (int i = 0; i < time_nodes.size(); i++) {
    timelines[i]->init_actions((TiXmlElement*)time_nodes[i]);
  }

  //Events
  for (int i = 0; i < event_nodes.size(); i++) {
    events[i]->init_actions((TiXmlElement*)event_nodes[i]);
  }
 
  // Preload sound files
  //bool success = SOUNDCLIENT->preload();
  //debugAssertM(success, "Sounds Failed to Load");
  // Reset the listener position
  //SOUNDCLIENT->setListenerLoc(CoordinateFrame());
  SOUNDCLIENT->load_all();
  SOUNDCLIENT->set_listener(the_app.get_cam_frame(), Vector3::zero());
}

//===========================================
void Story::init_global_settings(const TiXmlElement* root)
{
  if (!root) {
    return;
  }


  string elem_name;

  if (!the_app.is_caved_in()) {
    elem_name = "CameraPos";
  } else {
    elem_name = "CaveCameraPos";
  }

  const TiXmlElement* cam_elem = parser->get_element(root, elem_name);
  Placement cam_place;

  if (cam_elem) {
    cam_place.init(cam_elem->FirstChildElement("Placement"));
    CoordinateFrame frame = cam_place.get_world_frame();
    the_app.set_cam_frame(frame);

    double far_clip = 0;
    assign(far_clip, cam_elem->Attribute("far-clip"));
    the_app.update_far_clip(far_clip);
    
  } else {
    the_app.set_cam_frame(CoordinateFrame());
  }

  const TiXmlElement* elem = parser->get_element(root, "Background");
  if (elem) {
    Color3 back_color;
    assign(back_color, elem->Attribute("color"));
    the_app.set_background_color(back_color);
  }

  elem = parser->get_element(root, "WandNavigation");
  if (elem) {
    assign(allow_movement, elem->Attribute("allow-movement"));
    assign(allow_rotation, elem->Attribute("allow-rotation"));
  }
}

//===========================================
ContentRef Story::get_new_content(const TiXmlElement* obj_node)
{
  ContentRef content;

  const TiXmlElement* content_elem = parser->get_element(obj_node, "Content");
  if (!content_elem) {
    return content;
  }

  const TiXmlElement* child = parser->get_first_element(content_elem);
  if (!child) {
    return content;
  }

  string content_type = child->ValueStr();

  if (content_type == "Text") {
    content = new TextContent();
  } else if (content_type == "Image") {
    content = new ImageContent();
  } else if (content_type == "StereoImage") {
    content = new ImageContent();
  } else if (content_type == "Model") {
    content = new ModelContent();
  } else if (content_type == "None") {
    content = new TextContent();
  } else if (content_type == "Light") {
    content = new LightContent();
  } else if (content_type == "ParticleSystem") {
    content = new ParticleSystem();
  } else if (content_type == "Shader") {
    content = new ShaderContent();
  }
//  } else if(content_type == "Sound") {
//	  content = new SoundContent();
//  }

  if (content.isNull()) {
    msgBox("Content of Type \"" + content_type + "\" Not Supported");
  } else {
    content->init(obj_node, content_type);
  }

  return content;
}

//===========================================
void Story::do_collision_move(const Vector3& test_pos, Vector3& change_pos, const Vector3& dir)
{
  if (!do_coll_test) {
    change_pos += dir;
    return;
 }
 
  Vector3 pos = test_pos;
  Vector3 min_diff = dir;
  float mag;

  for (int i = 0; i < objects.length(); i++) {
    if (objects[i]->get_content() == "Model") {
      ModelContent *model = (ModelContent*)objects[i].pointer();
      if (model->do_collision(pos, mag, dir)) {
         Vector3 diff = pos - test_pos;
         if (diff.length() < min_diff.length()) {
           min_diff = diff;
         }
      }
    }
  }
  
  change_pos += min_diff;
}

//===========================================
void Story::compute_total_bbox()
{
  AABox box = objects[0]->get_bbox();
  total_bbox.set(total_bbox.low().min(box.low()), total_bbox.high().max(box.high()));
  for (int i = 0; i < objects.length(); i++) {
    box = objects[i]->get_bbox();
    total_bbox.set(total_bbox.low().min(box.low()), total_bbox.high().max(box.high()));
  }
}

//===========================================
void Story::do_lighting(RenderDevice *rd)
{
  if (!lights.size()) {
    GLight light = GLight::point(Vector3(0, 0, 4), Color3::white());
    rd->setLight(0, light);
  }

  int counter = 0;

  for (int i = 0; i < lights.size(); i++) {
    if (lights[i]->set_light(rd, counter)) {
      counter++;
      if (counter >= RenderDevice::MAX_LIGHTS) {
        break;
      }
    }
  }
}

//===========================================
void Story::render(RenderDevice *rd)
{
  for (int i = 0; i < objects.size(); i++) {
    objects[i]->render(rd);
  }

  if (draw_events) {
    for (int i = 0; i < events.size(); i++) {
      events[i]->render(rd);
    }
  }
}

//===========================================
void Story::select_new_obj_at_pointer(const Vector3& position, const Vector3& direction)
{
  Ray ray = Ray::fromOriginAndDirection(position, direction);
  float min_dist = G3D::inf();
  float max_dist = G3D::inf();

  if (selected_obj.notNull()) {
    selected_obj->select(false);
    selected_obj = ContentRef();
  }

  for (int i = 0; i < objects.size(); i++) {
    float curr_t = ray.intersectionTime(objects[i]->get_bbox());
    if (!isFinite(curr_t) || (curr_t < 0)) {
      continue;
    }

    if (objects[i]->is_click_stop() && (curr_t <= min_dist) && (objects[i]->get_alpha() > .75)) {
      min_dist = curr_t;
      selected_obj = objects[i];
    }
  }

  if (selected_obj.notNull() && selected_obj->is_active_link()) {
    selected_obj->select(true);
  }
}

//===========================================
void Story::check_selected_at_pointer(const Vector3& position, const Vector3& direction)
{
  if (selected_obj.isNull()) {
    return;
  }

  Ray ray = Ray::fromOriginAndDirection(position, direction);

  float curr_t = ray.intersectionTime(selected_obj->get_bbox());

  if (isFinite(curr_t) && (curr_t > 0)) {
    selected_obj->select(true);
  } else {
    selected_obj->select(false);
  }
}

//===========================================
void Story::activate_selected()
{
  if (selected_obj.isNull()) {
    return;
  }

  if (selected_obj->is_selected()) {
    selected_obj->activate();
    selected_obj->select(false);
  }

  selected_obj = ContentRef();
}

//===========================================
PlaceRef Story::get_place(const string& name)
{
  for (int i = 0; i < placements.size(); i++) {
    if (placements[i]->get_name() == name) {
      return placements[i];
    }
  }

  return PlaceRef();
}

//===========================================
SoundRef Story::get_sound(const string& name)
{
  for (int i = 0; i < sounds.size(); i++) {
    if (sounds[i]->get_name() == name) {
      return sounds[i];
    }
  }

  return SoundRef();
}

//===========================================
EventTriggerRef Story::get_event(const string& name)
{
  for (int i = 0; i < events.size(); i++) {
    if (events[i]->get_name() == name) {
      return events[i];
    }
  }

  return EventTriggerRef();
}

//===========================================
ParticleActionsRef Story::get_particle_actions(const string& name)
{
  for (int i = 0; i < particle_actions.size(); i++) {
    if (particle_actions[i]->get_name() == name) {
      return particle_actions[i];
    }
  }

  return ParticleActionsRef();
}

//===========================================
GroupRef Story::get_group(const string& name)
{
  for (int i = 0; i < groups.size(); i++) {
    if (groups[i]->get_name() == name) {
      return groups[i];
    }
  }

  return GroupRef();
}

//===========================================
TimelineRef Story::get_timeline(const string& name)
{
  for (int i = 0; i < timelines.size(); i++) {
    if (timelines[i]->get_name() == name) {
      return timelines[i];
    }
  }

  return TimelineRef();
}

//===========================================
void Story::add_action(const ActionInstRef& action)
{
  action_insts.append(action);
}

//===========================================
void Story::process_actions()
{
  for (int i = 0; i < action_insts.size(); i++) {
    if (action_insts[i].notNull()) {
      if (action_insts[i]->process()) {
        action_insts[i] = ActionInstRef();
      }
    }
  }
}

//===========================================
void Story::process_timelines()
{
  for (int i = 0; i < timelines.size(); i++) {
    timelines[i]->process();
  }
}

//===========================================
void Story::process_events()
{
  for (int i = 0; i < events.size(); i++) {
    events[i]->process();
  }
}

//===========================================
string Story::resolve_rel_path(const string& name)
{
  string full;

  if (beginsWith(name, "file:")) {
    full = name.substr(5);
  } else if (name[0] == '.') {
    string path_name = the_story->filename;

    // Path has been normalized to contain only forward /

    size_t index = path_name.rfind('/');
    if (index != string::npos) {
      full = path_name.substr(0, index);
      full += "/";
    }
    full += name;
  } else {
    full = name;
  }

  return full;
}

