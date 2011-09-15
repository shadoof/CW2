#include "main.h"

//===========================================
//TextContent
//===========================================
void TextContent::init(const TiXmlElement* root)
{
  Parser* parser = Story::the_story->get_parser();
  if (parser) {
    text = parser->get_element_wstring_value(root, "text");
  } else {
    alwaysAssertM(0, "No Parser");
    //string temp = Parser::get_element_value(root, "text");
  }

  size_t offset = 0;
  size_t last_offset = 0;

  //FTFont* the_font = FontMgr::getDefaultFont(the_app.getRenderDevice());
  string fontname = root->Attribute("font");

  float depth;
  assign(depth, root->Attribute("depth"));

  the_font = the_app.font_mgr.get_font(fontname, depth);
  
  if (!the_font.the_font || !text.length()) {
    return;
  }

  halign = root->Attribute("horiz-align");
  valign = root->Attribute("vert-align");

  placement.invert_look_at();

  TextLine line;

  while ((offset = text.find('\n', last_offset)) != wstring::npos) {
    wstring text_line = text.substr(last_offset, offset - last_offset);
    line.init(text_line, the_font);
    fragments.append(line);
    last_offset = offset + 1;
  }

  {
    wstring text_line = text.substr(last_offset, text.length() - last_offset);
    line.init(text_line, the_font);
    fragments.append(line);
  }

  //float total_height = the_font->LineHeight() * fragments.length();
  Vector3 total_bounds = Vector3::zero();

  //Horiz
  for (int i = 0; i < fragments.length(); i++) {
    //Vector3 dim = fragments[i].get_dimension();

    //total_bounds.x = G3D::max(total_bounds.x, dim.x);
    float curr_width = fragments[i].get_advance();

    total_bounds.x = G3D::max(total_bounds.x, curr_width);
    //total_bounds.z = G3D::max(total_bounds.z, dim.z);
    total_bounds.y += fragments[i].get_ascent() - fragments[i].get_descent();

    float xpos = 0;

    if (halign == "center") {
      xpos = -curr_width * .5;
    } else if (halign == "right") {
      xpos = -curr_width;
    } else {
      xpos = 0;
    }

    fragments[i].frame.translation.x = xpos;
  }

  total_bounds.z = the_font.depth * scale / 220 + .002f;

  float ypos = 0;

  Vector3 lower = total_bounds * -.5;
  Vector3 upper = total_bounds * .5;
  lower.z = -total_bounds.z;
  upper.z = 0;

  if (halign == "left") {
    lower.x += (total_bounds.x * .5);
    upper.x += (total_bounds.x * .5);
  } else if (halign == "right") {
    lower.x -= (total_bounds.x * .5);
    upper.x -= (total_bounds.x * .5);
  }

  if (valign == "center") {
    ypos += total_bounds.y * .5;
  } else if (valign == "bottom") {
    ypos += total_bounds.y;
    lower.y += (total_bounds.y * .5);
    upper.y += (total_bounds.y * .5);
  } else {
    ypos += 0;
    lower.y -= (total_bounds.y * .5);
    upper.y -= (total_bounds.y * .5);
  }

  local_bbox.set(lower, upper);

  for (int i = 0; i < fragments.length(); i++) {
    ypos -= fragments[i].get_ascent();

    fragments[i].frame.translation.y = ypos;

    ypos += fragments[i].get_descent();

    fragments[i].orient();
  }

  update_bbox();

  valid = (fragments.size() > 0);
}

//===========================================
void TextContent::render_derived(RenderDevice* RD)
{
  //glEnable(GL_TEXTURE_2D);

  RD->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, 
    RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA, 
    RenderDevice::BLENDEQ_ADD);

  if (!use_lighting()) {
    if (the_font.depth > 0) {
      RD->enableLighting();
    } else {
      RD->disableLighting();
    }
  }

  for (int i = 0; i < fragments.length(); i++) {
    fragments[i].render(RD);
  }

  //glDisable(GL_TEXTURE_2D);

  //Draw::box(bounding_box, RD);
}

//===========================================
//TextLine
//===========================================
void TextLine::init(const wstring& text_line, const FontEntry& f)
{
  text = text_line;
  font = f;
  scale = 1.f / 220;
}

//===========================================
void TextLine::orient()
{
  frame.rotation = scale * frame.rotation;
}

//===========================================
Vector3 TextLine::get_dimension()
{
  Vector3 ll, ur;
  font.the_font->BBox(text.c_str(), ll.x, ll.y, ll.z, ur.x, ur.y, ur.z);
  return (ur - ll) * scale;
}

//===========================================
void TextLine::render(RenderDevice* RD)
{
  RD->pushState();
  CoordinateFrame abc = RD->getObjectToWorldMatrix() * frame;
  RD->setObjectToWorldMatrix(abc);

  font.the_font->Render(text.c_str());

  RD->popState();
}

//===========================================
float TextLine::get_advance()
{
  return (font.the_font->Advance(text.c_str())) * scale;
}

//===========================================
float TextLine::get_ascent()
{
  return (font.the_font->Ascender()) * scale;
}

//===========================================
float TextLine::get_descent()
{
  return (font.the_font->Descender()) * scale;
}