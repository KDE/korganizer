#! /bin/sh
$EXTRACTRC `find . -name "*.rc" -o -name "*.ui" -o -name "*.kcfg"` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" -o -name "*.h" | grep -v '/tests/' | grep -v '/autotests/'` -o $podir/korganizer.pot
rm -f rc.cpp
