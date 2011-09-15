// SynchedSystem.cpp

#include "SynchedSystem.H"

#include <G3D/G3DAll.h>
#include <iostream>
using namespace std;

namespace VRG3D {

double SynchedSystem::_appStartTime = System::time();
double SynchedSystem::_localTime    = System::time();

SynchedSystem::TimeUpdateMethod SynchedSystem::_timeUpdateMethod = 
  SynchedSystem::USE_LOCAL_SYSTEM_TIME;


void
SynchedSystem::updateLocalTime(double t) 
{
  debugAssert(_timeUpdateMethod == PROGRAM_UPDATES_TIME);
  _localTime = t;
}

double
SynchedSystem::getLocalTime()
{
  if (_timeUpdateMethod == USE_LOCAL_SYSTEM_TIME) {
    return System::time();
  }
  else {
    return _localTime;
  }
}

void
SynchedSystem::setProgramUpdatesTime(double applicationStartTime) 
{
  _timeUpdateMethod = PROGRAM_UPDATES_TIME;
  _appStartTime = applicationStartTime;
  _localTime = applicationStartTime;
}


} // end namespace
