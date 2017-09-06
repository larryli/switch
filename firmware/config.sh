#!/bin/bash
DEST="../../nodemcu/nodemcu-firmware"
cp -rf *.h $DEST/app/include/
cp -rf *.c $DEST/app/modules/
cp -rf *html $DEST/app/modules/eus/
