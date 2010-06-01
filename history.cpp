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

#include <akonadi/kcal/groupware.h>
#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/utils.h>
#include <akonadi/kcal/incidencechanger.h>

#include <KCal/Incidence>
#include <Akonadi/Item>

#include <klocale.h>

#include <QWidget>

using namespace KCal;
using namespace KOrg;
using namespace Akonadi;

History::History( Akonadi::Calendar *calendar, QWidget *parent )
  : mCalendar( calendar ), mCurrentMultiEntry( 0 ), mParent( parent ),
  mChanger( new Akonadi::IncidenceChanger( calendar, parent, -1 ) ),
  mCurrentRunningEntry( 0 )
{
  // We create a new incidencechanger because we don't want CalendarView receiving it's signals
  // because of stuff we do here. It would cause CalendarView to create a undo entry for an
  // undo

  qRegisterMetaType<Akonadi::Item>( "Akonadi::Item" );
  qRegisterMetaType<Akonadi::IncidenceChanger::WhatChanged>("Akonadi::IncidenceChanger::WhatChanged");  
  connect( mChanger, SIGNAL(incidenceAddFinished(Akonadi::Item,bool)),
           this, SLOT(incidenceAddFinished(Akonadi::Item,bool)) );

  connect( mChanger, SIGNAL(incidenceChangeFinished(Akonadi::Item,Akonadi::Item,Akonadi::IncidenceChanger::WhatChanged,bool)),
           this, SLOT(incidenceChangeFinished(Akonadi::Item,Akonadi::Item,Akonadi::IncidenceChanger::WhatChanged,bool)), Qt::QueuedConnection );

  connect( mChanger, SIGNAL(incidenceDeleteFinished(Akonadi::Item,bool)),
           this, SLOT(incidenceDeleteFinished(Akonadi::Item,bool)) );

  mChanger->setGroupware( Groupware::instance() );
}

void History::undo()
{
  // If mCurrentRunningEntry != 0 there's already a modify job running
  if ( mUndoEntries.isEmpty() || mCurrentRunningEntry ) {
    return;
  }

  Entry *entry = mUndoEntries.pop();
  if ( !entry ) {
    return;
  }

  mCurrentRunningEntry = entry;
  mCurrentRunningOperation = UNDO;
  if ( entry->undo() ) {
    // We only enable it back when the jobs end, in finishUndo
    disableHistory();
  } else {
   finishUndo( false ); 
  }
}

void History::finishUndo( bool success ) {
  mRedoEntries.push( mCurrentRunningEntry );
  emit undone();

  emit redoAvailable( mCurrentRunningEntry->text() );

  if ( !mUndoEntries.isEmpty() ) {
    // We need the text of the next undo item on the stack, which means we need
    // to pop it from the stack to be able to investigate it, and then re-add it.
    Entry *entry = mUndoEntries.pop();
    mUndoEntries.push( entry );

    emit undoAvailable( entry ? entry->text() : QString() );
  } else {
    emit undoAvailable( QString() );
  }

  mCurrentRunningEntry = 0;
}

void History::redo()
{
  // If mCurrentRunningEntry != 0 there's already a modify job running
  if ( mRedoEntries.isEmpty() || mCurrentRunningEntry ) {
    return;
  }
  if ( mCurrentMultiEntry ) {
    mCurrentMultiEntry = 0;
  }
  Entry *entry = mRedoEntries.pop();
  if ( !entry ) {
    return;
  }
  mCurrentRunningOperation = REDO;
  mCurrentRunningEntry = entry;
  if ( entry->redo() ) {
    // We only enable it back when the jobs end, in finishUndo
    disableHistory();
  } else {
    finishRedo( false );
  }
}

