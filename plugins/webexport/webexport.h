#ifndef KORG_WEBEXPORT_H
#define KORG_WEBEXPORT_H
// $Id$

#include <korganizer/part.h>

#include <calendar.h>

class WebExport : public KOrg::Part {
    Q_OBJECT
  public:
    WebExport(KOrganizer *, const char *);
    ~WebExport();
    
    QString info();

  private slots:
    void exportWeb();

  private:
};

#endif
