#ifndef JOURNALENTRY_H
#define JOURNALENTRY_H
// $Id$
//
// Widget showing one Journal entry

#include <qframe.h>

#include <calendar.h>

class QLabel;
class QMultiLineEdit;

using namespace KCal;

class JournalEntry : public QFrame {
    Q_OBJECT
  public:
    JournalEntry(Calendar *,QWidget *parent);
    virtual ~JournalEntry();
    
    void setJournal(Journal *);
    Journal *journal() const;

    void setDate(const QDate &);

    void clear();

    void flushEntry();

  protected slots:
    void setDirty();

  protected:    
    bool eventFilter( QObject *o, QEvent *e );

    void writeJournal();
    
  private:
    Calendar *mCalendar;
    Journal *mJournal;
    QDate mDate;
    
    QLabel *mTitleLabel;
    QMultiLineEdit *mEditor;

    bool mDirty;
};

#endif
