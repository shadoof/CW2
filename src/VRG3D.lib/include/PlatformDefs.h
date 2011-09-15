/*
 *  PlatformDefs.h
 *  VRG3D
 *
 *  Created by CAVE on 9/25/09.
 *  Copyright 2009 CaveWriting. All rights reserved.
 *
 */
 
#ifndef _PLATFORM_DEFS_H_
#define _PLATFORM_DEFS_H_

#include <G3D/G3DAll.h>
 
 // Window Lib Settings moved here to be consistent
#ifdef G3D_WIN32
  #define USE_WIN32
#elif defined(G3D_OSX)
  #define USE_SDL
#else
  //Presumably linux -- use glut
  #define USE_GLUT
#endif

#endif

