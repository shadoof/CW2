#include "main.h"

//===========================================
//Content
//===========================================
void Content::init(const TiXmlElement* root, const string& type)
{
  content_type = type;
  link_counter = 1;

  name = root->Attribute("name");

  bool visible;
  assign(visible, Parser::get_element_value(root, "Visible"));
  selected_color.a = active_color.a = color.a = (visible ? 1.f : 0.f);

  bool lighting;
  assign(lighting, Parser::get_element_value(root, "Lighting"));
  if (lighting) {
    flags |= IS_USE_LIGHTING;
  }

  bool click_through;
  assign(click_through, Parser::get_element_value(root, "ClickThrough"));
  if (!click_through) {
    flags |= IS_CLICK_STOP;
  }

  bool around_axis;
  assign(around_axis, Parser::get_element_value(root, "AroundSelfAxis"));
  if (around_axis) {
    flags |= IS_AROUND_SELF_AXIS;
  }


  assign(color, Parser::get_element_value(root, "Color"));
  assign(scale, Parser::get_element_value(root, "Scale"));

  string sound_name = Parser::get_element_value(root, "SoundRef");
  sound = Story::the_story->get_sound(sound_name);

  placement.init(Parser::get_element(root, "Placement"));

  const TiXmlElement* linkRoot = Parser::get_element(root, "LinkRoot");

  const TiXmlElement* link_node = (linkRoot ? Parser::get_element(linkRoot, "Link") : NULL);
  
  if (link_node) {
    assign(active_color, Parser::get_element_value(link_node, "EnabledColor"));
    assign(selected_color, Parser::get_element_value(link_node, "SelectedColor"));

    bool active;
    assign(active, Parser::get_element_value(link_node, "Enabled"));

    bool remain;
    assign(remain, Parser::get_element_value(link_node, "RemainEnabled"));

    if (active) {
      flags |= IS_ACTIVE;
    }
    if (remain) {
      flags |= IS_REMAIN_ACTIVE;
    }
  }

  const TiXmlElement* content = Parser::get_element(root, "Content");

  //active_color.a = selected_color.a = 1.f;
  init(Parser::get_element(content, content_type));
}

//===========================================
void Content::init_actions(const TiXmlElement* root)
{
  const TiXmlElement* linkRoot = Parser::get_element(root, "LinkRoot");
  const TiXmlElement* link_node = (linkRoot ? Parser::get_element(linkRoot, "Link") : NULL);

  if (!link_node) {
    return;
  }

  Array<const TiXmlNode*> nodes;
  Parser::get_elements(nodes, link_node, "Actions");

  for (int i = 0; i < nodes.size(); i++) {
    TiXmlElement* element = (TiXmlElement*)nodes[i];

    ActionRef action = new Action();
    action->init(element, this);
    actions.append(action);

    const TiXmlElement* click_element = Parser::get_element(element, "Clicks");

    if (click_element) {
      click_element = Parser::get_element(click_element, "NumClicks");
    }

    unsigned int num = 0;
    bool is_reset = false;

    if (click_element) {
      string value = click_element->Attribute("num_clicks");
      if (value.length()) {
        assign(num, value);
      }
      assign(is_reset, click_element->Attribute("reset"));
    }

    if (is_reset) {
      num |= LINK_RESET;
    }
    click_nums.append(num);
  }
}

//===========================================
void Content::play_sound()
{
  if (sound.notNull()) {
    sound->play(placement.get_pos());
  }
}

//===========================================
void Content::update_bbox()
{
  Vector3 lower = local_bbox.low() * scale;
  Vector3 upper = local_bbox.high() * scale;

  CoordinateFrame frame = placement.get_frame();
  lower = frame.pointToWorldSpace(lower);
  upper = frame.pointToWorldSpace(upper);
  Vector3 temp = G3D::min(lower, upper);
  upper = G3D::max(lower, upper);
  lower = temp;

  bounding_box.set(lower, upper);
}

