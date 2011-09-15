#ifndef _GROUPS_H_
#define _GROUPS_H_

class Group : public CWBase
{
public:
  Group()  {}

  void init(const TiXmlElement* root);
  void resolve_group_refs(const TiXmlElement* root);

  void add(const ContentRef& obj);
  void add(const GroupRef& group);

  ContentRef get_object(const string& name);

  ContentRef get_object(int index) const    { return objects[index]; }
  int        get_size() const               { return objects.size(); }

protected:
  //Array<GroupRef> groups;
  Array<ContentRef> objects;
};


#endif
