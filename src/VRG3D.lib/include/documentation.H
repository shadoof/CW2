/** \mainpage VRG3D Documentation

\section QL Quick Links

<table border="0" cellspacing="0" cellpadding="0">
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::VRApp</td><td class="memItemRight" valign="bottom">Application base class.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::KnownVRSetups</td><td class="memItemRight" valign="bottom">Holds a list of key names and
descriptions of known VR form factors.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::DisplayTile</td><td class="memItemRight" valign="bottom">Describes the physical location of
the projection wall with respect to the coordinate system of the
trackers.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::ProjectionVRCamera</td><td class="memItemRight" valign="bottom">Knows how to setup the
correct projection matrices given a DisplayTile and head
position.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::Event</td><td class="memItemRight" valign="bottom">Reference counted Event class, stores a
named event plus data of type 1D, 2D, 3D, CoordinateFrame, or
string.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::EventNetMsg</td><td class="memItemRight" valign="bottom">A message for sending events over
the network.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::EventBufferNetMsg</td><td class="memItemRight" valign="bottom">A message for sending an
array of events over the network.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::InputDevice</td><td class="memItemRight" valign="bottom">Base class for input devices that
get polled each frame for new data.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::VRPNTrackerDevice</td><td class="memItemRight" valign="bottom">Connects to a VRPN server and
reports data on trackers.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::VRPNButtonDevice</td><td class="memItemRight" valign="bottom">Connects to a VRPN server and
reports data when buttons are pressed and released.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::VRPNAnalogDevice</td><td class="memItemRight" valign="bottom">Connects to a VRPN server and
reports data for analog devices.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::MouseToTracker</td><td class="memItemRight" valign="bottom">Simulates 6DOF tracking data
with mouse movement - useful for debugging VR apps on a
desktop.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">VRG3D::SynchedSystem</td><td class="memItemRight" valign="bottom">Maintains a clock that is synchronized across all rendering nodes.</td></tr>
<tr><td class="memItemLeft" nowrap align="left" valign="top">G3DOperators.H</td><td class="memItemRight" valign="bottom">Useful routines for strings and G3D types.</td></tr>
</table>

<ul>
<li>Demo application: <a class="el" href="#demo">vrg3d-demo.cpp</a></li>
<li>Makefile for the demo: <a class="el" href="#demomak">Makefile</a></li>
</ul>
<hr>


\section QuickStart Quick Start Guide for Brown Users


<ol>

<li>Put this in your <code>.cshrc</code> or <code>.tchsrc</code>
startup file to setup our <code>$G</code> software framework.  For
more info on $G, see 
http://vis.cs.brown.edu/resources/doc/gfxtools-docs/index.html

\verbatim
foreach d (/cygdrive/c/gfx /cygdrive/d/gfx /map/gfx0 /share/gfx)
  if (-r $d/tools/shared/lib/gfxtools-startup-inc) then
    source $d/tools/shared/lib/gfxtools-startup-inc
    break
  endif
end
\endverbatim
</li>

<li> Open a new shell so that your <code>.cshrc</code> file gets
sourced.  Now, follow these steps to create a new project of your own
(replace mynewproject with the name of your project):

<ol>
<li><code>mkdir ~/mynewproject</code>
<li><code>cp $G/src/G3D/VRG3D/Makefile.newproject ~/mynewproject/Makefile </code>
<li><code>cp $G/src/G3D/VRG3D/vrg3d-demo.cpp      ~/mynewproject/ </code>
<li><code>cd ~/mynewproject </code>
<li>Add your project to the <code>$G</code> cvs tree with the command 
<code>gfxprojinit mynewproject .</code>  (Remember the period at the end of 
the command.)
<li><code>cd $G/src/mynewproject</code>
<li>edit vrg3d-demo.cpp to build your own program by filling in the 
<code>doUserInput()</code> and <code>doGraphics()</code> routines.
<li>run <code>make</code> to compile the project.
<li>use the command <code>obj/mynewproject-d desktop</code> to run 
the program in desktop mode.
</ol>

<li>This gets you started with building your own VRG3D application, to 
run it in the Cave, also take a look at these 
<a class="el" href="#Brown">Cave specific instructions.</a>

</ol>


\section Overview Overview/Goals

VRG3D is a library that sits on top of G3D and implements a minimal
set of features required to run applications written with G3D in
projection-based VR setups, such as CAVEs and Fishtanks.  In
particular, this requires: 1. setting up a stereo display with a
virtual camera that appropriately updates the projection matrix based
on head tracking information each frame, 2. dealing with 6
degree-of-freedom input from trackers and interfacing with devices
typically used in VR, and 3. synchronizing the distribution of input
events and rendering across rendering clusters typically used in
multi-wall or tiled VR setups.

VRG3D provides config files and settings that can be used with serval
different VR configurations at Brown, including the CAVE, and at other
universities.


\section How How It Works

In keeping with a typical G3D demo application setup, VRG3D
applications should subclass from VRG3D::VRApp, which provides two key
methods for the application programer to override (doGraphics() and
doUserInput()).  

doGraphics() is called once per eye for stereo rendering.  VRG3D first
sets the appropriate draw buffer GL_BACK_LEFT or GL_BACK_RIGHT, then
clears the screen, then sets G3D::RenderDevice's prespective
projection matrix and camera-to-world matrix, finally it calls
doGraphics().  So, your application should not call any of these
methods itself.  You should also let VRG3D call
RenderDevice::beginFrame() and endFrame().

doUserInput() is called with an Array of events generated since the
last frame.  You can iterate through this list and listen for events
of a particular name.  See the VRG3D demo for an example.

Connections to typical VR devices, such as trackers, wands, buttons,
etc.. are usually handled through interfacing with UNC's VRPN library, 
which is freely available from the VRPN website:
http://www.cs.unc.edu/Research/vrpn VRG3D uses a device configuration
file to startup connections to a vrpn server and request events from
it.  One of the trackers specified in this config file should generate
an event named "Head_Tracker".  VRG3D will listen for this event and
update camera movement based on it.

The coordinates reported by the trackers should be registered with the
physical space of the projection screen(s).  These coordinates are
specified by the VRG3D::DisplayTile stored in VRG3D::VRApp.

Include Files: Put <CODE>\#include &lt;VRG3D.H&gt;</CODE> in your
project to include all of the VRG3D library headers.

Link Line: Link with VRG3D-d.lib (libVRG3D-d.a on linux systems) for
the debugging version of the library.  Use VRG3D.lib/libVRG3D.a for
the optimized version and libVRG3D-p.a for the profiling version.

Example code: The file vrg3d-demo.cpp is a good place to start to see
how to use the library.


\section Running Running VRG3D Applications in Different VR Setups

When you initialize a new VRApp you either pass it a string
identifying a "Known VR Setup" (see VRG3D::KnownVRSetups) or you 
pass it a bunch of custom settings that describe your own VR setup.
The vrg3d-demo takes as its first argument the name of the known 
setup to start.  If you run the demo with a -h argument it will 
print out the name and description of all the known setups.


\subsection Cluster Running VRG3D Applications on a Cluster

To run applications in a cluster, you need to start one vrg3d-server
to connect to input devices, distribute events to the rendering
clients, and synchronize the rendering and you need to start one
instance of a VRApp on each of the rendering nodes.  The server can be
run as a command-line program or it can open up a graphics window.  In
the window mode, it will pass any keyboard and mouse events generated
in the window on to the clients.  The window mode is the default.  To
run in command line mode start the server with -nogfx as the first
argument.


\subsection Brown Specific Instructions for Brown's CAVE

Prereqs:
<ol>
<li>You must be able to ssh to cs-front and ssh from cs-front to any 
of the other cs-* machines.
<li>You must have $G setup in your account.
</ol>

To run without keyboard and mouse input:
<ol>
<li>ssh cs-front
<li>cd to your program directory.
<li>Run: $G/bin/vrg3d-runcave your-program-name
<li>To quit, press Ctrl-C in the xterm in which you started the program. 
</ol>

To run with keyboard and mouse input from the windows machine "depthcube"
This is currently the machine that the wireless mouse is plugged into, so
use this if you want to get button press events from that mouse device.
<ol>
<li>Make sure the cavedemo user is logged in on "depthcube".  (A program
called grexecd should be running minimized whenever cavedemo is logged in.)
<li>From your linux machine: ssh cs-front
<li>cd to your program directory.
<li>run $G/bin/vrg3d-runcave-winserv-dc your-program-name
<li>To quit, press ESC in the black square window that pops up on the
windows machine.
</ol>

To run with keyboard and mouse input from the windows machine "audio-cave"
<ol>
<li>Make sure the cavedemo user is logged in on "audio-cave".  (A program 
called grexecd should be running minimized whenever cavedemo is logged in.)
<li>From your linux machine: ssh cs-front
<li>cd to your program directory.
<li>Run: $G/bin/vrg3d-runcave-winserv your-program-name
<li>To quit, press ESC in the black square window that pops up on the
windows machine.
</ol>

To run any of the vrg3d-runcave* scripts remotely when you cannot pop up
an xterm for each wall, run the script with -x as the first argument and it
will run ssh processes in the background of the current shell rather than
starting a new xterm for each one.


\section Testing Testing VR Applications on a Desktop Machine

If you tell VRG3D::VRApp to start with a "desktop" VR setup, then it
will just open a normal graphics window that is useful for testing
your code away from VR.  The VRG3D::MouseToTracker class is very
helpful in approximating VR tracker input on a desktop.  Look at the
demo application to see how it is used.  As you move the mouse around
it generates CoordinateFrame events named "Mouse1_Tracker".  If you
hold down the SHIFT key while moving the mouse up and down, you can
move the simulated tracker in the Z direction.  If you hold down the
X, Y, or Z key and then move the mouse from side to side, then you
will rotate the tracker around that axis.  To simulate more than one
tracker, you can tap the TAB key, now the mouse generates events
called "Mouse2_Tracker".  Setup your application to respond to switch
whether it responds to Mouse1_Tracker, Wand_Tracker, or some other name
event depending on whether you start it up in a desktop configuration,
in the Cave, etc..


\section DesktopToVR Considerations in Moving from Desktop to VR

\subsection Coords Coordinate System Conventions

RoomSpace: VRG3D is setup to work with projection-based VR where 3D
trackers produce input that is relative to a projection screen that is
placed in the room.  We call the coordinate space, that both the
trackers and the projection screen live in, "RoomSpace".  The
dimensions and position of the VRG3D::DisplayTile should be specified
in these coordinates, and all tracking events that come into your
program will be in this coordinate system.

VRG3D coordinate system conventions.  Typically, +X is to the right,
+Y is up, and +Z comes out of the screen.  In a Cave setup, we have
the same convention, and +Z typically ends up pointing out of the back
of the Cave, through the doorway.  For desktop, fishtank, and
powerwall setups where there is only one screen, the origin of the
coordinate system is placed directly in the center of the screen.  For
Caves and multi-wall setups, the origin is placed floating directly in
the center of the Cave.  Units are typically reported in feet.  This
setup means that if you place your models and datasets directly at the
origin of the world, you should see them when you start up your
program.


\subsection Camera Position Objects in the Scene, not the Camera

When writing 3D graphics programs for display on the desktop, we often
think about repositioning a virtual camera to get a good view of the
model.  In VR, we have to change this mentality slightly because head
tracking is constantly controlling the position of the virtual camera.

Instead of moving the camera to see the model, move the model so that
it lies within a reasonable position in the room so that the viewer
can walk around it.

VirtualSpace: In the VRG3D demo, we store a single coordinate frame
that specifies the transformation from RoomSpace to a space we call
VirtualSpace.  You may find it useful to use such a convention in your
programs, but VRG3D does not require you to.  VirtualSpace and
RoomSpace are identical when the demo program starts up, but as the
arrow keys are pressed a transformation is applied to the
virtual-to-room-space matrix.  This has the effect of moving the
"camera" around.  You can use a transformation like this to do
something like "clutching" the virtual object using a button and a
tracker and moving it around.  See the <a href="#demo">demo</a> for more ideas.


\section OGL Using OpenGL calls with G3D

If you want to make straight OpenGL calls from within a G3D
application, put them inside this wrapper so that you don't mix up
G3D's idea of the current OpenGL state.

<CODE>
\verbatim

renderDevice->pushState();
glPushAttrib(GL_ALL_ATTRIB_BITS);

// Put your opengl calls here

glPopAttrib();
renderDevice->popState();

\endverbatim
</CODE>



\section TrackD Using trackdAPI Rather than VRPN for Device Input

Edit all the Makefiles that have a line that starts with DEFINES= to
have the string USE_TRACKD at the end of it and recompile the library.
Model your device config file after the file "dive-devices.cfg".





<hr>
\section demo Example Demo Application
\include "vrg3d-demo.cpp"

<hr>
\section demomak Example Demo Makefile
\include "Makefile.newproject"

<hr>

Author and maintainer Daniel Keefe (dfk@cs.brown.edu)

*/
