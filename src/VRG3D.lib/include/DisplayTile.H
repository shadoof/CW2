/**
 * \author Daniel Keefe (dfk)
 *
 * \file  DisplayTile.H
 *
 */


#ifndef DISPLAYTILE_H
#define DISPLAYTILE_H

#include <G3D/G3DAll.h>

namespace VRG3D {

/** A description of the display screen for projection-based VR
    systems.  This is used together with a head position and
    orientation to setup the appropriate projection matrix for
    head-tracked rendering.

    Important: After setting or changing any of the variables that
    define the postion of the tile (top*, bot*), calculateRoomToTile()
    needs to be called to update the room2tile matrix.
*/
class DisplayTile
{ 
public:

  enum TileRenderType {
    TILE_MONO,                // normal mono rendering
    TILE_MONO_LEFT,           // mono rendering of left eye only
    TILE_MONO_RIGHT,          // mono rendering of right eye only
    TILE_STEREO,              // opengl quad buffered stereo
    TILE_CHECKERBOARD_STEREO  // checkerboard pattern encoded stereo (samsung tv display)
  };


  /// Sets the state of the tile and calls calculateRoomToTile.
  DisplayTile(Vector3 topLeftCorner, 
              Vector3 topRightCorner, 
              Vector3 botLeftCorner,
              Vector3 botRightCorner,
              TileRenderType tileRenderType,
              double  nearClipPlane,
              double  farClipPlane);

  DisplayTile() {}

  virtual ~DisplayTile();

  TileRenderType  renderType;
  double          nearClip;
  double          farClip;
  Vector3         topLeft;
  Vector3         topRight;
  Vector3         botLeft;
  Vector3         botRight;

  /** Calculates the appropriate room2tile matrix based on the
      location of the tile.  room2tile is cached in this class for
      speed and used by ProjectionVRCamera each frame.  If you change
      the position of the tile, you should call this to recalculate
      the room2tile matrix. */
  void            calculateRoomToTile();
  CoordinateFrame room2tile;

  /// Default setup for a desktop display
  static const DisplayTile& defaultDesktopTile();

};

} // end namespace


#endif
