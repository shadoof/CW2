#include "main.h"
#include <GL/glut.h>

//===========================================
//ParticleDomain
//===========================================
int ParticleActions::next_action_list = 0;
int ParticleSystem::next_group_id = 0;

using namespace PAPI;

ParticleContext_t P;

#define GET_VEC(X)   get_vec(dom_elem, X)
#define GET_FLOAT(X) get_float(dom_elem, X)

ParticleDomain::ParticleDomain(const TiXmlElement* root, string elem_name)
{
  if (!elem_name.length()) {
    elem_name = "ParticleDomain";
  }

  const TiXmlElement* dom_elem = Parser::get_element(root, elem_name);
  dom_elem = Parser::get_first_element(dom_elem);
  name = dom_elem->ValueStr();

  if (name == "Point") {
    domain = new PDPoint(GET_VEC("point"));
  } else if (name == "Line") {
    domain = new PDLine(GET_VEC("p1"), GET_VEC("p2"));
  } else if (name == "Triangle") {
    domain = new PDRectangle(GET_VEC("p1"), GET_VEC("p2"), GET_VEC("p3"));
  } else if (name == "Plane") {
    domain = new PDPlane(GET_VEC("point"), GET_VEC("normal"));
  } else if (name == "Rect") {
    domain = new PDRectangle(GET_VEC("point"), GET_VEC("u-dir"), GET_VEC("v-dir"));
  } else if (name == "Box") {
    domain = new PDBox(GET_VEC("p1"), GET_VEC("p2"));
  } else if (name == "Sphere") {
    domain = new PDSphere(GET_VEC("center"), GET_FLOAT("radius"), GET_FLOAT("radius-inner"));
  } else if (name == "Cylinder") {
    domain = new PDCylinder(GET_VEC("p1"), GET_VEC("p2"), GET_FLOAT("radius"), GET_FLOAT("radius-inner"));
  } else if (name == "Cone") {
    domain = new PDCone(GET_VEC("apex"), GET_VEC("base-center"), GET_FLOAT("radius"), GET_FLOAT("radius-inner"));
  } else if (name == "Blob") {
    domain = new PDBlob(GET_VEC("center"), GET_FLOAT("stdev"));
  } else if (name == "Disc") {
    domain = new PDDisc(GET_VEC("center"), GET_VEC("normal"), GET_FLOAT("radius"), GET_FLOAT("radius-inner"));
  } else {
    domain = new PDPoint(pVec(0.f, 0.f, 0.f));
    alwaysAssertM(0, "Invalid Particle Domain");
  }
}

//===========================================
pVec ParticleDomain::get_vec(const TiXmlElement* root, const string& name)
{
  pVec vec;
  assign(vec, root->Attribute(name.c_str()));
  return vec;
}

//===========================================
float ParticleDomain::get_float(const TiXmlElement* root, const string& name)
{
  if (!root) {
    return 0;
  }

  float val;
  assign(val, root->Attribute(name.c_str()));
  return val;
}

//===========================================
//ParticleActions
//===========================================
void ParticleActions::create_lists(int num)
{
  next_action_list = P.GenActionLists(num);
}

