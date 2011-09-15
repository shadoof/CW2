#ifndef _PARSE_H_
#define _PARSE_H_

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include <tinyxml.h>
#include <tinystr.h>

#include <VRG3D.H>

typedef ReferenceCountedPointer<class CWBase> BaseRef;
typedef ReferenceCountedPointer<class Story> StoryRef;
typedef ReferenceCountedPointer<class Placement> PlaceRef;
typedef ReferenceCountedPointer<class Content> ContentRef;
typedef ReferenceCountedPointer<class Group> GroupRef;
typedef ReferenceCountedPointer<class Timeline> TimelineRef;
typedef ReferenceCountedPointer<class EventTrigger> EventTriggerRef;
typedef ReferenceCountedPointer<class ActionInstance> ActionInstRef;
typedef ReferenceCountedPointer<class Action> ActionRef;
typedef ReferenceCountedPointer<class SoundObject> SoundRef;
typedef ReferenceCountedPointer<class LightContent> LightContentRef;
typedef ReferenceCountedPointer<class ParticleActions> ParticleActionsRef;


using std::string;

#define BUFF_SIZE 4096

#define S(X) X

//===========================================
class Parser
{
public:
  Parser() : the_root(NULL) {}
  ~Parser();

  bool init(const std::string& filename);

  static Vector3 to_vector(const string& str);

  static const TiXmlElement* get_first_element(const TiXmlNode* root);
  static const TiXmlElement* get_sibling_element(const TiXmlNode* curr);

  static void get_elements(Array<const TiXmlNode*>& elems, const TiXmlElement* root, const string& name = "");

  static const TiXmlElement* get_element(const TiXmlElement* root, const string& elem);
  static string get_element_value(const TiXmlElement* root, const string& name);
        
        wstring get_element_wstring_value(const TiXmlElement* root, const string& name);

    TiXmlElement* get_root() { return the_root; }

  static   void get_box(const TiXmlElement* child, AABox& box, bool& is_inside);

private:
  TiXmlElement* the_root;
};

//===========================================
template <class T>
bool assign(T &val, const string &str) {
  std::istringstream is(str.c_str());
  is >> val;
  if (!is) {
    return false;
  }
  else {
    return true;
  }
}

//===========================================
inline bool assign(Vector3 &val, const string &str) {
  val = Parser::to_vector(str);
  return true;
}

//===========================================
void assign(Color3 &color, string str);
void assign(Color4 &color, string str);

inline void assign(bool& val, string str)
{
  val = (str == "true" || str == "1");
}

#endif
