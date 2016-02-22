#!/bin/sh

kickoffrcname=`kf5-config --path config --locate kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/korganizer.desktop/\/org.kde.korganizer.desktop/" $kickoffrcname
fi
