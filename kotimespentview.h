#ifndef TIMESPENTVIEW_H
#define TIMESPENTVIEW_H

#include <QTextBrowser>

#include <korganizer/baseview.h>

class QDate;
class TimeSpentWidget;
/**
  This view show the time spent on each category.
*/
class KOTimeSpentView : public KOrg::BaseView
{
  Q_OBJECT
  public:
    explicit KOTimeSpentView(Calendar *calendar, QWidget *parent = 0 );
    ~KOTimeSpentView();

    virtual int currentDateCount();
    virtual Incidence::List selectedIncidences() { return Incidence::List(); }
    DateList selectedDates() { return DateList(); }

  public slots:
    virtual void updateView();
    virtual void showDates(const QDate &start, const QDate &end);
    virtual void showIncidences( const Incidence::List &incidenceList );

    void changeIncidenceDisplay(Incidence *, int);

  private:
    TimeSpentWidget *mView;

    QDate mStartDate;
    QDate mEndDate;

    friend class TimeSpentWidget;
};

#endif
