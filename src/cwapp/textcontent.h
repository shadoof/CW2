#ifndef _TEXT_CONTENT_H_
#define _TEXT_CONTENT_H_

//===========================================
class TextLine
{
public:
  void init(const wstring& text_line, const FontEntry& f);

  Vector3 get_dimension();

  void render(RenderDevice* RD);

  void orient();

  float get_advance();
  float get_ascent();
  float get_descent();

  FontEntry font;
  CoordinateFrame frame;
  float scale;

  wstring text;
};

//===========================================
class TextContent : public Content
{
public:
  virtual void init(const TiXmlElement* root);

  virtual void render_derived(RenderDevice* dev);

private:
  wstring text;
  string halign;
  string valign;

  Array<TextLine> fragments;
  FontEntry the_font;
};

#endif
