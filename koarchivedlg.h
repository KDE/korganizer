#ifndef _ARCHIVE_DLG
#define _ARCHIVE_DLG

#include <kdialogbase.h>

class KURLRequester;
class KDateEdit;
class CalObject;

class ArchiveDialog : public KDialogBase
{
    Q_OBJECT
  public:
    ArchiveDialog(CalObject *calendar,QWidget *parent=0, const char *name=0);
    virtual ~ArchiveDialog();

  protected slots:
    void slotUser1();
    void slotUser2();

  private:
    KURLRequester *mArchiveFile;
    KDateEdit *mDateEdit;
    
    CalObject *mCalendar;
};

#endif
