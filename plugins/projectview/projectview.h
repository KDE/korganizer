#ifndef KORG_PROJECTVIEW_H
#define KORG_PROJECTVIEW_H
// $Id$

#include <korganizer/part.h>
#include <korganizer/calendarviewbase.h>

class ProjectView : public KOrg::Part {
    Q_OBJECT
  public:
    ProjectView(KOrg::MainWindow *, const char *);
    ~ProjectView();
    
    QString info();

  private slots:
    void showView();

  private:
    KOrg::BaseView *mView;
};

#endif
