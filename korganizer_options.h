#ifndef KORGANIZER_OPTIONS_H
#define KORGANIZER_OPTIONS_H

#include <kcmdlineargs.h>
#include <klocale.h>

static const KCmdLineOptions korganizer_options[] =
{
  { "i", 0, 0 },
  { "import <url>", I18N_NOOP("Import calendar at <url> into default calendar"),
    0 },
  { "+[calendar]", I18N_NOOP("A calendar file to load"), 0 },
  KCmdLineLastOption
};

#endif /* KORGANIZER_OPTIONS_H */

