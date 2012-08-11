/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Mike McQuaid <mike@mikemcquaid.com>

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

#ifndef KORG_VIEWS_JOURNALVIEW_H
#define KORG_VIEWS_JOURNALVIEW_H

// Widget showing one Journal entry
#include <Akonadi/Calendar/IncidenceChanger>
#include <Akonadi/Calendar/ETMCalendar>

#include <QDate>
#include <QFrame>

#include <Akonadi/Item>

#include <KVBox>

class KTextBrowser;

class QPushButton;

class JournalView : public QFrame
{
  Q_OBJECT
  public:
    typedef QList<JournalView*> List;

    JournalView( const Akonadi::Item &journal,
                 const Akonadi::ETMCalendar::Ptr &calendar,
                 QWidget *parent );

    virtual ~JournalView();
    bool eventFilter ( QObject *, QEvent * );

    void setJournal( const Akonadi::Item &journal );
    Akonadi::Item journal() const { return mJournal; }

    void setCalendar( const Akonadi::ETMCalendar::Ptr &cal );
    QDate date() const { return mDate; }

    void clear();
    void readJournal( const Akonadi::Item &journal );

  protected slots:
    void setDirty();
    void deleteItem();
    void editItem();
    void printItem();

  public slots:
    void setIncidenceChanger( Akonadi::IncidenceChanger *changer ) { mChanger = changer; }
    void setDate( const QDate &date );

  signals:
    void configChanged();
    void deleteIncidence( const Akonadi::Item & );
    void editIncidence( const Akonadi::Item & );
    void incidenceSelected( const Akonadi::Item &, const QDate & );

  protected:
    void clearFields();

  private:
    Akonadi::Item mJournal;
    Akonadi::ETMCalendar::Ptr mCalendar;
    QDate mDate;
    bool mReadOnly;

    KTextBrowser *mBrowser;
    QPushButton *mEditButton;
    QPushButton *mDeleteButton;
    QPushButton *mPrintButton;

    bool mDirty;
    bool mWriteInProgress;
    Akonadi::IncidenceChanger *mChanger;
};

class JournalDateView : public KVBox
{
  Q_OBJECT
  public:
    typedef QList<JournalDateView*> List;

    JournalDateView( const Akonadi::ETMCalendar::Ptr &, QWidget *parent );
    virtual ~JournalDateView();

    void addJournal( const Akonadi::Item &journal );
    Akonadi::Item::List journals() const;

    void setDate( const QDate &date );
    QDate date() const { return mDate; }

    void clear();

  signals:
    void setIncidenceChangerSignal( Akonadi::IncidenceChanger *changer );
    void setDateSignal( const QDate & );
    void flushEntries();
    void editIncidence( const Akonadi::Item &journal );
    void deleteIncidence( const Akonadi::Item &journal );
    void newJournal( const QDate & );
    void incidenceSelected( const Akonadi::Item &, const QDate & );

  public slots:
    void emitNewJournal();
    void setIncidenceChanger( Akonadi::IncidenceChanger *changer );
    void journalEdited( const Akonadi::Item & );
    void journalDeleted( const Akonadi::Item & );

  private:
    Akonadi::ETMCalendar::Ptr mCalendar;
    QDate mDate;
    QMap<Akonadi::Item::Id,JournalView *> mEntries;

    Akonadi::IncidenceChanger *mChanger;
};

#endif // KORGANIZER_JOURNALVIEW_H
