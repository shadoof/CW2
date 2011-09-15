#include "main.h"

//===========================================
//TextContent
//===========================================
void ImageContent::init(const TiXmlElement* root)
{
  string name = root->ValueStr();

  if (name == "Image") {
    string left_name = root->Attribute("filename");
    left_name = Story::resolve_rel_path(left_name);


    try {
      left = the_app.tex_mgr.loadTexture(left_name);
      if (left.isNull()) {
        msgBox("Image file \"" + left_name + "\" was not found");
        return;
      }
    } catch (GImage::Error& err) {
      left = NULL;
      msgBox("Image file \"" + left_name + "\" is invalid and could not be loaded");
      return;
    }

    is_stereo = false;

    width = left->texelWidth();
    height = left->texelHeight();

  } else if (name == "StereoImage") {
    string left_name = root->Attribute("left-image");
    left_name = Story::resolve_rel_path(left_name);

    try {
      left = the_app.tex_mgr.loadTexture(left_name);
      if (left.isNull()) {
        msgBox("Image file \"" + left_name + "\" was not found");
        return;
      }
    } catch (GImage::Error& err) {
      left = NULL;
      msgBox("Image file \"" + left_name + "\" is invalid and could not be loaded");
      return;
    }

    width = left->texelWidth();
    height = left->texelHeight();

    string right_name = root->Attribute("right-image");
    right_name = Story::resolve_rel_path(right_name);

    try {
      right = the_app.tex_mgr.loadTexture(right_name);
      if (right.isNull()) {
        msgBox("Image file \"" + right_name + "\" was not found");
        return;
      }
    } catch (GImage::Error& err) {
      right = NULL;
      msgBox("Image file \"" + right_name + "\" is invalid and could not be loaded");
      return;
    }


    is_stereo = true;

    if (right->texelWidth() != width) {
      msgBox("Width of \"" + left_name + "\" does not match width of \"" + right_name + "\".\nThe widths of left and right image must be the same");
      return;
    }

    if (right->texelHeight() != height) {
      msgBox("Height of \"" + left_name + "\" does not match height of \"" + right_name + "\".\nThe heights of left and right image must be the same");
      return;
    }
  } else {
    alwaysAssertM(0, "Invalid Image Element");
  }

   //scale height to image proportions
  height_ratio = height / width;

  //scale *= 2.f;
  float max_scale = 1.f;

  if (height > width) {
    width *= (max_scale / height);
    height = max_scale;
  } else {
    height *= (max_scale / width);
    width = max_scale;
  }

//  float width = scale;
//  float height = height_ratio * scale;

  float min_x = -width * .5;
  float max_x = width * .5;
  float min_y = -height * .5;
  float max_y = height * .5;

  local_bbox.set(Vector3(min_x, min_y, 0), Vector3(max_x, max_y, 0.02));
  update_bbox();

  verts.append(Vector3(min_x, max_y, 0));
  verts.append(Vector3(min_x, min_y, 0));
  verts.append(Vector3(max_x, min_y, 0));
  verts.append(Vector3(max_x, max_y, 0));

  coords.append(Vector2(0, 0));
  coords.append(Vector2(0, 1));
  coords.append(Vector2(1, 1));
  coords.append(Vector2(1, 0));

  valid = true;
}


//===========================================
void ImageContent::render_derived(RenderDevice* RD)
{
  bool is_right = false;

  if (is_stereo) {
    GLint val = glGetInteger(GL_DRAW_BUFFER);
    is_right = (val == GL_BACK_RIGHT) || (val == GL_FRONT_RIGHT);
  }

  if (is_right) {
    RD->setTexture(0, right);
  } else {
    RD->setTexture(0, left);
  }

  RD->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, 
    RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA, 
    RenderDevice::BLENDEQ_ADD);


  //RD->setDepthTest(RenderDevice::DEPTH_ALWAYS_PASS);
  /*
  RD->setCullFace(RenderDevice::CULL_NONE);
  RD->setDepthTest(RenderDevice::DEPTH_ALWAYS_PASS); 
  RD->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, 
  RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA, 
  RenderDevice::BLENDEQ_ADD);
  //RD->setBlendFunc(RenderDevice::RenderDevice::BLEND_ONE, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
  */

  RD->beginPrimitive(RenderDevice::QUADS);

  RD->setTexCoord(0, coords[0]);
  RD->setNormal(Vector3::unitZ());
  RD->sendVertex(verts[0]);
  RD->setTexCoord(0, coords[1]);
  RD->setNormal(Vector3::unitZ());
  RD->sendVertex(verts[1]);
  RD->setTexCoord(0, coords[2]);
  RD->setNormal(Vector3::unitZ());
  RD->sendVertex(verts[2]);
  RD->setTexCoord(0, coords[3]);
  RD->setNormal(Vector3::unitZ());
  RD->sendVertex(verts[3]);

  RD->endPrimitive();

  RD->setTexture(0, NULL);
}