//===========================================
void Content::activate()
{
  if (!(flags & IS_REMAIN_ACTIVE)) {
    flags &= ~IS_ACTIVE;
  }

  bool reset_counter = false;

  for (int i = 0; i < actions.size(); i++) {
    if (click_nums[i]) {
      if ((click_nums[i] & ~LINK_RESET) != link_counter) {
        continue;
      }
      if (click_nums[i] & LINK_RESET) {
        reset_counter = true;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
      }
    }
    actions[i]->exec();
    if (the_app.reload_exit) {
      break;
    }
  }

  if (reset_counter) {
    link_counter = 1;
  } else {
    link_counter++;
  }
}

//===========================================
void Content::begin_interp(int type, const ActionRef& action)
{
  switch (type) {
    case Action::TRANS_VISIBLE:
      start_alpha = color.a;
      break;

    case Action::TRANS_COLOR:
      start_color = color;
      break;

    case Action::TRANS_MOVE:
      start_frame = placement.get_frame();
      end_frame = action->placement.get_frame();
//    move_vel = Placement::compute_vel(end_frame, start_frame, action->duration);
      if (sound.notNull()) {
        SOUNDCLIENT->start_interp(sound->get_index(), start_frame.translation, end_frame.translation, action->duration);
      }
      break;

    case Action::TRANS_MOVE_REL:
      start_frame = placement.get_frame();
      //end_frame = placement.get_relative(action->placement);
      if (flags & IS_AROUND_SELF_AXIS) {
        end_frame = start_frame * action->placement.get_world_frame();
      } else {
        end_frame = action->placement.get_world_frame() * start_frame;
      }

//    move_vel = Placement::compute_vel(end_frame, start_frame, action->duration);
      if (sound.notNull()) {
        SOUNDCLIENT->start_interp(sound->get_index(), start_frame.translation, end_frame.translation, action->duration);
      }
      break;

    case Action::TRANS_SCALE:
      start_scale = scale;
      break;

    case Action::TRANS_LINK:
      select(true);
      break;
  }
}

//===========================================
void Content::interp_alpha(const float& end_alpha, float elapsed)
{
  selected_color.a = active_color.a = color.a = ((1.f - elapsed) * start_alpha) + (elapsed * end_alpha);
}

//===========================================
void Content::interp_frame(const CoordinateFrame& end_frame, float elapsed)
{
  placement.set_world_frame(start_frame.lerp(end_frame, elapsed));
//  if (sound.notNull()) {
//    sound->set_pos_vel(placement.get_total_pos(), (elapsed < 1.f) ? move_vel : Vector3::zero());
//  }
  update_bbox();
}

//===========================================
void Content::interp_color(const Color4& end_color, float elapsed)
{
  float inv_elapse = (1.f - elapsed);
  color.r = (inv_elapse * start_color.r) + (elapsed * end_color.r);
  color.g = (inv_elapse * start_color.g) + (elapsed * end_color.g);
  color.b = (inv_elapse * start_color.b) + (elapsed * end_color.b);
}

//===========================================
void Content::interp_scale(const float& end_scale, float elapsed)
{
  scale = ((1.f - elapsed) * start_scale) + (elapsed * end_scale);
}

//===========================================
void Content::render(RenderDevice *RD)
{
  if ((color.a == 0) || !valid) {
    return;
  }

  RD->pushState();

  CoordinateFrame current = RD->getObjectToWorldMatrix();
  current = placement.apply(current);
  current.rotation = current.rotation * scale;
  RD->setObjectToWorldMatrix(current);

  if (is_selected()) {
    RD->setColor(selected_color);
  } else if (is_active_link()) {
    RD->setColor(active_color);
  } else {
    RD->setColor(color);
  }

  render_direct(RD);

  RD->popState();

  if (the_app.is_draw_bbox()) {
    Draw::box(bounding_box, RD, Color4::clear(), Color3::red());
  }
}

//===========================================
void Content::render_direct(RenderDevice *RD)
{
  RD->setShadeMode(RenderDevice::SHADE_SMOOTH);
  RD->setCullFace(G3D::RenderDevice::CULL_NONE);

  if (use_lighting()) {
    RD->enableLighting();
    RD->enableTwoSidedLighting();
  } else {
    RD->disableLighting();
  }

  render_derived(RD);
}
