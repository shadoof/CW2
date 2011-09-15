#include "main.h"


//===========================================
void Group::add(const ContentRef& obj)
{
  objects.append(obj);
}

//===========================================
void Group::add(const GroupRef& group)
{
  for (int i = 0; i < group->objects.size(); i++) {
    objects.append(group->objects[i]);
  }
}

//===========================================
void Group::init(const TiXmlElement* root)
{
  Array<const TiXmlNode*> nodes;
  
  Parser::get_elements(nodes, root, "Objects");

  for (int i = 0; i < nodes.size(); i++) {
    const TiXmlElement *elem = (TiXmlElement*)nodes[i];

    ContentRef obj = Story::the_story->get_object(elem->Attribute("name"));
    if (obj.notNull()) {
      add(obj);
    }
  }

  name = root->Attribute("name");
}

//===========================================
ContentRef Group::get_object(const string& name)
{
  for (int i = 0; i < objects.size(); i++) {
    if (objects[i]->get_name() == name) {
      return objects[i];
    }
  }

  return ContentRef();
}

//===========================================
void Group::resolve_group_refs(const TiXmlElement* root)
{
  Array<const TiXmlNode*> nodes;

  Parser::get_elements(nodes, root, "Groups");

  for (int i = 0; i < nodes.size(); i++) {
    const TiXmlElement *elem = (const TiXmlElement*)nodes[i];
    GroupRef group = Story::the_story->get_group(elem->Attribute("name"));
    if (group.notNull() && (group.pointer() != this)) {
      add(group);
    }
  }
}