//===========================================
void ParticleActions::init(const TiXmlElement* root)
{
  action_id = next_action_list++;

  name = root->Attribute("name");

  Array<const TiXmlNode*> nodes;
  Parser::get_elements(nodes, root, "ParticleAction");

  ParticleCommand cmd;
  memset(&cmd, 0, sizeof(ParticleCommand));

  P.NewActionList(action_id);

  //SOURCE ACTION

  {
    const TiXmlElement *child = NULL;

    child = Parser::get_element(root, "Vel");
    cmd.domain = new ParticleDomain(child);
    P.Velocity(cmd.domain->get_domain());

    child = Parser::get_element(root, "Source");
    cmd.domain = new ParticleDomain(child);
    assign(cmd.mag, child->Attribute("rate"));

    P.Source(cmd.mag, cmd.domain->get_domain());

    //commands.append(cmd);
  }

  //ACTION LIST

  for (int i = 0; i < nodes.length(); i++) {
    const TiXmlElement* child = Parser::get_first_element(nodes[i]);
    string name = child->ValueStr();


    if (name == "Avoid") {
      assign(cmd.mag, child->Attribute("magnitude"));
      assign(cmd.eps, child->Attribute("epsilon"));
      assign(cmd.lookahead, child->Attribute("lookahead"));
      cmd.domain = new ParticleDomain(child);

      P.Avoid(cmd.mag, cmd.eps, cmd.lookahead, cmd.domain->get_domain());

    } else if (name == "Bounce") {
      assign(cmd.mag, child->Attribute("friction"));
      assign(cmd.lookahead, child->Attribute("resilience"));
      assign(cmd.eps, child->Attribute("cutoff"));
      cmd.domain = new ParticleDomain(child);

      P.Bounce(cmd.mag, cmd.lookahead, cmd.eps, cmd.domain->get_domain());

    } else if (name == "Gravity") {
      assign(cmd.point, child->Attribute("direction"));

      P.Gravity(cmd.point);

    } else if (name == "Damping") {
      assign(cmd.point, child->Attribute("direction"));
      
      assign(cmd.eps, child->Attribute("vel_low"));
      assign(cmd.lookahead, child->Attribute("vel_high"));
      if (!cmd.lookahead) {
        cmd.lookahead = P_MAXFLOAT;
      }

      P.Damping(cmd.point, cmd.eps, cmd.lookahead);

    } else if (name == "Gravitate") {
      assign(cmd.mag, child->Attribute("magnitude"));
      assign(cmd.eps, child->Attribute("epsilon"));
      assign(cmd.lookahead, child->Attribute("max_radius"));
      if (!cmd.lookahead) {
        cmd.lookahead = P_MAXFLOAT;
      }

      P.Gravitate(cmd.mag, cmd.eps, cmd.lookahead);

    } else if (name == "Follow") {
      assign(cmd.mag, child->Attribute("magnitude"));
      assign(cmd.eps, child->Attribute("epsilon"));
      assign(cmd.lookahead, child->Attribute("max_radius"));
      if (!cmd.lookahead) {
        cmd.lookahead = P_MAXFLOAT;
      }

      P.Follow(cmd.mag, cmd.eps, cmd.lookahead);

    } else if (name == "MatchVel") {
      assign(cmd.mag, child->Attribute("magnitude"));
      assign(cmd.eps, child->Attribute("epsilon"));
      assign(cmd.lookahead, child->Attribute("max_radius"));
      if (!cmd.lookahead) {
        cmd.lookahead = P_MAXFLOAT;
      }

      P.MatchVelocity(cmd.mag, cmd.eps, cmd.lookahead);

    } else if (name == "OrbitPoint") {
      assign(cmd.point, child->Attribute("center"));

      assign(cmd.mag, child->Attribute("magnitude"));
      assign(cmd.eps, child->Attribute("epsilon"));
      assign(cmd.lookahead, child->Attribute("max_radius"));
      if (!cmd.lookahead) {
        cmd.lookahead = P_MAXFLOAT;
      }

      P.OrbitPoint(cmd.point, cmd.mag, cmd.eps, cmd.lookahead);

    } else if (name == "Jet") {
      cmd.domain = new ParticleDomain(child);
      cmd.domain_aux = new ParticleDomain(child, "AccelDomain");

      P.Jet(cmd.domain->get_domain(), cmd.domain_aux->get_domain());

    } else if (name == "RandomVel") {
      cmd.domain = new ParticleDomain(child);

      P.RandomVelocity(cmd.domain->get_domain());

    } else if (name == "RandomAccel") {
      cmd.domain = new ParticleDomain(child);

      P.RandomAccel(cmd.domain->get_domain());

    } else if (name == "RandomDisplace") {
      cmd.domain = new ParticleDomain(child);

      P.RandomDisplace(cmd.domain->get_domain());

    } else if (name == "TargetColor") {
      Color3 col;
      assign(col, child->Attribute("color"));
      cmd.point = pVec(col.r, col.g, col.b);

      assign(cmd.eps, child->Attribute("alpha"));
      assign(cmd.mag, child->Attribute("scale"));

      P.TargetColor(cmd.point, cmd.eps, cmd.mag);

    } else if (name == "TargetSize") {
      assign(cmd.point, child->Attribute("size"));
      assign(cmd.point_aux, child->Attribute("scale"));

      P.TargetSize(cmd.point, cmd.point_aux);

    } else if (name == "TargetVel") {
      assign(cmd.point, child->Attribute("size"));
      assign(cmd.mag, child->Attribute("scale"));

      P.TargetVelocity(cmd.point, cmd.mag);

    } else {
      alwaysAssertM(0, "Invalid Particle Action");
    }

    //commands.append(cmd);
  }

  //REMOVE ACTION
  {
    const TiXmlElement* node = Parser::get_element(root, "RemoveCondition");
    const TiXmlElement* child = Parser::get_first_element(node);

    string name = child->ValueStr();

    ParticleCommand cmd;

    if (name == "Age") {
      assign(cmd.mag, child->Attribute("age"));
      assign(cmd.bool_val, child->Attribute("younger-than"));

      P.KillOld(cmd.mag, cmd.bool_val);
    } else if (name == "Position") {
      cmd.domain = new ParticleDomain(child);

      assign(cmd.bool_val, child->Attribute("inside"));
      P.Sink(cmd.bool_val, cmd.domain->get_domain());

    } else if (name == "Velocity") {
      cmd.domain = new ParticleDomain(child);

      assign(cmd.bool_val, child->Attribute("inside"));
      P.SinkVelocity(cmd.bool_val, cmd.domain->get_domain());
    } else {
      alwaysAssertM(0, "Invalid RemoveCondition");
    }

    //commands.append(cmd);
  }


  P.EndActionList();
}

