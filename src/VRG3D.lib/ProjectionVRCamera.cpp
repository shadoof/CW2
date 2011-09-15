#include "ProjectionVRCamera.H"

namespace VRG3D {

ProjectionVRCamera::ProjectionVRCamera(DisplayTile dispTile, CoordinateFrame initHeadFrame,
                                       double interOcularDist)
{
  tile = dispTile; 
  headFrame = initHeadFrame;
  iod = interOcularDist;
}

ProjectionVRCamera::~ProjectionVRCamera()
{
}


CoordinateFrame
ProjectionVRCamera::getLeftEyeFrame()
{
  return headFrame * CoordinateFrame(Vector3(-iod/2.0, 0.0, 0.0));
}

CoordinateFrame
ProjectionVRCamera::getRightEyeFrame()
{
  return headFrame * CoordinateFrame(Vector3( iod/2.0, 0.0, 0.0));
}


void
ProjectionVRCamera::applyProjection(RenderDevice *rd, EyeProjectionType whichEye)    
{
  // 1. Get the center of the camera (the eye) position from the head position
  CoordinateFrame eye2room = headFrame;
  if (whichEye == LeftEye) {
    eye2room = getLeftEyeFrame();
    //rd->setDrawBuffer(RenderDevice::BUFFER_BACK_LEFT);
    glDrawBuffer(GL_BACK_LEFT);
  }
  else if (whichEye == RightEye) {
    eye2room = getRightEyeFrame();
    //rd->setDrawBuffer(RenderDevice::BUFFER_BACK_RIGHT);
    glDrawBuffer(GL_BACK_RIGHT);
  }
  else {
    rd->setDrawBuffer(RenderDevice::BUFFER_BACK);
  }

  // 2. Setup projection matrix
  Vector3 eye = (tile.room2tile * eye2room).translation;
  double halfWidth = (tile.topRight - tile.topLeft).length() / 2.0;
  double halfHeight = (tile.topRight - tile.botRight).length() / 2.0;
  double l = (-halfWidth - eye[0]);
  double r = ( halfWidth - eye[0]);
  double b = (-halfHeight - eye[1]);
  double t = ( halfHeight - eye[1]);
  double dist = eye[2];
  double k = tile.nearClip / dist;

  // 3. Add eye position to the projection (eye is in tile coordinates)
  CoordinateFrame r2t = CoordinateFrame(-eye) * tile.room2tile;

  // 4. Apply the projection to the RenderDevice
  rd->setProjectionMatrix( Matrix4::perspectiveProjection(l*k, r*k, b*k, t*k, tile.nearClip, tile.farClip) );
  rd->setCameraToWorldMatrix(r2t.inverse());
}


Vector3
ProjectionVRCamera::getLookVec()
{
  Vector3 filmPlaneCtr = tile.topLeft + 0.5*(tile.topRight-tile.topLeft) + 0.5*(tile.botLeft-tile.topLeft);
  return (filmPlaneCtr - headFrame.translation).unit();
}

} // end namespace

