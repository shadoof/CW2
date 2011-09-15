#ifndef _FONT_H_
#define _FONT_H_

#include "main.h"

#include <FTGLExtrdFont.h>
#include <FTGLOutlineFont.h>
#include <FTGLPolygonFont.h>
#include <FTGLTextureFont.h>
#include <FTGLPixmapFont.h>
#include <FTGLBitmapFont.h>
#include <FTGlyph.h>
#include <FTVectoriser.h>
#include <FTPoint.h>

//===========================================
struct FontEntry
{
  string name;
  float depth;
  FTFont* the_font;

  bool operator==(const FontEntry& font) const { return (name == font.name) && fuzzyEq(depth, font.depth); }
};

//===========================================
class FontMgr
{
public:
  void close();

  FontEntry get_font(const string& filename, float depth = 0);
  FTFont* load_font(const FontEntry& entry);

private:
  Array<FontEntry> font_entries;
};

#endif
