#ifndef _PARTICLE_SYSTEM_H_
#define _PARTICLE_SYSTEM_H_

#include "particle_api/pAPI.h"
#include "particle_api/pVec.h"
#include "particle_api/Particle.h"

using PAPI::pVec;
using PAPI::pDomain;
using PAPI::Particle_t;

//===========================================
inline bool assign(pVec &val, const string &str) {
  Vector3 vec = Parser::to_vector(str);
  val = pVec(vec.x, vec.y, vec.z);
  return true;
}

//===========================================
class ParticleDomain : public ReferenceCountedObject
{
public:
  ParticleDomain(const TiXmlElement* root, string elem_name = "");
  ~ParticleDomain() { delete domain; domain = NULL; }

  const pDomain& get_domain() { return *domain; }

private:
  static pVec  get_vec(const TiXmlElement* root, const string& name);
  static float get_float(const TiXmlElement* root, const string& name);

  pDomain* domain;
  string name;
};

typedef ReferenceCountedPointer<class ParticleDomain> ParticleDomainRef;

//===========================================
class ParticleActions : public CWBase
{
public:

  //===========================================
  struct ParticleCommand
  {
    pVec point, point_aux;
    ParticleDomainRef domain, domain_aux;

    float mag;
    float eps;
    float lookahead;

    bool bool_val;
  };

  enum Command {
    P_AVOID,
    P_BOUNCE,
    P_GRAVITY,
    P_GRAVITATE,
    P_FOLLOW,
    P_JET,
    P_KILLOLD,
  };

  ParticleActions() { action_id = -1; }

  virtual void init(const TiXmlElement* root);

  void apply();

  static void create_lists(int num);

private:
  Array<ParticleCommand> commands;

  static int next_action_list;
  int action_id;
};

typedef ReferenceCountedPointer<class ParticleActions> ParticleActionsRef;


//===========================================
class ParticleSystem : public Content
{
public:
  ParticleSystem() { group_id = -1; max_particles = 0; obj_counter = 0; sequential = false; look_at_cam = false; }

  virtual void init(const TiXmlElement* root);
  virtual void init_actions(const TiXmlElement* root);
  virtual void render_derived(RenderDevice* dev);

  void render_group(RenderDevice *rd);

  int get_new_particle();

  static void create_systems(int num);

  static void birth_callback(struct Particle_t& particle, void* sys);
  static void death_callback(struct Particle_t& particle, void* sys);



private:
  int group_id;
  size_t max_particles;
  bool sequential;
  int obj_counter;
  float timestep;
  bool look_at_cam;

  ParticleActionsRef actions;

  Array<Vector3> ppos;
  Array<Vector3> pvel;
  Array<float> pcolor;

  GroupRef group;

  static int next_group_id;
};

#endif
