#ifndef KORG_VERSION_H
#define KORG_VERSION_H

/*
  Version scheme: "x.y.z build".

  "x.y.z" follow the kdelibs version KOrganizer is released with.
  
  If "z" is 0, it the version is "x.y"
  
  "build" is empty for final versions. For developer version "build" is
  something like "pre", "alpha1", "alpha2", "beta1", "beta2", "rc1", "rc2".

  Examples in chronological order:
  
    3.0
    3.0.1
    3.1 alpha1
    3.1 beta1
    3.1 beta2
    3.1 rc1
    3.1
    3.1.1
    3.2 pre
    3.2 alpha1
*/

static const char korgVersion[] = "3.3 pre";

#endif
