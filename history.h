/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORG_HISTORY_H
#define KORG_HISTORY_H

#include <calendarsupport/incidencechanger.h>

#include <Akonadi/Item>
#include <Akonadi/Collection>

#include <KCalCore/Incidence>

#include <QObject>
#include <QStack>
#include <QList>

namespace CalendarSupport {
  class Calendar;
}

namespace KOrg {

class History : public QObject
{
  Q_OBJECT
  public:
    History( CalendarSupport::Calendar *, QWidget *parent );

    void recordDelete( const Akonadi::Item & );
    void recordAdd( const Akonadi::Item & );
    void recordEdit( const Akonadi::Item &oldItem,
                     const Akonadi::Item &newItem );
    void startMultiModify( const QString &description );
    void endMultiModify();

  public slots:
    void undo();
    void redo();

  private slots:
    void incidenceChangeFinished( const Akonadi::Item &,
                                  const Akonadi::Item &,
                                  CalendarSupport::IncidenceChanger::WhatChanged,
                                  bool );

    void incidenceAddFinished( const Akonadi::Item &, bool );

    void incidenceDeleteFinished( const Akonadi::Item &, bool );

  signals:
    void undone();
    void redone();

    void undoAvailable( const QString & );
    void redoAvailable( const QString & );

  private:
   class Entry;
   void disableHistory();

   // Called as soon as the modify jobs finish
   void finishUndo( bool success );
   void finishRedo( bool success );

  protected:
    void truncate();
    void addEntry( Entry *entry );

  private:

    class Entry
    {
      public:
        Entry( CalendarSupport::Calendar *, CalendarSupport::IncidenceChanger * );
        virtual ~Entry();

        virtual bool undo() = 0;
        virtual bool redo() = 0;

        virtual QString text() = 0;

        void setItemId( Akonadi::Item::Id );
        Akonadi::Item::Id itemId();

      protected:
        CalendarSupport::Calendar *mCalendar;
        CalendarSupport::IncidenceChanger *mChanger;
        Akonadi::Item::Id mItemId;
    };

    class EntryDelete : public Entry
    {
      public:
        EntryDelete( CalendarSupport::Calendar *, CalendarSupport::IncidenceChanger *,
                     const Akonadi::Item & );

        ~EntryDelete();

        bool undo();
        bool redo();

        QString text();

      private:
        KCalCore::Incidence::Ptr mIncidence;
        Akonadi::Collection mCollection;
        QString mParentUid;
    };

    class EntryAdd : public Entry
    {
      public:
        EntryAdd( CalendarSupport::Calendar *, CalendarSupport::IncidenceChanger *,
                  const Akonadi::Item & );

        ~EntryAdd();

        bool undo();
        bool redo();

        QString text();
      private:
        KCalCore::Incidence::Ptr mIncidence;
        Akonadi::Collection mCollection;
        QString mParentUid;
    };

    class EntryEdit : public Entry
    {
      public:
        EntryEdit( CalendarSupport::Calendar *calendar,
                   CalendarSupport::IncidenceChanger *,
                   const Akonadi::Item &oldItem,
                   const Akonadi::Item &newItem );
        ~EntryEdit();

        bool undo();
        bool redo();

        QString text();

      private:
        KCalCore::Incidence::Ptr mOldIncidence;
        KCalCore::Incidence::Ptr mNewIncidence;
    };

    class MultiEntry : public Entry
    {
      public:
        MultiEntry( CalendarSupport::Calendar *calendar, const QString &text );
        ~MultiEntry();

        void appendEntry( Entry *entry );
        bool undo();
        bool redo();

        QString text();
        void itemCreated( Akonadi::Item::Id );

      private:
        QList<Entry*> mEntries;
        QString mText;
    };

    CalendarSupport::Calendar *mCalendar;
    MultiEntry *mCurrentMultiEntry;
    QWidget *mParent;
    CalendarSupport::IncidenceChanger *mChanger;

    QStack<Entry*> mUndoEntries;
    QStack<Entry*> mRedoEntries;

    // If this is not 0 there's a modify job running that didn't end yet
    Entry *mCurrentRunningEntry;

    enum Operation {
      UNDO,
      REDO
    };

    // When the job ends we must know if we were doing a undo or redo
    Operation mCurrentRunningOperation;
};

}
#endif
