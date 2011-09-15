#! /bin/sh

echo $G3DINSTALLPATH

#Set paths
ROOTDIR=$(pwd)
LIBPATH=$(pwd)/../../src
BINPATH=$(pwd)/../../bin
LIBOUT=$(pwd)/buildlib

###Make sure lib exists
mkdir $LIBOUT

#####Build freetype

cd $LIBPATH/freetype-2.1.10/
./configure --prefix=$LIBOUT
make clean
make install

##### Not using GLUT on MacOS
#cd $LIBPATH/glut-3.7/
#./mkmkfiles.imake
#cd lib/glut
#make clean
#make
#cp libglut.* $LIBOUT/lib


##### Copy FMOD files to lib/inc dirs
cd $LIBPATH/fmod/inc
cp -f * $LIBOUT/include
cd $LIBPATH/fmod/lib
cp -f * $LIBOUT/lib


###### Use icompile to build Cwapp
# It should take care of the rest building tinyxml, VRG3D
# as dependencies
export CWLIBPATH=$LIBOUT

cd $LIBPATH/cwapp

$G3DINSTALLPATH/bin/icompile . --config $ROOTDIR/config-icompile --noprompt --opt

###### Copy to buildlib/bin dir
cd $LIBOUT
mkdir bin
cp -f -r $LIBPATH/cwapp/build/osx-*/cwapp* bin/

###### Make sample shortcut
cd $BINPATH/osx
ln -s ../samples samples

### Install into osx/bin

###### Build MOTU plugin
#cd $ROOTDIR/motu_surround_plugin
#xcodebuild -project motu_surround_plugin.xcodeproj install

###### Build CaveWriting wrapper 
cd $ROOTDIR/CaveWriting
xcodebuild -project CaveWriting.xcodeproj install

