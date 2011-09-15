#ifndef _STORY_H_
#define _STORY_H_

//===========================================
class Story : public Group
{
public:
  // Sound client
  //static SoundClientOpenAL *_defaultClient;

  Story() : parser(NULL), draw_events(false), draw_lights(false), do_coll_test(false),
                   allow_rotation(false), allow_movement(false) {}

  void init(const string& elem);
  void init(const TiXmlElement* elem);
  void init_global_settings(const TiXmlElement* root);

  void do_collision_move(const Vector3& test_pos, Vector3& change_pos, const Vector3& dir);
  void compute_total_bbox();
  void do_lighting(RenderDevice *rd);
  void render(RenderDevice *rd);

  void select_new_obj_at_pointer(const Vector3& position, const Vector3& direction);
  void check_selected_at_pointer(const Vector3& position, const Vector3& direction);
  void activate_selected();

  PlaceRef    get_place(const string& name);
  GroupRef    get_group(const string& name);
  TimelineRef get_timeline(const string& name);
  SoundRef    get_sound(const string& name);
  EventTriggerRef    get_event(const string& name);
  ParticleActionsRef get_particle_actions(const string& name);

  ContentRef get_new_content(const TiXmlElement* obj_node);

  void add_action(const ActionInstRef& action);
  void process_actions();

  void process_timelines();
  void process_events();

  Parser* get_parser()                         { return parser; }

  void set_filename (const string& file)  { filename = file; }

  void set_do_coll_test () { do_coll_test = true; }

  static string resolve_rel_path(const string& name);

  static StoryRef the_story;


private:
  ContentRef selected_obj;

  Array<GroupRef> groups;
  Array<TimelineRef> timelines;
  Array<PlaceRef> placements;
  Array<SoundRef> sounds;
  Array<EventTriggerRef> events;
  Array<LightContentRef> lights;

  Array<ParticleActionsRef> particle_actions;

  Array<ActionInstRef> action_insts;

  string filename;
  AABox total_bbox;

  Parser* parser;

public:
  bool draw_events;
  bool draw_lights;
  bool do_coll_test;
  bool allow_rotation;
  bool allow_movement;
};

#endif