//===========================================
void ParticleActions::apply()
{
  P.CallActionList(action_id);
}

//===========================================
//ParticleSystem
//===========================================

int SpotTexID = 0;

// Symmetric gaussian centered at origin.
// No covariance matrix. Give it X and Y.
inline float Gaussian2(float x, float y, float sigma)
{
  // The sqrt of 2 pi.
#define MY_SQRT2PI 2.506628274631000502415765284811045253006
  return exp(-0.5 * (x*x + y*y) / (sigma*sigma)) / (MY_SQRT2PI * sigma);
}

void MakeGaussianSpotTexture()
{
  const int DIM = 32;
  const int DIM2 = 16;
  const float TEX_SCALE = 6.0;

  glGenTextures(1, (GLuint *)&SpotTexID);
  glBindTexture(GL_TEXTURE_2D, SpotTexID);

  float *img = new float[DIM*DIM];

  for(int y=0; y<DIM; y++) {
    for(int x=0; x<DIM; x++) {
      // Clamping the edges to zero allows Nvidia's blend optimizations to do their thing.
      if(x==0 || x==DIM-1 || y==0 || y==DIM-1)
        img[y*DIM+x] = 0;
      else {
        img[y*DIM+x] = TEX_SCALE * Gaussian2(x-DIM2, y-DIM2, (DIM*0.15));
      }
    }
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA16, DIM, DIM, GL_ALPHA, GL_FLOAT, img);
}

void ParticleSystem::create_systems(int num)
{
//  next_group_id = P.GenParticleGroups(num);


  bool GotExt = true;
  // GotExt = glh_init_extensions("GL_ARB_multitexture") && GotExt;
  // GotExt = glh_init_extensions("GL_NV_vertex_program") && GotExt;
  // GotExt = glh_init_extensions("GL_NV_fragment_program") && GotExt;
  // GotExt = glh_init_extensions("WGL_ARB_pixel_format") && GotExt;
  GotExt = glutExtensionSupported("GL_ARB_point_parameters") && GotExt;
  GotExt = glutExtensionSupported("GL_ARB_point_sprite") && GotExt;

  // Make the point size attenuate with distance.
  // These numbers are arbitrary and need to be fixed for accuracy.
  // The most correct way to do this is to compute the determinant of the upper 3x3 of the
  // ModelView + Viewport matrix. This gives a measure of the change in size from model space
  // to eye space. The cube root of this estimates the 1D change in scale. Divide this by W
  // per point.
  float params[3] = {0.0f, 0.0f, 0.00001f};
  glPointParameterfvARB(GL_POINT_DISTANCE_ATTENUATION_ARB, params);
  glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 0);
  glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, 5000);
  glPointParameterfARB(GL_POINT_FADE_THRESHOLD_SIZE_ARB, 1);

  // Texture unit state
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  float col[4] = {1.f, 1.f, 1.f, 1.f};
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, col);
//  glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

  MakeGaussianSpotTexture();
}

