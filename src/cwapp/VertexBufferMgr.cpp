#include "main.h"
#include "VertexBufferMgr.H"


VertexBufferMgr *VertexBufferMgr::_instance = 0;

void
VertexBufferMgr::init()
{
  debugAssert( !_instance );
  _instance = new VertexBufferMgr();
}  

void
VertexBufferMgr::cleanupDynamic()
{
  if ( _instance ) {
    for ( int i = 0; i < _instance->_dynamic_var_areas.size(); i++ )
      _instance->_dynamic_var_areas[i]->reset();
    _instance->_dynamic_var_areas.clear();    
  }
}

void
VertexBufferMgr::printVARStats()
{
  if ( _instance ) 
    _instance->printStats();
}

  
VARAreaRef
VertexBufferMgr::currentStaticVARArea( size_t requiredSize )
{
  if ( !_instance ) 
    init();
  return _instance->getStaticVARAreaRef( requiredSize );
}

VARAreaRef
VertexBufferMgr::currentDynamicVARArea( size_t requiredSize )
{
  if ( !_instance ) 
    init();
  return _instance->getDynamicVARAreaRef( requiredSize );
}

VertexBufferMgr::VertexBufferMgr()
{
  _var_area_size = 1024*1024;//ConfigVal("Freeform_VARArea_segment_size",1024*1024,false);
  _total_var_size_allocated = 0;
}

VARAreaRef
VertexBufferMgr::getDynamicVARAreaRef( size_t requiredSize )
{
   // Use a first fit approach
  for ( int i = 0; i < _dynamic_var_areas.size(); i++ ) 
    if ( requiredSize < _dynamic_var_areas[i]->freeSize() )
      return _dynamic_var_areas[i];
  
  size_t newSize = requiredSize < _var_area_size ? _var_area_size : requiredSize + 64;
  _dynamic_var_areas.append( VARArea::create(newSize, VARArea::WRITE_EVERY_FRAME) );
  
  if ( _dynamic_var_areas.last()->freeSize() < requiredSize )
    Log::common()->printf("VertexBufferMgr just tried to get %d of dynamic VAR space but only got %d. I bet you're about to crash.\n", 
		requiredSize, (_dynamic_var_areas.last()->freeSize()) );
    
  return _dynamic_var_areas.last();
}

VARAreaRef
VertexBufferMgr::getStaticVARAreaRef( size_t requiredSize )
{  
  // Use a first fit approach
  for ( int i = 0; i < _static_var_areas.size(); i++ ) 
    if ( requiredSize < _static_var_areas[i]->freeSize() )
      return _static_var_areas[i];
  
  size_t newSize = requiredSize < _var_area_size ? _var_area_size : requiredSize + 64;
  _static_var_areas.append( VARArea::create(newSize, VARArea::WRITE_ONCE) );
  
  if ( _static_var_areas.last()->freeSize() < requiredSize )
    Log::common()->printf("VertexBufferMgr just tried to get %d of static VAR space but only got %d. I bet you're about to crash.\n", 
		requiredSize, (_static_var_areas.last()->freeSize()) );
  
  _total_var_size_allocated += _static_var_areas.last()->freeSize();
  
  return _static_var_areas.last();
}

void
VertexBufferMgr::printStats()
{
  cerr << "Static VAR Area Stats: " << endl;
  for ( int i = 0; i < _static_var_areas.size(); i++ ) {
    double t = (double)(_static_var_areas[i]->totalSize());
    t = t / 100.0;
    char buf[1024];
    
    sprintf( buf, "%d", _static_var_areas[i]->totalSize() );
    
    cerr << "Static #" << i << " | Size: " << buf 
	 << "\tAllocated: " << ((double)(_static_var_areas[i]->allocatedSize()) / t)
	 << "%\tPeak: " << ((double)(_static_var_areas[i]->peakAllocatedSize()) / t)
	 << "%\tFree: " << ((double)(_static_var_areas[i]->freeSize()) / t ) << "%" << endl;
    
  }

  cerr << endl << "Dynamic VAR Area Stats: " << endl;
  for ( int i = 0; i < _dynamic_var_areas.size(); i++ ) {
    double t = (double)(_dynamic_var_areas[i]->totalSize());
    t = t / 100.0;
    char buf[1024];
    
    sprintf( buf, "%d", _dynamic_var_areas[i]->totalSize() );
    
    cerr << "Dynamic #" << i << " | Size: " << buf 
	 << "\tAllocated: " << ((double)(_dynamic_var_areas[i]->allocatedSize()) / t)
	 << "%\tPeak: " << ((double)(_dynamic_var_areas[i]->peakAllocatedSize()) / t)
	 << "%\tFree: " << ((double)(_dynamic_var_areas[i]->freeSize()) / t ) << "%" << endl;
    
  }
}
