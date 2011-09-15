#include "main.h"


SoundServer* SoundServer::curr_server = NULL;

SoundServer::SoundServer(int port) : _running(true),
												 _waiting(true),
												 _serving(false)
{
  sound_sys = new SoundSystem();

  if (!sound_sys->init(error_handler))
  {
	  std::cout << "Cannot initialize sound system! Shutting down . . .\n";
	  exit(1);
  }

  _networkDevice = NetworkDevice::instance();
  if (!_networkDevice) 
  {
	  std::cout << "Cannot initialize socket! Shutting down . . .\n";
	  exit(1);
  }

  sessions = 0;

  _listener = NetListener::create(port);

  curr_server = this;
  //_conduit = _networkDevice->createLightweightConduit(port, true, false);
}

SoundServer::~SoundServer()
{
  curr_server = NULL;
  _conduit = NULL;
  _networkDevice->cleanup();
  sound_sys->close();
  delete _networkDevice;
  delete sound_sys;
}

void SoundServer::stop()
{
	_running = false;
	_waiting = false;
	_serving = false;
}

bool SoundServer::verifyAddress(NetAddress &address)
{
	return _clientAddress == address;
}

void SoundServer::resolveSounds(Array<SoundRequest>& req, string login)
{
  for (int i = 0; i < req.length(); i++) {
	//if (do_scp) {
	  //string dir = "audio/" + login;
    int index = -1;
    index = req[i].filename.find("studentwork/");

    if (index == string::npos) {
      index = req[i].filename.find("cave-work/");
    }
    if (index != string::npos) {
	    string dir = "/studentwork/";
      
      //string filename = req[i].filename.substr(req[i].filename.find_last_of('/') + 1);
      string filename = req[i].filename.substr(req[i].filename.find_first_of('/', index) + 1);
  		
      string path_filename = dir + filename;

      req[i].filename = path_filename;

    }
	  
	  cout << "Sound: " << req[i].filename << endl;
    		
	  /*if (!G3D::fileExists(path_filename)) {
        #ifdef G3D_WIN32
	    mkdir(dir.c_str());
        #else
	    mkdir(dir.c_str(), S_IRWXU);
        #endif

	    string req = "scp cavedemo@login.cascv.brown.edu:/share/htdocs/cavewriting/2006/studentwork/" + login + "/" + filename;
        req += " " + path_filename;

        cout << "scp: " << req << endl;
        int res = system(req.c_str());
        cout << "res: " << res << endl;
	  }*/
	  //fnamefull = path_filename;
    //}        
  }
}

void SoundServer::connectionLoop()
{
	// Repeat the loop continuously while the server is running
	while(_running)
	{
		std::cout << "Waiting for connections . . .\n";
		// Handle incoming connections

    _conduit = _listener->waitForConnection();
    _waiting = _conduit->ok();
    _clientAddress = _conduit->address();

    while(_waiting)
		{
      if (!_conduit->ok()) {
        cout << "Client Disconnected" << endl;
        break;
      }

      sound_sys->system_update();

      if(_conduit->messageWaiting())
			{
				// New connection
				if(_conduit->waitingMessageType() == SM_SOUND_LOAD)
				{
					SoundServerNetMessageLoad msg;
					_conduit->receive(msg);
					Array<SoundRequest> req_array = msg.getSoundArray();
					resolveSounds(req_array, msg.getLogin());					
					sound_sys->load_sounds(req_array);
                    _waiting = false;
					break;
				}
			}
		}

    if (!_conduit->ok()) {
      cout << "Resetting Server" << endl;
      sound_sys->unload_all();
      continue;
    }

    SoundServerNetMessageLoadResponse msg(sessions++);
    _conduit->send(msg.type(), msg);

    _serving = true;
		
		// Handle sound requests
		while(_serving)
		{
      if (!_conduit->ok()) {
        cout << "Client Disconnected" << endl;
        break;
      }

      sound_sys->system_update();

			if (_conduit->messageWaiting()) {
				NetAddress address;
				switch(_conduit->waitingMessageType()) {

				// Setting a source's position
				case SM_SOUND_SETPOSVEL:
					{
						SoundServerNetMessageSetPosVel msg;
						_conduit->receive(msg);
            sound_sys->set_pos_vel(msg.get_index(), msg.get_pos(), msg.get_vel());
					}
					break;

        case SM_SOUND_INTERP:
          {
            SoundServerNetMessageInterp msg;
            _conduit->receive(msg);
            sound_sys->start_interp(msg.get_index(), msg.get_start_pos(), msg.get_end_pos(), msg.get_duration());
          }
          break;
				
				// Play a source
				case SM_SOUND_PLAY:
					{
						SoundServerNetMessagePlay msg;
						_conduit->receive(msg);
            sound_sys->play_sound(msg.get_index(), msg.get_pos());
					}
					break;
					
				// Stop a source
				case SM_SOUND_STOP:
					{
						SoundServerNetMessageStop msg;
						_conduit->receive(msg);
            sound_sys->stop(msg.get_index());
					}
					break;
					
				// Set the listener position and orientation
				case SM_SOUND_SETLISTENER:
					{
            SoundServerNetMessageSetListener msg;
            _conduit->receive(msg);
            sound_sys->set_listener(msg.get_frame(), msg.get_vel());
					}
					break;
					
				// Kill the server
				case SM_SOUND_UNLOAD_ALL:
					{
            SoundServerNetMessageUnloadAll msg;
            _conduit->receive(msg);
            sound_sys->unload_all();
            _waiting = true;
            _serving = false;
					}
					break;
					
				default:
					break;
				}
			}
		}
		
		std::cout << "Connection to " << _clientAddress << " closed.\n\n";
    sound_sys->unload_all();
	}
}

void SoundServer::send_error_msg(const string& err)
{
  if (_conduit.isNull()) {
    return;
  }

  SoundServerNetMessageError msg(err);
  _conduit->send(msg.type(), msg);  
}

void SoundServer::error_handler(unsigned int code, const char* msg)
{
  if (curr_server) {
    curr_server->send_error_msg(msg);
  }
}
