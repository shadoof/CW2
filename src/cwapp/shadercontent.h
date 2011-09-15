#ifndef _SHADER_CONTENT_H_
#define _SHADER_CONTENT_H_

class ShaderContent : public Content
{
public:
  ShaderContent() {}

  virtual void init(const TiXmlElement* root);
  virtual void render_derived(RenderDevice* dev);

private:
  TextureRef tex0, tex1;

  Array<Vector3> verts;
  Array<Vector2> coords;

  string vertex_file;
  string pixel_file;
};


#endif
