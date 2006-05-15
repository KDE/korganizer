//#include <stdio.h>
#include "vobject.h"
#include "vcc.h"

void main(int argc, char *argv[])
{
  VObject *t, *v;
  //  FILE *fp;

  //fp = fopen(argv[1], "r");

  v = Parse_MIME_FromFileName(argv[1]);
  while (v) {
    printVObject(stdout, v);
    t = v;
    v = nextVObjectInList(v);
    cleanVObject(t);
  }
}
