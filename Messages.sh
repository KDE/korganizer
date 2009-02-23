#! /bin/sh
$PREPARETIPS > tips.cpp
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" -o -name "*.h"` -o $podir/korganizer.pot
rm -f tips.cpp
rm -f rc.cpp