//===========================================
void ParticleSystem::init(const TiXmlElement* root)
{
  //group_id = next_group_id++;
  group_id = P.GenParticleGroups(1);
  
  P.CurrentGroup(group_id);

  P.BirthCallback(&birth_callback, this);

  P.Color(color.r, color.g, color.b);

  P.Size(scale);

  assign(timestep, root->Attribute("speed"));
  timestep = (timestep > 0 ? timestep : 1.f);
  P.TimeStep(timestep);
  timestep *= 30.f; //frame scaler

  assign(max_particles, root->Attribute("max-particles"));
  P.SetMaxParticles(max_particles);

  assign(look_at_cam, root->Attribute("look-at-camera"));

  assign(sequential, root->Attribute("sequential"));


//  TiXmlElement* particle_elem = Parser::get_element(root, "Particles");
//  particle_elem = Parser::get_first_element(particle_elem);
//  string particle_name = particle_elem->ValueStr();
}

//===========================================
void ParticleSystem::init_actions(const TiXmlElement* root)
{
  Content::init_actions(root);

  const TiXmlElement* content = Parser::get_element(root, "Content");
  content = Parser::get_element(content, content_type);

  string group_name = content->Attribute("particle-group");
  group = Story::the_story->get_group(group_name);

  string actions_name = content->Attribute("actions-name");
  actions = Story::the_story->get_particle_actions(actions_name);

  if (actions.notNull()) {
    valid = true;
  }
}


//void DrawGroupAsPoints()
//{
//  glEnable(GL_POINT_SPRITE_ARB);
//  glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, true);
//  size_t cnt = P.GetGroupCount();
//  if(cnt < 1) return;
//  float *ptr;
//  size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs;
//  cnt = P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
//    size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs);
//  if(cnt < 1) return;
////  glEnableClientState(GL_COLOR_ARRAY);
////  glColorPointer(4, GL_FLOAT, int(flstride) * sizeof(float), ptr + color3Ofs);
//  glEnableClientState(GL_VERTEX_ARRAY);
//  glVertexPointer(3, GL_FLOAT, int(flstride) * sizeof(float), ptr + pos3Ofs);
//  glDrawArrays(GL_POINTS, 0, (GLsizei)cnt);
//  glDisableClientState(GL_VERTEX_ARRAY);
//  glDisableClientState(GL_COLOR_ARRAY);
//  glDisable(GL_POINT_SPRITE_ARB);
//}

void DrawGroupAsTriSprites(ParticleContext_t &P, const pVec &view, const pVec &up,
                           float size_scale = 1.0f, bool draw_tex=false,
                           bool const_size=false, bool const_color=false)
{
  int cnt = (int)P.GetGroupCount();

  if(cnt < 1)
    return;

  pVec *ppos = new pVec[cnt];
  float *color = const_color ? NULL : new float[cnt * 4];
  pVec *size = const_size ? NULL : new pVec[cnt];

  P.GetParticles(0, cnt, (float *)ppos, color, NULL, (float *)size);

  // Compute the vectors from the particle to the corners of its tri.
  // 2
  // |\ The particle is at the center of the x.
  // |-\ V0, V1, and V2 go from there to the vertices.
  // |x|\ The texcoords are (0,0), (2,0), and (0,2) respectively.
  // 0-+-1 We clamp the texture so the rest is transparent.

  pVec right = Cross(view, up);
  right.normalize();
  pVec nup = Cross(right, view);
  right *= size_scale;
  nup *= size_scale;

  pVec V0 = -(right + nup);
  pVec V1 = V0 + right * 4;
  pVec V2 = V0 + nup * 4;

  glBegin(GL_TRIANGLES);

  for(int i = 0; i < cnt; i++) {
    pVec &p = ppos[i];

    if(!const_color)
      glColor4fv((GLfloat *)&color[i*4]);

    pVec sV0 = V0;
    pVec sV1 = V1;
    pVec sV2 = V2;

    if(!const_size)
    {
      sV0 *= size[i].x();
      sV1 *= size[i].x();
      sV2 *= size[i].x();
    }

    if(draw_tex) {
      //glTexCoord2f(0,0);
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);
    }
    pVec ver = p + sV0;
    glVertex3fv((GLfloat *)&ver);

    if(draw_tex) {
      //glTexCoord2f(2,0);
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 2, 0);
    }

    ver = p + sV1;
    glVertex3fv((GLfloat *)&ver);

    if(draw_tex) {
      //glTexCoord2f(0,2);
      glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 2);
    }

    ver = p + sV2;
    glVertex3fv((GLfloat *)&ver);
  }

  glEnd();

  delete [] ppos;
  if(color) delete [] color;
  if(size) delete [] size;
}

