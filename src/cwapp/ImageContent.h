#ifndef _IMAGE_CONTENT_H_
#define _IMAGE_CONTENT_H_

class ImageContent : public Content
{
public:
  ImageContent() : width(0), height(0), height_ratio(0), is_stereo(false) {}

  virtual void init(const TiXmlElement* root);
  virtual void render_derived(RenderDevice* dev);

private:
  TextureRef left;
  TextureRef right;
  bool is_stereo;

  Array<Vector3> verts;
  Array<Vector2> coords;

  float width, height, height_ratio;
};


#endif
