/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KORGANIZER_JOURNALVIEW_H
#define KORGANIZER_JOURNALVIEW_H
//
// Widget showing one Journal entry

#include <kcal/journal.h>
#include <kcal/listbase.h>
#include <kvbox.h>

#include <QDate>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>

class QLabel;
class QCheckBox;
class QGridLayout;
class KLineEdit;
class KTextBrowser;
class KTimeEdit;
class QPushButton;

namespace KOrg {
  class IncidenceChangerBase;
}
using namespace KOrg;

namespace KCal {
  class Calendar;
}
using namespace KCal;

class JournalView : public QWidget
{
  Q_OBJECT
  public:
    typedef ListBase<JournalView> List;

    JournalView( Journal *j, QWidget *parent );
    virtual ~JournalView();

    void setJournal( Journal * );
    Journal *journal() const { return mJournal; }

    void setCalendar( Calendar *cal );
    QDate date() const { return mDate; }

    void clear();
    void readJournal( Journal *j );

    bool isReadOnly() const { return mReadOnly; }
    void setReadOnly( bool readonly );

  protected slots:
    void setDirty();
    void deleteItem();
    void editItem();
    void printItem();

  public slots:
    void setIncidenceChanger( IncidenceChangerBase *changer ) { mChanger = changer; }
    void setDate( const QDate &date );

  signals:
    void configChanged();
    void deleteIncidence( Incidence * );
    void editIncidence( Incidence * );

  protected:
    void clearFields();
    bool eventFilter( QObject *o, QEvent *e );

  private:
    Journal *mJournal;
    Calendar *mCalendar;
    QDate mDate;
    bool mReadOnly;

    KTextBrowser *mBrowser;
    QPushButton *mEditButton;
    QPushButton *mDeleteButton;
    QPushButton *mPrintButton;

    QGridLayout *mLayout;

    bool mDirty;
    bool mWriteInProgress;
    IncidenceChangerBase *mChanger;
};

class JournalDateView : public KVBox
{
  Q_OBJECT
  public:
    typedef ListBase<JournalDateView> List;

    JournalDateView( Calendar *, QWidget *parent );
    virtual ~JournalDateView();

    void addJournal( Journal * );
    Journal::List journals() const;

    void setDate( const QDate &date );
    QDate date() const { return mDate; }

    void clear();

  signals:
    void setIncidenceChangerSignal( IncidenceChangerBase *changer );
    void setDateSignal( const QDate & );
    void flushEntries();
    void editIncidence( Incidence * );
    void deleteIncidence( Incidence * );
    void newJournal( const QDate & );

  public slots:
    void emitNewJournal();
    void setIncidenceChanger( IncidenceChangerBase *changer );
    void journalEdited( Journal * );
    void journalDeleted( Journal * );

  private:
    Calendar *mCalendar;
    QDate mDate;
    QMap<Journal *,JournalView *> mEntries;

    IncidenceChangerBase *mChanger;
};

#endif // KORGANIZER_JOURNALVIEW_H