//===========================================
void ParticleSystem::birth_callback(struct Particle_t& particle, void* data)
{
  ParticleSystem *curr_sys = (ParticleSystem*)data;
  if (!curr_sys) {
    return;
  }

  if (curr_sys->group.notNull()) {
    particle.data = curr_sys->get_new_particle();
  }
}

//===========================================
void ParticleSystem::death_callback(struct Particle_t& particle, void* data)
{
}

//===========================================
int ParticleSystem::get_new_particle()
{
  int id;
  if (sequential) {
    id = obj_counter++;
    obj_counter %= group->get_size();
  } else {
    id = G3D::iRandom(0, group->get_size() - 1);
  }
  return id;
}

//===========================================
void ParticleSystem::render_group(RenderDevice *rd)
{
//  ppos.resize(count);
//  pcolor.resize(count);
//  pvel.resize(count);
//  //pGetParticles(0, count, (float*)ppos.getCArray(), pcolor.getCArray(), NULL, (float *)psize.getCArray(), NULL);
//  P.GetParticles(0, count, (float*)ppos.getCArray(), NULL, (float *)pvel.getCArray());

  Particle_t *particle = NULL;
  int count = (int)P.GetGroupCount();

  if (count < 1) {
    return;
  }

  count = (int)P.GetParticlePointer(particle);

  //dev->setBlendFunc(RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA, RenderDevice::BLENDEQ_ADD);
  
  FontEntry entry = the_app.font_mgr.get_font("");
  CoordinateFrame prev_frame = rd->getObjectToWorldMatrix();

  CoordinateFrame frame, new_frame;
  Vector3 up, look;

  if (look_at_cam) {
    frame = get_curr_frame().inverse() * the_app.get_actual_frame();
  }

  for (int i = 0; i < count; i++) {
    Vector3 pos = *(Vector3*)&particle[i].pos;
    Vector3 vel = *(Vector3*)&particle[i].vel;

    ContentRef obj = group->get_object(particle[i].data);

    if (!look_at_cam) {
      vel.unitize();
      look = vel.cross(Vector3::unitY());
      up = look.cross(vel);

      frame.rotation.setColumn(0, vel);
      frame.rotation.setColumn(1, up);
      frame.rotation.setColumn(2, -look);
      frame.translation = pos;
      new_frame = prev_frame * frame * obj->get_curr_frame();
    } else {
      frame.translation = pos;
      new_frame = prev_frame * frame;
    }

    ShaderRef shader = obj->get_shader();
    if (shader.notNull()) {
      shader->args.set("age", particle[i].age);
    }

    new_frame.rotation = new_frame.rotation * obj->get_scale();
    rd->setObjectToWorldMatrix(new_frame);

    Color4 col = obj->get_color();
    col.a = color.a;
    rd->setColor(col);

    obj->render_direct(rd);
  }
}

//===========================================
void ParticleSystem::render_derived(RenderDevice* dev)
{
  P.CurrentGroup(group_id);

  float rate = dev->frameRate();
  if (rate < 5.f) {
    rate = 5.f;
  }
  P.TimeStep(timestep / rate);

  actions->apply();

  P.Move();

  if (group.notNull()) {
    render_group(dev);
    return;
  }

  int count = (int)P.GetGroupCount();

  if (!count) {
    return;
  }

  dev->pushState();

  //DrawGroupAsPoints();

  CoordinateFrame frame = get_curr_frame().inverse() * the_app.get_actual_frame();
  Vector3 look = frame.lookVector();
  Vector3 up = frame.upVector();

  glActiveTextureARB(GL_TEXTURE0_ARB + 0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, SpotTexID);

  dev->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA, RenderDevice::BLENDEQ_ADD);
  dev->setDepthTest(RenderDevice::DEPTH_ALWAYS_PASS);
  //dev->setTexture(0, tex);
  //glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  DrawGroupAsTriSprites(P, pVec(look.x, look.y, look.z), pVec(up.x, up.y, up.z), 0.1, true, true, true);
  glDisable(GL_TEXTURE_2D);

  dev->popState();
}