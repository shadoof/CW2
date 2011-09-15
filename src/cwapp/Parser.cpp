#include "main.h"
#include "ConvertUTF.h"

//===========================================
void assign(Color3 &color, string str)
{
  if (str.length() > 0) {
    if (str[0] != '(') {
      str = "(" + str + ")";
    }

    Vector3 vec = Parser::to_vector(str);
    
    //scale down to 0-1 range
    color.r = vec.x / 255.f;
    color.g = vec.y / 255.f;
    color.b = vec.z / 255.f;
  }
}

//===========================================
void assign(Color4 &color, string str)
{
  if (str.length() > 0) {
    if (str[0] != '(') {
      str = "(" + str + ")";
    }

    Vector3 vec = Parser::to_vector(str);

    //scale down to 0-1 range
    color.r = vec.x / 255.f;
    color.g = vec.y / 255.f;
    color.b = vec.z / 255.f;
  }
}

//===========================================
Parser::~Parser()
{
}


//===========================================
bool Parser::init(const std::string& filename)
{
  TiXmlDocument* doc = new TiXmlDocument();

  if (!G3D::fileExists(filename)) {
    string error = "Story File \"" + filename + "\" Was Not Found. Check the name and make sure it exists.";
    msgBox(error, "CaveWriting Error");
    return false;
  }

  doc->SetCondenseWhiteSpace(false);

  // Get the document, and its root element
  if (!doc->LoadFile(filename)) {
    string error = "File \"" + filename + "\" is not a valid Story XML file";
    msgBox(error, "CaveWriting Error");
    return false;
  }

  the_root = doc->FirstChildElement("Story");

  return true;
}

//===========================================
Vector3 Parser::to_vector(const string& str)
{
  TextInput input(TextInput::FROM_STRING, str);
  Vector3 vec;
  vec.deserialize(input);
  return vec;
}

//===========================================
const TiXmlElement* Parser::get_element(const TiXmlElement* root, const string& name)
{
  return root->FirstChildElement(name);
}

//===========================================
string Parser::get_element_value(const TiXmlElement* root, const string& name)
{
  const TiXmlElement *elem = get_element(root, name);
  if (elem) {
    return string(elem->GetText());
  }
  return string();
}

//===========================================
wstring Parser::get_element_wstring_value(const TiXmlElement* root, const string& name)
{
  static wchar_t buff[BUFF_SIZE];
  memset(buff, 0, sizeof(buff));

  std::string str = get_element_value(root, name);

  const char* srcp = str.c_str();

  wchar_t* dstp = buff;

  if (sizeof(wchar_t) == 2)
  {

    ConvertUTF8toUTF16((const UTF8**)&srcp, (const UTF8*)&srcp[str.length()],
                       (UTF16**)&dstp,      (UTF16*)&buff[BUFF_SIZE], lenientConversion);

  }
  else if (sizeof(wchar_t) == 4)
  {
    ConvertUTF8toUTF32((const UTF8**)&srcp, (const UTF8*)&srcp[str.length()],
                       (UTF32**)&dstp,      (UTF32*)&buff[BUFF_SIZE], lenientConversion);
      
  }
  else
  {
     //ERROR!
  }

  return wstring(buff);
  //if (elem) {
  //  const XMLCh* chars = elem->getTextContent();
  //  if (chars) {
	 //   if (!wchar_transcoder) {
	 //     string non_unicode(chars);
	 //     text.assign(non_unicode.begin(), non_unicode.end());
	 //   } else {
  //      unsigned int len = XMLString::stringLen(chars);
  //      XMLSize_t eaten = 0;
	 //   wchar_transcoder->transcodeTo(chars, len, buff, BUFF_SIZE - sizeof(wchar_t), eaten, XMLTranscoder::UnRep_RepChar);
  //      wchar_t *wide = (wchar_t*)buff;
  //      wide[eaten] = NULL;
  //      text = wide;
	 //   }
  //  }
  //}
  //return text;
}

//===========================================
void Parser::get_elements(Array<const TiXmlNode*>& elems, const TiXmlElement* root, const string& name)
{
  elems.clear();
  if (!root) {
    return;
  }
  const TiXmlNode *child = root->FirstChild();
  while (child) {
    if ((!name.length() || (name == child->ValueStr())) && (child->Type() == TiXmlNode::ELEMENT)) {
      elems.append(child);
    }
    child = child->NextSibling();
  }
}

//===========================================
const TiXmlElement* Parser::get_first_element(const TiXmlNode* root)
{
  const TiXmlNode *child = root->FirstChild();
  if (!child) {
    return NULL;
  }
  if (child->Type() == TiXmlNode::ELEMENT) {
    return (const TiXmlElement*)child;
  } else {
    return get_sibling_element(child);
  }
}

//===========================================
const TiXmlElement* Parser::get_sibling_element(const TiXmlNode* curr)
{
  const TiXmlNode *sib = NULL;

  if (!curr) {
    return NULL;
  }

  while (curr = curr->NextSibling()) {
    if (curr->Type() == TiXmlNode::ELEMENT) {
      return (const TiXmlElement*)curr;
    }
  }

  return NULL;
}

//===========================================
void Parser::get_box(const TiXmlElement* child, AABox& box, bool& is_inside)
{
  Vector3 p1, p2;
  assign(p1, child->Attribute("corner1"));
  assign(p2, child->Attribute("corner2"));

  Vector3 bmin = p1.min(p2);
  Vector3 bmax = p1.max(p2);

  bool ignore_y;
  assign(ignore_y, child->Attribute("ignore-Y"));
  if (ignore_y) {
    bmin.y = -1000;
    bmax.y = 1000;
  }

  box.set(bmin, bmax);

  const TiXmlElement *inside_elem = Parser::get_element(child, "Movement");
  const TiXmlElement *in_out = Parser::get_first_element(inside_elem);
  string inside = in_out->ValueStr();
  is_inside = (inside == "Inside");
}
