#ifndef __VERTEXBUFFERMGR_H__
#define __VERTEXBUFFERMGR_H__

/**
 * @file   VertexBufferMgr.H
 * @author "Dmitri Lemmerman" <dlemmerm@cs.brown.edu>
 * @date   Wed Sep 22 17:35:48 2004
 * 
 * @brief  VertexBufferMgr is a singleton class that keeps track 
 *         of how memory is assigned for vertex buffers. The static
 *         VARAreas are never erased. The dynamic ones are erased every frame.
 * 
 */


#include <VRG3D.H>

class VertexBufferMgr
{
public:
  static void       init();
  static void       cleanupDynamic();
  static VARAreaRef currentStaticVARArea ( size_t requiredSize );
  static VARAreaRef currentDynamicVARArea( size_t requiredSize );
  static void       printVARStats();
private:
  static VertexBufferMgr    *_instance;
  
  VertexBufferMgr();

  VARAreaRef getStaticVARAreaRef ( size_t requiredSize );
  VARAreaRef getDynamicVARAreaRef( size_t requiredSize );
  void       printStats();

  size_t              _var_area_size, _total_var_size_allocated;
  Array< VARAreaRef > _static_var_areas;
  Array< VARAreaRef > _dynamic_var_areas;
};

#endif

