/**
 * \author John Huffman at Brown Univ. CCV
 *
 * \file  CheckerboardStencil.H
 * \brief Draws a checkerboard pattern in the stencil buffer for use
   with stereo rendering to a SAMSUNG TV display.  

   Usage: 

   checkerboard_stencil(w,h);

   glStencilFunc(GL_NOTEQUAL, 1, 1);
   // Draw scene for left eye

   glStencilFunc(GL_EQUAL, 1, 1);
   // Draw scene for right eye

 */

#ifndef __CHECKERBOARD_STENCIL
#define __CHECKERBOARD_STENCIL

#ifdef __cplusplus
extern "C" {
#endif

  void checkerboard_stencil(int gliWindowWidth, int gliWindowHeight);

#ifdef __cplusplus
}
#endif

#endif

