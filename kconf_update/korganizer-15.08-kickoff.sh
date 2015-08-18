#!/bin/sh

sed -i "s/\/korganizer.desktop/\/org.kde.korganizer.desktop/" `kf5-config --path config --locate kickoffrc`
