#ifndef _LIGHT_CONTENT_H_
#define _LIGHT_CONTENT_H_

#include "content.h"

class LightContent : public Content
{
public:
  enum LightType
  {
    POINT,
    DIR,
    SPOT,
  };

  LightContent(void) : type(POINT), index(-1), angle(0), specular(true), diffuse(true) {}
  virtual ~LightContent(void) {}

  virtual void init(const TiXmlElement* root);
  virtual void render_derived(RenderDevice* dev);

  bool set_light(RenderDevice* dev, int index);

  LightContent* get_light() { return this; }


private:
  GLight the_light;
  LightType type;
  bool specular, diffuse;
  float angle;
  int index;

  float const_att, lin_att, quad_att;
};

#endif
