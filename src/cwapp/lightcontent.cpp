#include "main.h"

//===========================================
void LightContent::init(const TiXmlElement* root)
{
  const TiXmlElement* elem = Parser::get_first_element(root);
  string elem_name = elem->ValueStr();

  assign(diffuse, root->Attribute("diffuse"));
  assign(specular, root->Attribute("specular"));

  assign(const_att, root->Attribute("const_atten"));
  assign(lin_att, root->Attribute("quad_atten"));
  assign(quad_att, root->Attribute("quad_atten"));

  Color3 the_color = Color3(color.r, color.g, color.b);

  if (elem_name == "Point") {
    type = POINT;
    the_light = GLight::point(placement.get_pos(), the_color, const_att, lin_att, quad_att, specular, diffuse);
  } else if (elem_name == "Directional") {
    type = DIR;
    Vector3 look = placement.get_world_frame().lookVector();

    the_light = GLight::directional(-look, the_color, specular, diffuse);
  } else if (elem_name == "Spot") {
    type = SPOT;

    assign(angle, elem->Attribute("angle"));
    //angle *= G3D::pi() / 180.f;

    //CoordinateFrame flip_frame = placement.get_world_frame();
//    flip_frame.translation = -flip_frame.translation;
//    flip_frame.translation.z = -flip_frame.translation.z;
//    flip_frame.translation.z = -flip_frame.translation.z;
//    placement.set_world_frame(flip_frame.inverse());

    Vector3 look = placement.get_world_frame().lookVector();

    //flip_frame.translation += look;
    //flip_frame.lookAt(flip_frame.translation - look);

    //flip_frame.rotation = flip_frame.rotation * Matrix3::fromAxisAngle(flip_frame.rightVector(), G3D::pi());
    //look = flip_frame.lookVector();

    //placement.set_world_frame(flip_frame);

    the_light = GLight::spot(placement.get_pos(), -look, angle, the_color, const_att, lin_att, quad_att, specular, diffuse);
  }
  valid = true;
}

//===========================================
bool LightContent::set_light(RenderDevice* dev, int index)
{
  if ((color.a == 0) || !valid) {
    the_light.enabled = false;
    return false;
  } else {
    the_light.enabled = true;

    switch (type) {
      case POINT:
        //the_light.position = Vector4(the_app.get_actual_frame().pointToObjectSpace(placement.get_pos()), 1.f);
        //the_light.position = Vector4(placement.get_pos(), 1.f);
        break;

      case DIR:
        //the_light.position = Vector4(the_app.get_actual_frame().vectorToObjectSpace(-placement.get_frame().lookVector()), 0);
        the_light.position = Vector4(-placement.get_frame().lookVector(), 0);
        break;

      case SPOT:
//        CoordinateFrame frame = the_app.get_actual_frame().inverse() * placement.get_frame();
        CoordinateFrame frame = placement.get_frame();
        the_light.position = Vector4(frame.translation, 1.f);
        //the_light.position.y = -the_light.position.y;
        the_light.spotDirection = -frame.lookVector().direction();
        //the_light.spotDirection.y = -the_light.spotDirection.y;
        break;
    }
    //the_light.position = Vector4(placement.get_pos() - the_app.get_actual_frame().translation, 1.f);
  }

  dev->enableLighting();
  dev->enableTwoSidedLighting();
  dev->setLight(index, the_light);
  return true;
}

//===========================================
void LightContent::render_derived(RenderDevice* rd)
{
  if (!Story::the_story->draw_lights) {
    return;
  }

  rd->disableLighting();

  switch (type) {
    case POINT:
      Draw::sphere(Sphere(Vector3::zero(), 0.1), rd, color, Color4::clear());
      break;

    case DIR:
      Draw::arrow(Vector3::zero(), Vector3::unitZ(), rd, color, 1);
      break;

    case SPOT:
      Draw::arrow(Vector3::zero(), Vector3::unitZ(), rd, color, 1);
      break;
  }
}
