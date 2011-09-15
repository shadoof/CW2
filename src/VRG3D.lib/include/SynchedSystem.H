/*
 * \author Andy Foresberg (asf) and Daniel Keefe (dfk)
 *
 * \file  SynchedSystem.H
 *
 */



#ifndef SYNCHEDSYSTEM_H
#define SYNCHEDSYSTEM_H

namespace VRG3D {


/** This is a static class that reproduces some of the functionality
    of G3D::System for use specifically in clustered rendering
    environments.  For example, when doing animation based on the
    system's clock you want the time to be the same across all the
    rendering nodes, so you can't use System::getLocalTime() because
    it will be slightly off between the various nodes.  Instead, use
    SynchedSystem::getLocalTime() and the value for time will be the
    same across all walls, although, it will only be as accurate as
    the rate at which the time is updated from the vrg3d event server,
    which will be once per frame.
*/
class SynchedSystem
{
public:

  /// Programs should access the time through these methods
  static double getLocalTime();
  static double getAppStartTime()  { return _appStartTime; }
  static double getAppRunningTime(){ return getLocalTime() - _appStartTime; }

  enum TimeUpdateMethod {
    USE_LOCAL_SYSTEM_TIME,  // used in non-cluster situations (default)
    PROGRAM_UPDATES_TIME    // used in clustered situations
  };

  /// Called by VRApp when running as a cluster client to initialize
  /// the system.
  static void setProgramUpdatesTime(double applicationStartTime);
  
  /// Called by VRApp when running as a cluster client once per frame
  /// when it receives a new timing event from the synchronization
  /// server.
  static void updateLocalTime(double t);

  static TimeUpdateMethod getTimeUpdateMethod() { return _timeUpdateMethod; }

protected:
  static TimeUpdateMethod _timeUpdateMethod;
  static double           _appStartTime;
  static double           _localTime;
};

} // end namespace

#endif
