/*
 *  main.cpp
 *  cwapp
 *
 *  Created by Ilya on 9/28/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <unistd.h>
#include <string>

#include "SDL.h"

extern  bool   gFinderLaunch;

int main(int argc, char** argv)
{
  std::string str = argv[0];
  str = str.substr(0, str.rfind('/'));
  str += "/cwapp";
  argv[0] = strdup(str.c_str());
  
  printf("enter stub: %s\n", str.c_str());
  
  for (int i = 0; i < argc; i++)
  {
    printf("  arg %d: %s\n", i, argv[i]);
  }
  
  // If double click start, then we must fork
  if ( gFinderLaunch )
  {  
    vfork();
  }
  
  return execv(str.c_str(), argv);
}
