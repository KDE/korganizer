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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "history.h"

#include <kcal/calendar.h>
#include <kcal/incidence.h>

#include <klocale.h>

using namespace KCal;
using namespace KOrg;

History::History( Calendar *calendar )
  : mCalendar( calendar ), mCurrentMultiEntry( 0 )
{
}

void History::undo()
{
  if ( mUndoEntries.isEmpty() ) {
    return;
  }

  Entry *entry = mUndoEntries.pop();
  if ( !entry ) {
    return;
  }

  entry->undo();
  mRedoEntries.push( entry );
  emit undone();

  emit redoAvailable( entry->text() );

  if ( !mUndoEntries.isEmpty() ) {
    // We need the text of the next undo item on the stack, which means we need
    // to pop it from the stack to be able to investigate it, and then re-add it.
    entry = mUndoEntries.pop();
    mUndoEntries.push( entry );
    emit undoAvailable( entry ? entry->text() : QString() );
  } else {
    emit undoAvailable( QString() );
  }
}

void History::redo()
{
  if ( mRedoEntries.isEmpty() ) {
    return;
  }
  if ( mCurrentMultiEntry ) {
    mCurrentMultiEntry = 0;
  }
  Entry *entry = mRedoEntries.pop();
  if ( !entry ) {
    return;
  }

  emit undoAvailable( entry->text() );

  entry->redo();
  mUndoEntries.push( entry );
  emit redone();

  if ( !mRedoEntries.isEmpty() ) {
    // We need the text of the next redo item on the stack, which means we need
    // to pop it from the stack to be able to investigate it, and then re-add it.
    entry = mRedoEntries.pop();
    mRedoEntries.push( entry );
    emit redoAvailable( entry ? entry->text() : QString() );
  } else {
    emit redoAvailable( QString() );
  }
}

void History::truncate()
{
  qDeleteAll( mUndoEntries );
  mUndoEntries.clear();
  emit redoAvailable( QString() );
}

void History::addEntry( Entry *entry )
{
  if ( mCurrentMultiEntry && mCurrentMultiEntry != entry ) {
    mCurrentMultiEntry->appendEntry( entry );
  } else {
    truncate();
    mUndoEntries.push( entry );
    emit undoAvailable( entry->text() );
  }
}

void History::recordDelete( Incidence *incidence )
{
  addEntry( new EntryDelete( mCalendar, incidence ) );
}

void History::recordAdd( Incidence *incidence )
{
  addEntry( new EntryAdd( mCalendar, incidence ) );
}

void History::recordEdit( Incidence *oldIncidence, Incidence *newIncidence )
{
  addEntry( new EntryEdit( mCalendar, oldIncidence, newIncidence ) );
}

void History::startMultiModify( const QString &description )
{
  if ( mCurrentMultiEntry ) {
    endMultiModify();
  }
  mCurrentMultiEntry = new MultiEntry( mCalendar, description );
  addEntry( mCurrentMultiEntry );
}

void History::endMultiModify()
{
  mCurrentMultiEntry = 0;
}

History::Entry::Entry( Calendar *calendar )
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
  // TODO: Use the proper resource instead of asking!
  mCalendar->addIncidence( mIncidence->clone() );
}

void History::EntryDelete::redo()
{
  Incidence *incidence = mCalendar->incidence( mIncidence->uid() );
  mCalendar->deleteIncidence( incidence );
}

QString History::EntryDelete::text()
{
  return i18n( "Delete %1", QString::fromLatin1( mIncidence->type() ) );
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
  if ( incidence ) {
    mCalendar->deleteIncidence( incidence );
  }
}

void History::EntryAdd::redo()
{
  // TODO: User the proper resource instead of asking again
  mCalendar->addIncidence( mIncidence->clone() );
}

QString History::EntryAdd::text()
{
  return i18n( "Add %1", QString::fromLatin1( mIncidence->type() ) );
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
  if ( incidence ) {
    mCalendar->deleteIncidence( incidence );
  }
  // TODO: Use the proper resource instead of asking again
  mCalendar->addIncidence( mOldIncidence->clone() );
}

void History::EntryEdit::redo()
{
  Incidence *incidence = mCalendar->incidence( mOldIncidence->uid() );
  if ( incidence ) {
    mCalendar->deleteIncidence( incidence );
  }
  // TODO: Use the proper resource instead of asking again
  mCalendar->addIncidence( mNewIncidence->clone() );
}

QString History::EntryEdit::text()
{
  return i18n( "Edit %1", QString::fromLatin1( mNewIncidence->type() ) );
}

History::MultiEntry::MultiEntry( Calendar *calendar, const QString &text )
  : Entry( calendar ), mText( text )
{
}

History::MultiEntry::~MultiEntry()
{
  qDeleteAll( mEntries );
}

void History::MultiEntry::appendEntry( Entry *entry )
{
  mEntries.append( entry );
}

void History::MultiEntry::undo()
{
  for ( int i = mEntries.size()-1; i>=0; --i ) {
    Entry *entry = mEntries.at( i );
    if ( entry ) {
      entry->undo();
    }
  }
}

void History::MultiEntry::redo()
{
  foreach ( Entry *entry, mEntries ) {
    entry->redo();
  }
}

QString History::MultiEntry::text()
{
  return mText;
}

#include "history.moc"
