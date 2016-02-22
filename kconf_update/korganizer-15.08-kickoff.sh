#!/bin/sh

kickoffrcname=`qtpaths --locate-file  GenericConfigLocation kickoffrc`
if [ -f "$kickoffrcname" ]; then
   sed -i "s/\/korganizer.desktop/\/org.kde.korganizer.desktop/" $kickoffrcname
fi
