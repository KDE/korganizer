/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
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

#include "history.h"

#include <libkcal/calendar.h>
#include <libkcal/incidence.h>

#include <klocale.h>
#include <kdebug.h>

using namespace KCal;
using namespace KOrg;

History::History( KCal::Calendar *calendar )
  : mCalendar( calendar ), mCurrentMultiEntry( 0 ), 
    mUndoEntry( mEntries ), mRedoEntry( mEntries )
{
  mEntries.setAutoDelete( true );
}

void History::undo()
{
  if ( mCurrentMultiEntry ) mCurrentMultiEntry = 0;
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
  if ( mCurrentMultiEntry ) mCurrentMultiEntry = 0;
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
  Entry *entry = new EntryDelete( mCalendar, incidence );
  if (mCurrentMultiEntry) {
    mCurrentMultiEntry->appendEntry( entry );
  } else {
    truncate();
    mEntries.append( entry );
    mUndoEntry.toLast();
    mRedoEntry = QPtrList<Entry>( mEntries );
    emit undoAvailable( entry->text() );
  }
}

void History::recordAdd( Incidence *incidence )
{
  Entry *entry = new EntryAdd( mCalendar, incidence );
  if (mCurrentMultiEntry) {
    mCurrentMultiEntry->appendEntry( entry );
  } else {
    truncate();
    mEntries.append( entry );
    mUndoEntry.toLast();
    mRedoEntry = QPtrList<Entry>( mEntries );
    emit undoAvailable( entry->text() );
  }
}

void History::recordEdit( Incidence *oldIncidence, Incidence *newIncidence )
{
  Entry *entry = new EntryEdit( mCalendar, oldIncidence, newIncidence );
  if (mCurrentMultiEntry) {
    mCurrentMultiEntry->appendEntry( entry );
  } else {
    truncate();
    mEntries.append( entry );
    mUndoEntry.toLast();
    mRedoEntry = QPtrList<Entry>( mEntries );
    emit undoAvailable( entry->text() );
  }
}

void History::startMultiModify( const QString &description )
{
  if ( mCurrentMultiEntry ) {
    endMultiModify();
  }
  mCurrentMultiEntry = new MultiEntry( mCalendar, description );
  truncate();
  mEntries.append( mCurrentMultiEntry );
  mUndoEntry.toLast();
  mRedoEntry = QPtrList<Entry>( mEntries );
  emit undoAvailable( mCurrentMultiEntry->text() );
}

void History::endMultiModify()
{
  mCurrentMultiEntry = 0;
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
  if ( incidence )
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
  if ( incidence )
      mCalendar->deleteIncidence( incidence );
  mCalendar->addIncidence( mOldIncidence->clone() );
}

void History::EntryEdit::redo()
{
  Incidence *incidence = mCalendar->incidence( mOldIncidence->uid() );
  if ( incidence )
      mCalendar->deleteIncidence( incidence );
  mCalendar->addIncidence( mNewIncidence->clone() );
}

QString History::EntryEdit::text()
{
  return i18n("Edit %1").arg(mNewIncidence->type());
}

History::MultiEntry::MultiEntry( Calendar *calendar, const QString &text )
  : Entry( calendar ), mText( text )
{
  mEntries.setAutoDelete( true );
}

History::MultiEntry::~MultiEntry()
{
}

void History::MultiEntry::appendEntry( Entry* entry )
{
  mEntries.append( entry );
}

void History::MultiEntry::undo()
{
  QPtrListIterator<Entry> it( mEntries );
  it.toLast();
  Entry *entry;
  while ( (entry = it.current()) != 0 ) {
    --it;
    entry->undo();
  }
}

void History::MultiEntry::redo()
{
  QPtrListIterator<Entry> it( mEntries );
  Entry *entry;
  while ( (entry = it.current()) != 0 ) {
    ++it;
    entry->redo();
  }
}

QString History::MultiEntry::text()
{
  return mText;
}

#include "history.moc"
