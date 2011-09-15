#include "main.h"

//===========================================
//ShaderContent
//===========================================
void ShaderContent::init(const TiXmlElement* root)
{
  string name = root->Value();

  float width = 1.f, height = 1.f;

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

  vertex_file = "std.vp";
  pixel_file = "particle.fp";

  Texture::Settings settings;
  settings.wrapMode = G3D::WrapMode::CLAMP;
  settings.interpolateMode = Texture::BILINEAR_NO_MIPMAP;

  tex0 = Texture::fromFile("Flame.tga", TextureFormat::AUTO(), Texture::DIM_2D, settings);

  shader = Shader::fromFiles(vertex_file, pixel_file);
  shader->args.set("tex0", tex0);
  shader->args.set("age", 1.0);

  valid = true;
}

//===========================================
void ShaderContent::render_derived(RenderDevice* RD)
{
  if (shader.notNull()) {
    shader->args.set("tex0", tex0);
    RD->setShader(shader);
  }

  RD->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, 
    RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA, 
    RenderDevice::BLENDEQ_ADD);

  RD->setDepthTest(RenderDevice::DEPTH_ALWAYS_PASS);


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
}