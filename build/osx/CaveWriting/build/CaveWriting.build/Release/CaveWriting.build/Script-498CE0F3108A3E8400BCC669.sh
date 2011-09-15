#!/bin/sh
install_name_tool -change ./libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib $CONFIGURATION_BUILD_DIR/$CONTENTS_FOLDER_PATH/MacOS/CaveWriting

install_name_tool -change ./libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib $INSTALL_DIR/$CONTENTS_FOLDER_PATH/MacOS/CaveWriting
