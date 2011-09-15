#! /bin/sh

ROOT=$(pwd)

#First, extract G3D if it doesn't exist
cd ../../src
if [ ! -d "G3D" ]; then
  unzip G3D-7.01-src.zip
  # Need to create an empty glu.h file as G3D includes it
  touch G3D/GLG3D.lib/include/GLG3D/GL/glu.h
fi

cd $ROOT/CaveWriting
/Developer/usr/bin/xcodebuild -project CaveWriting.xcodeproj -configuration Debug install


