/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef JOURNALENTRY_H
#define JOURNALENTRY_H
//
// Widget showing one Journal entry

#include <qvbox.h>

class QLabel;
class KActiveLabel;
class KPushButton;
class QCheckBox;
class QGridLayout;
class KLineEdit;
class KTextEdit;
class KTimeEdit;
namespace KOrg {
class IncidenceChangerBase;
}
using namespace KOrg;
namespace KCal {
  class Calendar; 
  class Journal;
}
using namespace KCal;

class JournalEntry : public QWidget {
    Q_OBJECT
  public:
    typedef ListBase<JournalEntry> List;
    
    JournalEntry( Journal* j, QWidget *parent );
    virtual ~JournalEntry();
    
    void setJournal(Journal *);
    Journal *journal() const { return mJournal; }

    QDate date() const { return mDate; }

    void clear();
    void readJournal( Journal *j );

    bool isReadOnly() const { return mReadOnly; }
    void setReadOnly( bool readonly );

  protected slots:
    void setDirty();
    void deleteItem();
  public slots:
    void setIncidenceChanger( IncidenceChangerBase *changer ) { mChanger = changer; }
    void setDate(const QDate &);
    void flushEntry();
    
  signals:
    void deleteIncidence( Incidence * );

  protected:    
    void clearFields();
    bool eventFilter( QObject *o, QEvent *e );

    void writeJournal();
    
  private:
    void writeJournalPrivate( Journal *j );

    Journal *mJournal;
    QDate mDate;
    bool mReadOnly;
    
    QLabel *mTitleLabel;
    KLineEdit *mTitleEdit;
    KTextEdit *mEditor;
    QCheckBox *mTimeCheck;
    KTimeEdit *mTimeEdit;
    KPushButton *mDeleteButton;
    
    QGridLayout *mLayout;

    bool mDirty;
    IncidenceChangerBase *mChanger;
};


class JournalDateEntry : public QVBox {
    Q_OBJECT
  public:
    typedef ListBase<JournalDateEntry> List;
    
    JournalDateEntry( Calendar *, QWidget *parent );
    virtual ~JournalDateEntry();
    
    void addJournal( Journal * );
    Journal::List journals() const;

    void setDate( const QDate & );
    QDate date() const { return mDate; }

    void clear();

    
  signals:
    void setIncidenceChangerSignal( IncidenceChangerBase *changer );
    void setDateSignal( const QDate & );
    void flushEntries();
    void deleteIncidence( Incidence * );
    
  public slots:
    void newJournal();
    void setIncidenceChanger( IncidenceChangerBase *changer );
    void journalEdited( Journal* );
    void journalDeleted( Journal* );
  
  private:
    Calendar *mCalendar;
    QDate mDate;
    QMap<Journal*,JournalEntry*> mEntries;

    KActiveLabel *mTitle;
    QWidget *mAddBar;
    IncidenceChangerBase *mChanger;
};


#endif
