#ifndef _MODEL_CONTENT_H_
#define _MODEL_CONTENT_H_

#include "StudioModel.h"

class ModelContent : public Content
{
public:
  ModelContent() :do_collis(false) {}

  virtual void init(const TiXmlElement* root);
  virtual void render_derived(RenderDevice* dev);

  StudioModelRef get_model() const { return model; }

  bool do_collision(Vector3& pos, float& mag, const Vector3& dir) const;

private:
  StudioModelRef model;
  bool do_collis;
};

#endif
