#ifndef _ARCHIVE_DLG
#define _ARCHIVE_DLG

#include <kdialogbase.h>

class KURLRequester;
class KDateEdit;
class Calendar;

class ArchiveDialog : public KDialogBase
{
    Q_OBJECT
  public:
    ArchiveDialog(Calendar *calendar,QWidget *parent=0, const char *name=0);
    virtual ~ArchiveDialog();

  signals:
    void eventsDeleted();

  protected slots:
    void slotUser1();
    void slotUser2();

  private:
    KURLRequester *mArchiveFile;
    KDateEdit *mDateEdit;
    
    Calendar *mCalendar;
};

#endif
