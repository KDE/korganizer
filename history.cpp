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

#include "history.h"

#include <libkcal/calendar.h>
#include <libkcal/incidence.h>

#include <klocale.h>
#include <kdebug.h>

using namespace KCal;
using namespace KOrg;

History::History( KCal::Calendar *calendar )
  : mCalendar( calendar ), mUndoEntry( mEntries ), mRedoEntry( mEntries )
{
  mEntries.setAutoDelete( true );
}

void History::undo()
{
  Entry *entry = mUndoEntry.current();
  if ( !entry ) return;

  entry->undo();
  emit undone();

  emit redoAvailable( entry->text() );

  mRedoEntry = mUndoEntry;
  --mUndoEntry;

  entry = mUndoEntry.current();
  if ( entry ) emit undoAvailable( entry->text() );
  else emit undoAvailable( QString::null );
}

void History::redo()
{
  Entry *entry = mRedoEntry.current();
  if ( !entry ) return;
  
  emit undoAvailable( entry->text() );

  entry->redo();
  emit redone();

  mUndoEntry = mRedoEntry;
  ++mRedoEntry;
  
  entry = mRedoEntry.current();
  if ( entry ) emit redoAvailable( entry->text() );
  else emit redoAvailable( QString::null );
}

void History::truncate()
{
  while ( mUndoEntry.current() != mEntries.last() ) {
    mEntries.removeLast();
  }
  mRedoEntry = QPtrList<Entry>( mEntries );
  emit redoAvailable( QString::null );
}

void History::recordDelete( Incidence *incidence )
{
  truncate();
  Entry *entry = new EntryDelete( mCalendar, incidence );
  mEntries.append( entry );
  mUndoEntry.toLast();
  mRedoEntry = QPtrList<Entry>( mEntries );
  emit undoAvailable( entry->text() );
}

void History::recordAdd( Incidence *incidence )
{
  truncate();
  Entry *entry = new EntryAdd( mCalendar, incidence );
  mEntries.append( entry );
  mUndoEntry.toLast();
  mRedoEntry = QPtrList<Entry>( mEntries );
  emit undoAvailable( entry->text() );
}

void History::recordEdit( Incidence *oldIncidence, Incidence *newIncidence )
{
  truncate();
  Entry *entry = new EntryEdit( mCalendar, oldIncidence, newIncidence );
  mEntries.append( entry );
  mUndoEntry.toLast();
  mRedoEntry = QPtrList<Entry>( mEntries );
  emit undoAvailable( entry->text() );
}


History::Entry::Entry( KCal::Calendar *calendar )
  : mCalendar( calendar )
{
}

History::Entry::~Entry()
{
}

History::EntryDelete::EntryDelete( Calendar *calendar, Incidence *incidence )
  : Entry( calendar ), mIncidence( incidence->clone() )
{
}

History::EntryDelete::~EntryDelete()
{
  delete mIncidence;
}

void History::EntryDelete::undo()
{
  mCalendar->addIncidence( mIncidence->clone() );
}

void History::EntryDelete::redo()
{
  Incidence *incidence = mCalendar->incidence( mIncidence->uid() );
  mCalendar->deleteIncidence( incidence );
}

QString History::EntryDelete::text()
{
  return i18n("Delete %1").arg(mIncidence->type());
}


History::EntryAdd::EntryAdd( Calendar *calendar, Incidence *incidence )
  : Entry( calendar ), mIncidence( incidence->clone() )
{
}

History::EntryAdd::~EntryAdd()
{
  delete mIncidence;
}

void History::EntryAdd::undo()
{
  Incidence *incidence = mCalendar->incidence( mIncidence->uid() );
  mCalendar->deleteIncidence( incidence );
}

void History::EntryAdd::redo()
{
  mCalendar->addIncidence( mIncidence->clone() );
}

QString History::EntryAdd::text()
{
  return i18n("Add %1").arg(mIncidence->type());
}


History::EntryEdit::EntryEdit( Calendar *calendar, Incidence *oldIncidence,
                               Incidence *newIncidence )
  : Entry( calendar ), mOldIncidence( oldIncidence->clone() ),
    mNewIncidence( newIncidence->clone() )
{
}

History::EntryEdit::~EntryEdit()
{
  delete mOldIncidence;
  delete mNewIncidence;
}

void History::EntryEdit::undo()
{
  Incidence *incidence = mCalendar->incidence( mNewIncidence->uid() );
  mCalendar->deleteIncidence( incidence );
  mCalendar->addIncidence( mOldIncidence->clone() );
}

void History::EntryEdit::redo()
{
  Incidence *incidence = mCalendar->incidence( mOldIncidence->uid() );
  mCalendar->deleteIncidence( incidence );
  mCalendar->addIncidence( mNewIncidence->clone() );
}

QString History::EntryEdit::text()
{
  return i18n("Edit %1").arg(mNewIncidence->type());
}

#include "history.moc"
