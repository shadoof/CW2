#include "main.h"

//===========================================
void FontMgr::close()
{
  for(int i = 0; i < font_entries.size(); i++) {
    delete font_entries[i].the_font;
  }
  font_entries.clear();
}

//===========================================
FontEntry FontMgr::get_font(const string& filename, float depth)
{
  FontEntry entry;
  entry.the_font = NULL;
  if (!filename.length()) {
    entry.name = "Courier.ttf";
  } else {
    entry.name = filename;
  }
  entry.depth = fabs(depth);

  for (int i = 0; i < font_entries.length(); i++) {
    if (font_entries[i] == entry) {
      return font_entries[i];
    }
  }

  entry.the_font = load_font(entry);

  if (entry.the_font)
  {
    font_entries.append(entry);
    return entry;
  }

  // Attempt loading default courier
  if (filename != "Courier.ttf")
  {
    return get_font("Courier.ttf");
  }

  msgBox("No Fonts Found At All!", "Fatal Error");
  return entry;
}

//===========================================
FTFont* FontMgr::load_font(const FontEntry& entry)
{
  // Assume path relative to story first
  string name = Story::resolve_rel_path(entry.name);

  // If it doesn't exist, check the CW fonts dir
  if (!G3D::fileExists(name)) {
    name = Story::resolve_rel_path("./fonts/" + entry.name);
  }

  // If it doesn't exist, check the CW fonts dir
  if (!G3D::fileExists(name)) {
    name = "./fonts/" + entry.name;
  }

  //See if in parent fonts dir
  if (!G3D::fileExists(name)) {
    name = "../fonts/" + entry.name;
  }

  if (!G3D::fileExists(name)) {
    msgBox("The required font file \"" + name + "\" could not be found.\n"
           "Attempting to substitute Courier.ttf", "Fatal Error");
    return NULL;
  }

  FTFont* font = NULL;

  try {
    if (entry.depth > 0) {
      font = new FTGLExtrdFont(name.c_str());
      font->Depth(entry.depth);
    } else {
      font = new FTGLPolygonFont(name.c_str());
    }

    bool setFaceSize = font->FaceSize(64);
    font->UseDisplayList(true);
    font->CharMap(ft_encoding_unicode);

    return font;
  } catch (...) {
    msgBox("There was an error loading the font file \"" + entry.name + "\"could not be loaded", "Fatal Error");
    return NULL;
  }
}