void History::finishRedo( bool success )
{
  emit undoAvailable( mCurrentRunningEntry->text() );

  mUndoEntries.push( mCurrentRunningEntry );
  emit redone();

  if ( !mRedoEntries.isEmpty() ) {
    // We need the text of the next redo item on the stack, which means we need
    // to pop it from the stack to be able to investigate it, and then re-add it.
    Entry *entry = mRedoEntries.pop();
    mRedoEntries.push( entry );
    emit redoAvailable( entry ? entry->text() : QString() );
  } else {
    emit redoAvailable( QString() );
  }

  mCurrentRunningEntry = 0;
}

void History::disableHistory()
{
  emit undoAvailable( QString() );
  emit redoAvailable( QString() );
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

void History::recordDelete( const Akonadi::Item &incidence )
{
  addEntry( new EntryDelete( mCalendar, mChanger, incidence ) );
}

void History::recordAdd( const Akonadi::Item &incidence )
{
  addEntry( new EntryAdd( mCalendar, mChanger, incidence ) );
}

void History::recordEdit( const Akonadi::Item &oldItem, const Akonadi::Item &newItem )
{
  addEntry( new EntryEdit( mCalendar, mChanger, oldItem, newItem ) );
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


void History::incidenceChangeFinished( const Akonadi::Item &,
                                       const Akonadi::Item &,
                                       const Akonadi::IncidenceChanger::WhatChanged,
                                       bool success )
{
  if ( mCurrentRunningOperation == UNDO ){
    finishUndo( success );
  } else {
    finishRedo( success );
  }
}

void History::incidenceAddFinished( const Akonadi::Item &item, bool success )
{

  /* When we undo a delete or redo an add, a new item is created but will have
   * a different Id, so we must iterate the stack and update the id so they don't use the old one
   */
  const Akonadi::Item::Id oldId = mCurrentRunningEntry->itemId();
  foreach ( Entry *entry, mUndoEntries ) {
    if ( entry->itemId() == oldId ) {
      entry->setItemId( item.id() );
    }
  }
  foreach ( Entry *entry, mRedoEntries ) {
    if ( entry->itemId() == oldId ) {
      entry->setItemId( item.id() );
    }
  }

  mCurrentRunningEntry->setItemId( item.id() );

  if ( mCurrentRunningOperation == UNDO ) {
    finishUndo( success );
  } else {
    finishRedo( success );
  }
}

void History::incidenceDeleteFinished( const Akonadi::Item &, bool success )
{
  if ( mCurrentRunningOperation == UNDO ) {
    finishUndo( success );
  } else {
    finishRedo( success );
  }
}
History::Entry::Entry( Akonadi::Calendar *calendar, Akonadi::IncidenceChanger *changer )
  : mCalendar( calendar ), mChanger( changer )
{
}

History::Entry::~Entry()
{
}

// could go to KCal, but probably the best is to remove raw pointer usage from KCal
void History::Entry::removeRelations( const Incidence::Ptr &incidence )
{
  const Incidence::List childs = incidence->relations();
  foreach ( Incidence *child, childs ) {
    incidence->removeRelation( child );
  }

  incidence->setRelatedTo( 0 );
}

void History::Entry::setItemId( Akonadi::Item::Id id )
{
  mItemId = id;
}

Akonadi::Item::Id History::Entry::itemId()
{
  return mItemId;
}

History::EntryDelete::EntryDelete( Akonadi::Calendar *calendar,
                                   Akonadi::IncidenceChanger *changer,
                                   const Akonadi::Item &item )
  : Entry( calendar, changer ), mIncidence( Akonadi::incidence( item )->clone() ),
  mCollection( item.parentCollection() )
{
  // Save the parent uid here, because we are going to clear the relations
  // KCal uses raw pointers and we can't have pointers to parents that might be dead already
  //
  // Other problem is that when you clone() a parent incidence it will have pointers to it's
  // children, but all children point to the original father. Should KCal do recursive cloning?
  mParentUid = mIncidence->relatedToUid();
  removeRelations( mIncidence );
  mItemId = item.id();
}

History::EntryDelete::~EntryDelete()
{
}

bool History::EntryDelete::undo()
{
  Incidence::Ptr incidence( mIncidence->clone() );
  incidence->setRelatedToUid( mParentUid );
  return mChanger->addIncidence( incidence, mCollection, 0 );
}

bool History::EntryDelete::redo()
{
  const Akonadi::Item item = mCalendar->incidence( mItemId );
  if ( mChanger->isNotDeleted( item.id() ) ) {
    return mChanger->deleteIncidence( item, 0 );
  } else {
    return true;
  }
}

QString History::EntryDelete::text()
{
  return i18n( "Delete %1", QString::fromLatin1( mIncidence->type() ) );
}

History::EntryAdd::EntryAdd( Akonadi::Calendar *calendar,
                             Akonadi::IncidenceChanger *changer,
                             const Akonadi::Item &item )
  : Entry( calendar, changer ), mIncidence( Akonadi::incidence( item )->clone() ),
  mCollection( item.parentCollection() )
{
  // See comments in EntryDelete's constructor
  mParentUid = mIncidence->relatedToUid();
  removeRelations( mIncidence );
  mItemId = item.id();
}

History::EntryAdd::~EntryAdd()
{
}

bool History::EntryAdd::undo()
{
  const Akonadi::Item item = mCalendar->incidence( mItemId );
  if ( mChanger->isNotDeleted( item.id() ) ) {
    return mChanger->deleteIncidence( item, 0 );
  } else {
    return true;
  }
}

bool History::EntryAdd::redo()
{
  Incidence::Ptr incidence( mIncidence->clone() );
  incidence->setRelatedToUid( mParentUid );
  return mChanger->addIncidence( incidence, mCollection, 0 );
}

QString History::EntryAdd::text()
{
  return i18n( "Add %1", QString::fromLatin1( mIncidence->type() ) );
}

History::EntryEdit::EntryEdit( Akonadi::Calendar *calendar,
                               Akonadi::IncidenceChanger *changer,
                               const Akonadi::Item &oldItem,
                               const Akonadi::Item &newItem )
  : Entry( calendar, changer ),
  mOldIncidence( Akonadi::incidence( oldItem )->clone() ),
  mNewIncidence( Akonadi::incidence( newItem )->clone() )
{
  mItemId = oldItem.id();
  removeRelations( mOldIncidence );
  removeRelations( mNewIncidence );
}

History::EntryEdit::~EntryEdit()
{
}

bool History::EntryEdit::undo()
{
  Akonadi::Item item = mCalendar->incidence( mItemId );
  item.setPayload<KCal::Incidence::Ptr>( mOldIncidence );
  return mChanger->changeIncidence( mNewIncidence, item, Akonadi::IncidenceChanger::UNKNOWN_MODIFIED, 0 );
}

bool History::EntryEdit::redo()
{
  Akonadi::Item item = mCalendar->incidence( mItemId );
  item.setPayload<KCal::Incidence::Ptr>( mNewIncidence );
  return mChanger->changeIncidence( mOldIncidence, item, Akonadi::IncidenceChanger::UNKNOWN_MODIFIED, 0 );
}

QString History::EntryEdit::text()
{
  return i18n( "Edit %1", QString::fromLatin1( mNewIncidence->type() ) );
}

History::MultiEntry::MultiEntry( Akonadi::Calendar *calendar, const QString &text )
  : Entry( calendar, 0 ), mText( text )
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

bool History::MultiEntry::undo()
{
  for ( int i = mEntries.size()-1; i>=0; --i ) {
    Entry *entry = mEntries.at( i );
    if ( entry ) {
      entry->undo();
    }
  }

  // What should we do if one fails? Rollback everything?
  return true;
}

bool History::MultiEntry::redo()
{
  foreach ( Entry *entry, mEntries ) {
    entry->redo();
  }

  // What should we do if one fails? Rollback everything?
  return true;
}

QString History::MultiEntry::text()
{
  return mText;
}

#include "history.moc"
