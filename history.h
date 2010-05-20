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

#include <Akonadi/Item>
#include <Akonadi/Collection>

#include <KCal/Incidence>

#include <QObject>
#include <QStack>
#include <QList>

#include <boost/shared_ptr.hpp>

namespace Akonadi {
  class Calendar;
  class IncidenceChanger;
}

namespace KOrg {

class History : public QObject
{
  Q_OBJECT
  public:
    explicit History( Akonadi::Calendar *, QWidget *parent );

    void recordDelete( const Akonadi::Item & );
    void recordAdd( const Akonadi::Item & );
    void recordEdit( const Akonadi::Item &oldItem,
                     const Akonadi::Item &newItem );
    void startMultiModify( const QString &description );
    void endMultiModify();
    void setIncidenceChanger( Akonadi::IncidenceChanger *changer );

  public slots:
    void undo();
    void redo();

  signals:
    void undone();
    void redone();

    void undoAvailable( const QString & );
    void redoAvailable( const QString & );

  private:
   class Entry;

  protected:
    void truncate();
    void addEntry( Entry *entry );

  private:

    class Entry
    {
      public:
        Entry( Akonadi::Calendar *, Akonadi::IncidenceChanger * );
        virtual ~Entry();

        virtual void undo() = 0;
        virtual void redo() = 0;

        virtual QString text() = 0;

      protected:
        Akonadi::Calendar *mCalendar;
        Akonadi::IncidenceChanger *mChanger;
    };

    class EntryDelete : public Entry
    {
      public:
        EntryDelete( Akonadi::Calendar *, Akonadi::IncidenceChanger *, const Akonadi::Item & );
        ~EntryDelete();

        void undo();
        void redo();

        QString text();

      private:
        Akonadi::Item::Id mItemId;
        KCal::Incidence::Ptr mIncidence;
        Akonadi::Collection mCollection;
    };

    class EntryAdd : public Entry
    {
      public:
        EntryAdd( Akonadi::Calendar *, Akonadi::IncidenceChanger *, const Akonadi::Item & );
        ~EntryAdd();

        void undo();
        void redo();

        QString text();

      private:
        Akonadi::Item::Id mItemId;
        KCal::Incidence::Ptr mIncidence;
        Akonadi::Collection mCollection;
    };

    class EntryEdit : public Entry
    {
      public:
        EntryEdit( Akonadi::Calendar *calendar,
                   Akonadi::IncidenceChanger *,
                   const Akonadi::Item &oldItem,
                   const Akonadi::Item &newItem );
        ~EntryEdit();

        void undo();
        void redo();

        QString text();

      private:
        KCal::Incidence::Ptr mOldIncidence;
        KCal::Incidence::Ptr mNewIncidence;
        Akonadi::Item::Id mItemId;
    };

    class MultiEntry : public Entry
    {
      public:
        MultiEntry( Akonadi::Calendar *calendar, const QString &text );
        ~MultiEntry();

        void appendEntry( Entry *entry );
        void undo();
        void redo();

        QString text();

      private:
        QList<Entry*> mEntries;
        QString mText;
    };

    Akonadi::Calendar *mCalendar;
    MultiEntry *mCurrentMultiEntry;
    Akonadi::IncidenceChanger *mChanger;
    QWidget *mParent;

    QStack<Entry*> mUndoEntries;
    QStack<Entry*> mRedoEntries;
};

}
#endif
