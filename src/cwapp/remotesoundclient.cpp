#include "main.h"

using VRG3D::SynchedSystem;

//===========================================
RemoteSoundClient::RemoteSoundClient(const string& host, int port)
{
  server_addr = NetAddress(host, port);
  if(!server_addr.ok()) {
    string error = "Invalid network address to the sound server! Shutting down . . .";
    msgBox(error, "Network Error");
  }

  net_dev = NetworkDevice::instance();
  if (!net_dev) {
    string error = "Cannot initialize socket! Shutting down . . .";
    msgBox(error, "Network Error");
  }

  //_conduit = _netDevice->createLightweightConduit(port + 1, true, false);
  conduit = ReliableConduit::create(server_addr);

  last_send_time.append(SynchedSystem::getAppRunningTime());
}

//===========================================
void RemoteSoundClient::close()
{
  conduit = NULL;
  net_dev->cleanup();
  delete net_dev;
}

//===========================================
unsigned int RemoteSoundClient::add_sound(const SoundRequest& req)
{
  unsigned int len = requests.length();
  requests.append(req);
  last_send_time.append(SynchedSystem::getAppRunningTime());
  return len;
}

//===========================================
void RemoteSoundClient::load_all()
{
  if (!requests.length()) {
    conduit = NULL;
    return;
  }

  string login;

#ifdef G3D_WIN32
  char buff[64];
  DWORD b_size = 64;
  GetUserNameA(buff, &b_size);
  buff[b_size] = 0;
  login = buff;
#else
  char* login_ch = getenv("LOGNAME");
  if (login_ch) {
    login = login_ch;
  }
#endif

  SoundServerNetMessageLoad msg(requests, login);
  conduit->send(msg.type(), msg);

  if (!conduit->ok()) {
    conduit = NULL;
    return;
  }

  // Wait for a reply
  double start_time = SynchedSystem::getAppRunningTime();

  do {
    if (conduit->messageWaiting()) {
      int type = conduit->waitingMessageType();

      if (type == SM_SOUND_LOAD_ACK) 	{
        SoundServerNetMessageLoadResponse resp;
        conduit->receive(resp);
        break;

      } else if ((type == 0) || !conduit->ok()) {
        conduit = NULL;
        return;
      } else {
        system_update();
      }
    }
  } while ((SynchedSystem::getAppRunningTime() - start_time) < 30);
}

//===========================================
void RemoteSoundClient::system_update()
{
  if (conduit.isNull()) {
    return;
  }

  if (conduit->messageWaiting()) {
    int type = conduit->waitingMessageType();

    if (type == SM_SOUND_ERROR) 	{
      SoundServerNetMessageError err;
      conduit->receive(err);
      error_handler(0, err.get_error().c_str());


    } else if (type == 0) {
      conduit = NULL;
      return;
    }
  }

  float curr_time = SynchedSystem::getAppRunningTime();

//  if ((curr_time - last_time) < 0.5) {
//    return;
//  }
//
//  last_time = curr_time;

  for (int i = 0; i < send_req.length(); i++) {
    int index = send_req[i]->the_index;
    if ((curr_time - last_send_time[index]) < .2f) {
      continue;
    }

    last_send_time[index] = curr_time;
    send_req[i]->send(conduit);
    delete send_req[i];
    send_req.fastRemove(i);
  }
}

//===========================================
bool RemoteSoundClient::play_sound(unsigned int index, const Vector3& pos)
{
  if (conduit.isNull()) {
    return false;
  }

  SoundServerNetMessagePlay msg(index, pos);
  send_msg(index, msg);

  return true;
}

//===========================================
bool RemoteSoundClient::set_pos_vel(unsigned int index, const Vector3& pos, const Vector3& vel)
{
  if (conduit.isNull()) {
    return false;
  }

  SoundServerNetMessageSetPosVel msg(index, pos, vel);
  send_msg(index, msg);

  return true;
}

//===========================================
bool RemoteSoundClient::start_interp(unsigned int index, const Vector3& start, const Vector3& end, float duration)
{
  if (conduit.isNull()) {
    return false;
  }

  SoundServerNetMessageInterp msg(index, start, end, duration);
  send_msg(index, msg);

  return true;
}

//===========================================
bool RemoteSoundClient::set_listener(const CoordinateFrame& frame, const Vector3& vel)
{
  if (conduit.isNull()) {
    return false;
  }

  SoundServerNetMessageSetListener msg(frame, vel);
  //conduit->send(msg.type(), msg);
  send_msg(-1, msg);

  return true;
}

//===========================================
bool RemoteSoundClient::stop(unsigned int index)
{
  if (conduit.isNull()) {
    return false;
  }

  SoundServerNetMessageStop msg(index);
  send_msg(index, msg);

  return true;
}

//===========================================
void RemoteSoundClient::unload_all()
{
  if (conduit.isNull()) {
    return;
  }

  SoundServerNetMessageUnloadAll msg;
  send_msg(-1, msg);
}

//===========================================
float RemoteSoundClient::get_app_running_time()
{
  return SynchedSystem::getAppRunningTime();
}
