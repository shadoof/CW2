#ifndef _REMOTE_SOUND_CLIENT_H_
#define _REMOTE_SOUND_CLIENT_H_

#include <queue>

class RemoteSoundClient : public BaseSoundClient
{
public:
  RemoteSoundClient(const string& host, int port);
  virtual ~RemoteSoundClient() {}

  void close();

  bool is_succeeded() const { return (conduit.notNull() && conduit->ok()); }

  unsigned int add_sound(const SoundRequest& req);

  void load_all();
  void system_update();
  bool play_sound(unsigned int index, const Vector3& pos);
  bool set_pos_vel(unsigned int index, const Vector3& pos, const Vector3& vel);
  bool start_interp(unsigned int index, const Vector3& start, const Vector3& end, float duration);
  bool set_listener(const CoordinateFrame& frame, const Vector3& vel);
  bool stop(unsigned int index);
  void unload_all();

  float get_app_running_time();

  class BaseSendPacket
  {
  public:
    virtual ~BaseSendPacket() {}
    virtual void send(ReliableConduitRef& conduit) = 0;

    float curr_time;
    int the_index;

  };

  struct PacketStore
  {
    PacketStore() { packet = NULL; }

    void set(BaseSendPacket* new_pack)
    {
      if (packet) {
        delete packet;
      }
      packet = new_pack;
    }

  private:
    BaseSendPacket *packet;
  };

  template <class T>
  class SendPacket : public BaseSendPacket
  {
  public:
    SendPacket(int index, float time, const T& obj) { the_index = index; curr_time = time; the_obj = obj; }
    virtual void send(ReliableConduitRef& conduit)
    {
      conduit->send(the_obj.type(), the_obj);
    }


  private:
    T the_obj;
  };

  template <class T>
  void send_msg(unsigned int index, const T& packet)
  {
    float curr_time = get_app_running_time();
    send_req.append(new SendPacket<T>(index + 1, curr_time, packet));
  }

private:
  NetworkDevice* net_dev;
  NetAddress server_addr;
  ReliableConduitRef conduit;
  Array<SoundRequest> requests;

  Array<float> last_send_time;
  //priority_queue<float, PacketStore*> send_req;
  Array<BaseSendPacket*> send_req;
};
#endif
