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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KORG_HISTORY_H
#define KORG_HISTORY_H

#include <qobject.h>
#include <qptrlist.h>

namespace KCal {

class Calendar;
class Incidence;

}

namespace KOrg {

class History : public QObject
{
    Q_OBJECT
  public:
    History( KCal::Calendar * );

    void recordDelete( KCal::Incidence * );
    void recordAdd( KCal::Incidence * );
    void recordEdit( KCal::Incidence *oldIncidence,
                     KCal::Incidence *newIncidence );
    void startMultiModify( const QString &description );
    void endMultiModify();

  public slots:
    void undo();
    void redo();

  signals:
    void undone();
    void redone();

    void undoAvailable( const QString & );
    void redoAvailable( const QString & );

  protected:
    void truncate();

  private:
  
    class Entry
    {
      public:
        Entry( KCal::Calendar * );
        virtual ~Entry();
    
        virtual void undo() = 0;
        virtual void redo() = 0;

        virtual QString text() = 0;

      protected:
        KCal::Calendar *mCalendar;
    };

    class EntryDelete : public Entry
    {
      public:
        EntryDelete( KCal::Calendar *, KCal::Incidence * );
        ~EntryDelete();
        
        void undo();
        void redo();
    
        QString text();
    
      private:
        KCal::Incidence *mIncidence;
    };

    class EntryAdd : public Entry
    {
      public:
        EntryAdd( KCal::Calendar *, KCal::Incidence * );
        ~EntryAdd();
        
        void undo();
        void redo();

        QString text();

      private:
        KCal::Incidence *mIncidence;
    };
    
    class EntryEdit : public Entry
    {
      public:
        EntryEdit( KCal::Calendar *calendar, KCal::Incidence *oldIncidence,
                   KCal::Incidence *newIncidence );
        ~EntryEdit();
        
        void undo();
        void redo();
      
        QString text();
      
      private:
        KCal::Incidence *mOldIncidence;
        KCal::Incidence *mNewIncidence;
    };

    class MultiEntry : public Entry
    {
      public:
        MultiEntry( KCal::Calendar *calendar, const QString &text );
        ~MultiEntry();
        
        void appendEntry( Entry* entry );
        void undo();
        void redo();
      
        QString text();
      
      private:
        QPtrList<Entry> mEntries;
        QString mText;
    };

    KCal::Calendar *mCalendar;
    MultiEntry *mCurrentMultiEntry;

    QPtrList<Entry> mEntries;
    QPtrListIterator<Entry> mUndoEntry;
    QPtrListIterator<Entry> mRedoEntry;
};

}
#endif
