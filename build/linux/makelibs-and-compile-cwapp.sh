#! /bin/sh

echo $G3DINSTALLPATH

#Set paths
ROOTDIR=$(pwd)
LIBPATH=$(pwd)/../../src
BINPATH=$(pwd)/../../bin
LIBOUT=$(pwd)/buildlib
export INCLUDE=$LIBPATH
export LIBRARY=$LIBPATH

###Make sure lib exists
mkdir buildlib

#####Build lib3ds

#cd $LIBPATH/lib3ds-1.2.0/
#./configure --prefix=$LIBOUT
#make install

#####Build freetype

cd $LIBPATH/freetype-2.1.10/
./configure --prefix=$LIBOUT
make install

#####Build GLUT
cd $LIBPATH/glut-3.7/
#./mkmkfiles.imake
cd lib/glut
make
cp libglut.* $LIBOUT/lib


##### Copy FMOD files to lib/inc dirs
cd $LIBPATH/fmod/inc
cp -f * $LIBOUT/include
cd $LIBPATH/fmod/lib
cp -f * $LIBOUT/lib


###### Use icompile to build Cwapp
# It should take care of the rest building tinyxml, VRG3D
# as dependencies
export CWLIBPATH=$LIBOUT
echo $CWLIBPATH

cd $LIBPATH/cwapp
$G3DINSTALLPATH/bin/icompile . --config $ROOTDIR/config-icompile --noprompt --opt --verbosity 3

###### Copy to bin dir

##force consistent name "linux"
cd $BINPATH
mkdir linux
cp -f -r $LIBPATH/cwapp/build/linux-*/cwapp* linux/

###### Copy dynamic libs to bin/ and set  LD_LIBRARY_PATH -- Ready to run!
cd $BINPATH/linux
mkdir lib
cp $LIBOUT/lib/*.so ./lib/
export LD_LIBRARY_PATH=$BINPATH/linux/lib

