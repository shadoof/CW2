#include "main.h"

//===========================================
void ModelContent::init(const TiXmlElement* root)
{
  if (the_app.var_static.isNull()) {
    return;
  }

//  scale = scale * scale;
//  update_bbox();


  string modelname = root->Attribute("filename");

  assign(do_collis, root->Attribute("check-collisions"));
  if (do_collis) {
    Story::the_story->set_do_coll_test();
  }

  modelname = Story::resolve_rel_path(modelname);

  int last_dir = modelname.find_last_of("/");
  std::string basedir, file;

  if (last_dir >= 0) {
	  basedir = modelname.substr(0, last_dir + 1);
	  file = modelname.substr(last_dir + 1);
  } else {
	  basedir = "./";
	  file = modelname;
  }

  if (!G3D::fileExists(modelname)) {
    msgBox("The model file \"" + modelname + "\" could not be found", "Model Error");
    return;
  }

	cout << "BaseDir " << basedir << endl << "Model " << file << endl;

  try {
	  model = new StudioModel(basedir, file, scale * scale, true, do_collis);
    scale = 1.f;
  } catch (...) {
    model = NULL;
    msgBox("There was an error loading model \"" + modelname + "\". It will not be displayed");
    return;
  }
	model->getObjectSpaceBoundingBox(local_bbox);
  update_bbox();

  if (model.notNull()) {
    valid = true;
  }
}

//===========================================
bool ModelContent::do_collision(Vector3& pos, float& mag, const Vector3& dir) const
{
  if (!do_collis || (color.a == 0.0)) {
    return false;
  }

  Vector3 slide_vec;

  Vector3 real_pos, real_dir;

  CoordinateFrame frame = placement.get_frame();
  real_pos = frame.pointToObjectSpace(pos);
  real_dir = frame.vectorToObjectSpace(dir);

  //Vector3 orig_pos = real_pos;

  real_pos = real_pos / scale;
  real_dir = real_dir / scale;
  
  if (model->slideCollision(real_pos, 2, real_dir, slide_vec)) {
    real_pos += slide_vec;
    real_pos = real_pos * scale;
    pos = frame.pointToWorldSpace(real_pos);
    return true;
  }

//  mag = (real_pos - last).length();


  
//  cout << "real_dir: " << real_dir << endl;

//  frame.translation = Vector3::zero();
//  frame.rotation = Matrix3::identity() * (scale);
//  pos = frame.pointToWorldSpace(pos);

  return false;
}

//===========================================
void ModelContent::render_derived(RenderDevice* RD)
{
  glEnable(GL_TEXTURE_2D);
  
  if (!fuzzyEq(color.a, 0.0) && !fuzzyEq(color.a, 1.0)) {
    RD->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, 
    RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA, 
    RenderDevice::BLENDEQ_ADD);
  }

  if (is_selected()) {
    model->render(RD, RD->color());
  } else {
    model->render(RD, color.a);
  }
  glDisable(GL_TEXTURE_2D);
}
