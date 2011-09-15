#ifndef SOUNDSERVER_H_
#define SOUNDSERVER_H_

class SoundServer
{
public:
  SoundServer(int port);
  virtual ~SoundServer();
	
  void resolveSounds(Array<SoundRequest>& req, string login);
	
  void connectionLoop();
  void stop();

  static void error_handler(unsigned int code, const char* msg);

  void send_error_msg(const string& msg);

	
private:
  bool verifyAddress(NetAddress &address);

  static SoundServer *curr_server;
	
	// Network
	NetworkDevice			*_networkDevice;
	ReliableConduitRef	_conduit;
  NetListenerRef    _listener;
	NetAddress				_clientAddress;

	// System
	SoundSystem		*sound_sys;
  
	// Server state
	bool					 _running, _waiting, _serving;
  int sessions;
};

#endif
