/*
  This file is part of KOrganizer.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2008 Ron Goodheart <rong.dev@gmail.com>
  Copyright (c) 2010 Laurent Montel <montel@kde.org>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

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

#include "calprintdefaultplugins.h"
#include "koglobals.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalCore/Visitor>

#include <KCalUtils/IncidenceFormatter>
#include <KCalUtils/Stringify>

#include <KCalendarSystem>
#include <KDateTime>
#include <KConfigGroup>
#include <KSystemTimeZones>

#include <QDateTime>
#include <QPainter>
#include <QPrinter>

using namespace KCalUtils;

static QString cleanStr( const QString &instr )
{
  QString ret = instr;
  return ret.replace( '\n', ' ' );
}

/**************************************************************
 *           Print Incidence
 **************************************************************/

CalPrintIncidence::CalPrintIncidence() : CalPrintPluginBase()
{
}

CalPrintIncidence::~CalPrintIncidence()
{
}

QWidget *CalPrintIncidence::createConfigWidget( QWidget *w )
{
  return new CalPrintIncidenceConfig( w );
}

void CalPrintIncidence::readSettingsWidget()
{
  CalPrintIncidenceConfig *cfg =
      dynamic_cast<CalPrintIncidenceConfig *>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    mUseColors = cfg->mColors->isChecked();
    mPrintFooter = cfg->mPrintFooter->isChecked();
    mShowOptions = cfg->mShowDetails->isChecked();
    mShowSubitemsNotes = cfg->mShowSubitemsNotes->isChecked();
    mShowAttendees = cfg->mShowAttendees->isChecked();
    mShowAttachments = cfg->mShowAttachments->isChecked();
    mShowNoteLines = cfg->mShowNoteLines->isChecked();
  }
}

void CalPrintIncidence::setSettingsWidget()
{
  CalPrintIncidenceConfig *cfg =
      dynamic_cast<CalPrintIncidenceConfig *>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mColors->setChecked( mUseColors );
    cfg->mPrintFooter->setChecked( mPrintFooter );
    cfg->mShowDetails->setChecked( mShowOptions );
    cfg->mShowSubitemsNotes->setChecked( mShowSubitemsNotes );
    cfg->mShowAttendees->setChecked( mShowAttendees );
    cfg->mShowAttachments->setChecked( mShowAttachments );
    cfg->mShowNoteLines->setChecked( mShowNoteLines );
  }
}

void CalPrintIncidence::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName() );
    mShowOptions = grp.readEntry( "Show Options", false );
    mShowSubitemsNotes = grp.readEntry( "Show Subitems and Notes", false );
    mShowAttendees = grp.readEntry( "Use Attendees", false );
    mShowAttachments = grp.readEntry( "Use Attachments", false );
    mShowNoteLines = grp.readEntry( "Note Lines", false );
  }
  setSettingsWidget();
}

void CalPrintIncidence::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName() );
    grp.writeEntry( "Show Options", mShowOptions );
    grp.writeEntry( "Show Subitems and Notes", mShowSubitemsNotes );
    grp.writeEntry( "Use Attendees", mShowAttendees );
    grp.writeEntry( "Use Attachments", mShowAttachments );
    grp.writeEntry( "Note Lines", mShowNoteLines );
  }
}

class TimePrintStringsVisitor : public Visitor
{
  public:
    TimePrintStringsVisitor() {}

    bool act( IncidenceBase::Ptr incidence )
    {
      return incidence->accept( *this, incidence );
    }
    QString mStartCaption, mStartString;
    QString mEndCaption, mEndString;
    QString mDurationCaption, mDurationString;

  protected:
    bool visit( Event::Ptr event ) {
      if ( event->dtStart().isValid() ) {
        mStartCaption =  i18n( "Start date: " );
        mStartString = IncidenceFormatter::dateTimeToString(
          event->dtStart(), event->allDay(), false );
      } else {
        mStartCaption = i18n( "No start date" );
        mStartString.clear();
      }

      if ( event->hasEndDate() ) {
        mEndCaption = i18n( "End date: " );
        mEndString = IncidenceFormatter::dateTimeToString(
          event->dtEnd(), event->allDay(), false );
      } else if ( event->hasDuration() ) {
        mEndCaption = i18n( "Duration: " );
        int mins = event->duration().asSeconds() / 60;
        if ( mins >= 60 ) {
          mEndString += i18np( "1 hour ", "%1 hours ", mins / 60 );
        }
        if ( mins % 60 > 0 ) {
          mEndString += i18np( "1 minute ", "%1 minutes ", mins % 60 );
        }
      } else {
        mEndCaption = i18n( "No end date" );
        mEndString.clear();
      }
      return true;
    }
  bool visit( Todo::Ptr todo ) {
      if ( todo->hasStartDate() ) {
        mStartCaption =  i18n( "Start date: " );
        mStartString = IncidenceFormatter::dateTimeToString(
          todo->dtStart(), todo->allDay(), false );
      } else {
        mStartCaption = i18n( "No start date" );
        mStartString.clear();
      }

      if ( todo->hasDueDate() ) {
        mEndCaption = i18n( "Due date: " );
        mEndString = IncidenceFormatter::dateTimeToString(
          todo->dtDue(), todo->allDay(), false );
      } else {
        mEndCaption = i18n( "No due date" );
        mEndString.clear();
      }
      return true;
    }
    bool visit( Journal::Ptr journal ) {
      mStartCaption = i18n( "Start date: " );
      mStartString = IncidenceFormatter::dateTimeToString(
        journal->dtStart(), journal->allDay(), false );
      mEndCaption.clear();
      mEndString.clear();
      return true;
    }
    bool visit( FreeBusy::Ptr fb ) {
      Q_UNUSED( fb );
      return true;
    }
};

int CalPrintIncidence::printCaptionAndText( QPainter &p, const QRect &box,
                                            const QString &caption,
                                            const QString &text,
                                            QFont captionFont, QFont textFont )
{
  QFontMetrics captionFM( captionFont );
  int textWd = captionFM.width( caption );
  QRect textRect( box );

  QFont oldFont( p.font() );
  p.setFont( captionFont );
  p.drawText( box, Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, caption );

  if ( !text.isEmpty() ) {
    textRect.setLeft( textRect.left() + textWd );
    p.setFont( textFont );
    p.drawText( textRect, Qt::AlignLeft|Qt::AlignTop|Qt::TextSingleLine, text );
  }
  p.setFont( oldFont );
  return textRect.bottom();
}

void CalPrintIncidence::print( QPainter &p, int width, int height )
{
  QFont oldFont( p.font() );
  QFont textFont( "sans-serif", 11, QFont::Normal );
  QFont captionFont( "sans-serif", 11, QFont::Bold );
  p.setFont( textFont );
  int lineHeight = p.fontMetrics().lineSpacing();
  QString cap, txt;

  Incidence::List::ConstIterator it;
  for ( it = mSelectedIncidences.constBegin(); it != mSelectedIncidences.constEnd(); ++it ) {
    // don't do anything on a 0-pointer!
    if ( !(*it) ) {
      continue;
    }
    if ( it != mSelectedIncidences.constBegin() ) {
      mPrinter->newPage();
    }

    const bool isJournal = ( (*it)->type() == Incidence::TypeJournal );

    //  PAGE Layout (same for landscape and portrait! astonishingly, it looks good with both!):
    //  +-----------------------------------+
    //  | Header:  Summary                  |
    //  +===================================+
    //  | start: ______   end: _________    |
    //  | repeats: ___________________      |
    //  | reminder: __________________      |
    //  +-----------------------------------+
    //  | Location: ______________________  |
    //  +------------------------+----------+
    //  | Description:           | Notes or |
    //  |                        | Subitems |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  |                        |          |
    //  +------------------------+----------+
    //  | Attachments:           | Settings |
    //  |                        |          |
    //  +------------------------+----------+
    //  | Attendees:                        |
    //  |                                   |
    //  +-----------------------------------+
    //  | Categories: _____________________ |
    //  +-----------------------------------+

    QRect box( 0, 0, width, height );
    QRect titleBox( box );
    titleBox.setHeight( headerHeight() );
    QColor headerColor = categoryBgColor( *it );
    // Draw summary as header, no small calendars in title bar, expand height if needed
    int titleBottom = drawHeader( p, (*it)->summary(), QDate(), QDate(),
                                  titleBox, true, headerColor );
    titleBox.setBottom( titleBottom );

    QRect timesBox( titleBox );
    timesBox.setTop( titleBox.bottom() + padding() );
    timesBox.setHeight( height / 8 );

    TimePrintStringsVisitor stringVis;
    int h = timesBox.top();
    if ( stringVis.act( *it ) ) {
      QRect textRect( timesBox.left() + padding(),
                      timesBox.top() + padding(), 0, lineHeight );
      textRect.setRight( timesBox.center().x() );
      h = printCaptionAndText( p, textRect, stringVis.mStartCaption,
                               stringVis.mStartString, captionFont, textFont );

      textRect.setLeft( textRect.right() );
      textRect.setRight( timesBox.right() - padding() );
      h = qMax( printCaptionAndText( p, textRect, stringVis.mEndCaption,
                                     stringVis.mEndString, captionFont, textFont ), h );
    }

    // Recurrence Printing
    if ( (*it)->recurs() ) {
      QRect recurBox( timesBox.left() + padding(), h + padding(),
                      timesBox.right() - padding(), lineHeight );
      KCalCore::Recurrence *recurs = (*it)->recurrence();
      QString displayString = IncidenceFormatter::recurrenceString((*it));
      // exception dates
      QString exceptString;
      if ( !recurs->exDates().isEmpty() ) {
        exceptString = i18nc( "except for listed dates", " except" );
        for ( int i = 0; i < recurs->exDates().size(); i++ ) {
          exceptString.append( " " );
          exceptString.append( KGlobal::locale()->formatDate( recurs->exDates()[i],
                                                              KLocale::ShortDate ) );
        }
      }
      displayString.append( exceptString );
      h = qMax( printCaptionAndText( p, recurBox, i18n( "Repeats: " ),
                                    displayString, captionFont, textFont ), h );
    }

    if ( !isJournal ) {
    // Alarms Printing
    QRect alarmBox( timesBox.left() + padding(), h + padding(),
                    timesBox.right() - padding(), lineHeight );
    Alarm::List alarms = (*it)->alarms();
    if ( alarms.count() == 0 ) {
      cap = i18n( "No reminders" );
      txt.clear();
    } else {
      cap = i18np( "Reminder: ", "%1 reminders: ", alarms.count() );

      QStringList alarmStrings;
      KCalCore::Alarm::List::ConstIterator it;
      for ( it = alarms.constBegin(); it != alarms.constEnd(); ++it ) {
        Alarm::Ptr alarm = *it;

        // Alarm offset, copied from koeditoralarms.cpp:
        KLocalizedString offsetstr;
        int offset = 0;
        if ( alarm->hasStartOffset() ) {
          offset = alarm->startOffset().asSeconds();
          if ( offset < 0 ) {
            offsetstr = ki18nc( "N days/hours/minutes before/after the start/end",
                                "%1 before the start" );
            offset = -offset;
          } else {
            offsetstr = ki18nc( "N days/hours/minutes before/after the start/end",
                                "%1 after the start" );
          }
        } else if ( alarm->hasEndOffset() ) {
          offset = alarm->endOffset().asSeconds();
          if ( offset < 0 ) {
            offsetstr = ki18nc( "N days/hours/minutes before/after the start/end",
                                "%1 before the end" );
            offset = -offset;
          } else {
            offsetstr = ki18nc( "N days/hours/minutes before/after the start/end",
                                "%1 after the end" );
          }
        }

        offset = offset / 60; // make minutes
        int useoffset = offset;

        if ( offset % ( 24 * 60 ) == 0 && offset > 0 ) { // divides evenly into days?
          useoffset = offset / ( 24 * 60 );
          offsetstr = offsetstr.subs( i18np( "1 day", "%1 days", useoffset ) );
        } else if ( offset % 60 == 0 && offset > 0 ) { // divides evenly into hours?
          useoffset = offset / 60;
          offsetstr = offsetstr.subs( i18np( "1 hour", "%1 hours", useoffset ) );
        } else {
          useoffset = offset;
          offsetstr = offsetstr.subs( i18np( "1 minute", "%1 minutes", useoffset ) );
        }
        alarmStrings << offsetstr.toString();
      }
      txt = alarmStrings.join( i18nc( "Spacer for the joined list of categories", ", " ) );

    }
    h = qMax( printCaptionAndText( p, alarmBox, cap, txt, captionFont, textFont ), h );
    }
    QRect organizerBox( timesBox.left() + padding(), h + padding(),
                        timesBox.right() - padding(), lineHeight );
    h = qMax( printCaptionAndText( p, organizerBox, i18n( "Organizer: " ),
                                   (*it)->organizer()->fullName(), captionFont, textFont ), h );

    // Finally, draw the frame around the time information...
    timesBox.setBottom( qMax( timesBox.bottom(), h + padding() ) );
    drawBox( p, BOX_BORDER_WIDTH, timesBox );

    QRect locationBox( timesBox );
    locationBox.setTop( timesBox.bottom() + padding() );
    locationBox.setHeight( 0 );
    int locationBottom = 0;
    if ( !isJournal ) {
      locationBottom = drawBoxWithCaption( p, locationBox, i18n( "Location: " ),
                                           (*it)->location(), /*sameLine=*/true,
                                           /*expand=*/true, captionFont, textFont );
    }
    locationBox.setBottom( locationBottom );

    // Now start constructing the boxes from the bottom:
    QRect footerBox( locationBox );
    footerBox.setBottom( box.bottom() );
    footerBox.setTop( footerBox.bottom() - lineHeight - 2 * padding() );

    QRect categoriesBox( footerBox );
    categoriesBox.setBottom( footerBox.top() );
    categoriesBox.setTop( categoriesBox.bottom() - lineHeight - 2 * padding() );
    QRect attendeesBox( box.left(), categoriesBox.top() - padding() - box.height() / 9,
                        box.width(), box.height() / 9 );
    QRect attachmentsBox( box.left(), attendeesBox.top() - padding() - box.height() / 9,
                          box.width() * 3 / 4 - padding(), box.height() / 9 );
    QRect optionsBox( isJournal ? box.left() :  attachmentsBox.right() + padding(),
                      attachmentsBox.top(), 0, 0 );
    optionsBox.setRight( box.right() );
    optionsBox.setBottom( attachmentsBox.bottom() );
    QRect notesBox( optionsBox.left(),
                    isJournal ? ( timesBox.bottom() + padding() ) :
                                ( locationBox.bottom() + padding() ),
                    optionsBox.width(), 0 );
    notesBox.setBottom( optionsBox.top() - padding() );
    QRect descriptionBox( notesBox );
    descriptionBox.setLeft( box.left() );
    descriptionBox.setRight( attachmentsBox.right() );

    // Adjust boxes depending on the show options...
    if ( !mShowSubitemsNotes || isJournal ) {
      descriptionBox.setRight( box.right() );
    }
    if ( !mShowAttachments || !mShowAttendees ) {
        descriptionBox.setBottom( attachmentsBox.bottom() );
        optionsBox.setTop( attendeesBox.top() );
        optionsBox.setBottom( attendeesBox.bottom() );
        notesBox.setBottom( attachmentsBox.bottom() );
        if ( mShowOptions ) {
          attendeesBox.setRight( attachmentsBox.right() );
        }
      if ( !mShowAttachments && !mShowAttendees ) {
        if ( mShowSubitemsNotes ) {
          descriptionBox.setBottom( attendeesBox.bottom() );
        }
        if ( !mShowOptions ) {
          descriptionBox.setBottom( attendeesBox.bottom() );
          notesBox.setBottom( attendeesBox.bottom() );
        }
      }
    }
    if ( mShowAttachments && !isJournal ) {
      if ( !mShowOptions ) {
        attachmentsBox.setRight( box.right() );
        attachmentsBox.setRight( box.right() );
      }
      if ( !mShowAttendees ) {
        attachmentsBox.setTop( attendeesBox.top() );
        attachmentsBox.setBottom( attendeesBox.bottom() );
      }
    }
    int newBottom = drawBoxWithCaption( p, descriptionBox, i18n( "Description:" ),
                                        (*it)->description(), /*sameLine=*/false,
                                        /*expand=*/false, captionFont, textFont,
                                        (*it)->descriptionIsRich() );
    if ( mShowNoteLines ) {
      drawNoteLines( p, descriptionBox, newBottom );
    }

    Akonadi::Item item = mCalendar->item( (*it)->uid() );
    Akonadi::Item::List relations = mCalendar->childItems( item.id() );

    if ( mShowSubitemsNotes && !isJournal ) {
      if ( relations.isEmpty() || (*it)->type() != Incidence::TypeTodo ) {
        int notesPosition = drawBoxWithCaption( p, notesBox, i18n( "Notes:" ),
                                                QString(), /*sameLine=*/false,
                                                /*expand=*/false, captionFont, textFont );
        drawNoteLines( p, notesBox, notesPosition );
      } else {

        QString subitemCaption;
        if ( relations.count() == 0 ) {
          subitemCaption = i18n( "No Subitems" );
          txt.clear();
        } else {
          subitemCaption = i18np( "1 Subitem:",
                                  "%1 Subitems:",
                                  relations.count() );
        }

        QString subitemString;
        QString statusString;
        QString datesString;
        int count = 0;
        foreach ( const Akonadi::Item &item, relations ) {
          Todo::Ptr todo = CalendarSupport::todo( item );
          ++count;
          if ( !todo ) { // defensive, skip any zero pointers
            continue;
          }
          // format the status
          statusString = Stringify::incidenceStatus( todo->status() );
          if ( statusString.isEmpty() ) {
            if ( todo->status() == Incidence::StatusNone ) {
              statusString = i18nc( "no status", "none" );
            } else {
              statusString = i18nc( "unknown status", "unknown" );
            }
          }
          // format the dates if provided
          datesString.clear();
          KDateTime::Spec spec = CalendarSupport::KCalPrefs::instance()->timeSpec();
          if ( todo->dtStart().isValid() ) {
            datesString += i18nc(
              "subitem start date", "Start Date: %1\n",
              KGlobal::locale()->formatDate( todo->dtStart().toTimeSpec( spec ).date(),
                                             KLocale::ShortDate ) );
            if ( !todo->allDay() ) {
              datesString += i18nc(
                "subitem start time", "Start Time: %1\n",
                KGlobal::locale()->formatTime( todo->dtStart().toTimeSpec( spec ).time(),
                                               false, false ) );
            }
          }
          if ( todo->dateTime( Incidence::RoleEnd ).isValid() ) {
            subitemString +=
              i18nc( "subitem due date", "Due Date: %1\n",
                     KGlobal::locale()->formatDate(
                       todo->dateTime( Incidence::RoleEnd ).toTimeSpec( spec ).date(),
                       KLocale::ShortDate ) );

            if ( !todo->allDay() ) {
              subitemString += i18nc(
                "subitem due time", "Due Time: %1\n",
                KGlobal::locale()->formatTime(
                  todo->dateTime( Incidence::RoleEnd ).toTimeSpec( spec ).time(),
                  false, false ) );
            }
          }
          subitemString += i18nc( "subitem counter", "%1: ", count );
          subitemString += todo->summary();
          subitemString += '\n';
          if ( !datesString.isEmpty() ) {
            subitemString += datesString;
            subitemString += '\n';
          }
          subitemString += i18nc( "subitem Status: statusString",
                                  "Status: %1\n",
                                   statusString );
          subitemString += IncidenceFormatter::recurrenceString( todo ) + '\n';
          subitemString += i18nc( "subitem Priority: N",
                                  "Priority: <numid>%1</numid>\n",
                                  todo->priority() );
          subitemString += i18nc( "subitem Secrecy: secrecyString",
                                  "Secrecy: %1\n",
                                  Stringify::incidenceSecrecy( todo->secrecy() ) );
          subitemString += '\n';
        }
        drawBoxWithCaption( p, notesBox, subitemCaption,
                  subitemString, /*sameLine=*/false,
                            /*expand=*/false, captionFont, textFont );
      }
    }

    if ( mShowAttachments && !isJournal ) {
      Attachment::List attachments = (*it)->attachments();
      QString attachmentCaption;
      if ( attachments.count() == 0 ) {
        attachmentCaption = i18n( "No Attachments" );
        txt.clear();
      } else {
        attachmentCaption = i18np( "1 Attachment:",
                                   "%1 Attachments:",
                                   attachments.count() );
      }
      QString attachmentString;
      Attachment::List::ConstIterator ait = attachments.constBegin();
      for ( ; ait != attachments.constEnd(); ++ait ) {
        if ( !attachmentString.isEmpty() ) {
          attachmentString += i18nc( "Spacer for list of attachments", "  " );
        }
        attachmentString.append( (*ait)->label() );
      }
      drawBoxWithCaption( p, attachmentsBox,
                          attachmentCaption, attachmentString,
                          /*sameLine=*/false, /*expand=*/false,
                          captionFont, textFont );
    }
    if ( mShowAttendees ) {
      Attendee::List attendees = (*it)->attendees();
      QString attendeeCaption;
      if ( attendees.count() == 0 ) {
        attendeeCaption = i18n( "No Attendees" );
      } else {
        attendeeCaption = i18np( "1 Attendee:",
                                 "%1 Attendees:",
                                 attendees.count() );
      }
      QString attendeeString;
      Attendee::List::ConstIterator ait = attendees.constBegin();
      for ( ; ait != attendees.constEnd(); ++ait ) {
        if ( !attendeeString.isEmpty() ) {
          attendeeString += '\n';
        }
        attendeeString += i18nc(
                "Formatting of an attendee: "
                "'Name (Role): Status', e.g. 'Reinhold Kainhofer "
                "<reinhold@kainhofer.com> (Participant): Awaiting Response'",
                "%1 (%2): %3",
                (*ait)->fullName(),
                Stringify::attendeeRole( (*ait)->role() ),
                Stringify::attendeeStatus( ( *ait )->status() ) );
      }
      drawBoxWithCaption( p, attendeesBox, attendeeCaption, attendeeString,
                          /*sameLine=*/false, /*expand=*/false,
                          captionFont, textFont );
    }

    if ( mShowOptions ) {
      QString optionsString;
      if ( !Stringify::incidenceStatus( (*it)->status() ).isEmpty() ) {
        optionsString += i18n( "Status: %1", Stringify::incidenceStatus( (*it)->status() ) );
        optionsString += '\n';
      }
      if ( !Stringify::incidenceSecrecy( (*it)->secrecy() ).isEmpty() ) {
        optionsString += i18n( "Secrecy: %1", Stringify::incidenceSecrecy( (*it)->secrecy() ) );
        optionsString += '\n';
      }
      if ( (*it)->type() == Incidence::TypeEvent ) {
        Event::Ptr e = (*it).staticCast<Event>();
        if ( e->transparency() == Event::Opaque ) {
          optionsString += i18n( "Show as: Busy" );
        } else {
          optionsString += i18n( "Show as: Free" );
        }
        optionsString += '\n';
      } else if ( (*it)->type() == Incidence::TypeTodo ) {
        Todo::Ptr t = (*it).staticCast<Todo>();
        if ( t->isOverdue() ) {
          optionsString += i18n( "This task is overdue!" );
          optionsString += '\n';
        }
      } else if ( (*it)->type() == Incidence::TypeJournal ) {
        //TODO: Anything Journal-specific?
      }
      drawBoxWithCaption( p, optionsBox, i18n( "Settings: " ),
             optionsString, /*sameLine=*/false, /*expand=*/false,
             captionFont, textFont );
    }

    drawBoxWithCaption( p, categoriesBox, i18n( "Categories: " ),
           (*it)->categories().join( i18nc( "Spacer for the joined list of categories", ", " ) ),
           /*sameLine=*/true, /*expand=*/false, captionFont, textFont );

    if ( mPrintFooter ) {
      drawFooter( p, footerBox );
    }
  }
  p.setFont( oldFont );
}

/**************************************************************
 *           Print Day
 **************************************************************/

CalPrintDay::CalPrintDay() : CalPrintPluginBase()
{
}

CalPrintDay::~CalPrintDay()
{
}

QWidget *CalPrintDay::createConfigWidget( QWidget *w )
{
  return new CalPrintDayConfig( w );
}

void CalPrintDay::readSettingsWidget()
{
  CalPrintDayConfig *cfg =
      dynamic_cast<CalPrintDayConfig *>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    if ( cfg->mPrintTypeFilofax->isChecked() ) {
      mDayPrintType = Filofax;
    } else if ( cfg->mPrintTypeTimetable->isChecked() ) {
      mDayPrintType = Timetable;
    } else {
      mDayPrintType = SingleTimetable;
    }

    mStartTime = cfg->mFromTime->time();
    mEndTime = cfg->mToTime->time();
    mIncludeAllEvents = cfg->mIncludeAllEvents->isChecked();

    mIncludeDescription = cfg->mIncludeDescription->isChecked();
    mSingleLineLimit = cfg->mSingleLineLimit->isChecked();
    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mUseColors = cfg->mColors->isChecked();
    mPrintFooter = cfg->mPrintFooter->isChecked();
    mShowNoteLines = cfg->mShowNoteLines->isChecked();
    mExcludeTime = cfg->mExcludeTime->isChecked();
    mExcludeConfidential = cfg->mExcludeConfidential->isChecked();
    mExcludePrivate = cfg->mExcludePrivate->isChecked();
  }
}

void CalPrintDay::setSettingsWidget()
{
  CalPrintDayConfig *cfg = dynamic_cast<CalPrintDayConfig *>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mPrintTypeFilofax->setChecked( mDayPrintType == Filofax );
    cfg->mPrintTypeTimetable->setChecked( mDayPrintType == Timetable );
    cfg->mPrintTypeSingleTimetable->setChecked( mDayPrintType == SingleTimetable );

    cfg->mFromTime->setTime( mStartTime );
    cfg->mToTime->setTime( mEndTime );
    cfg->mIncludeAllEvents->setChecked( mIncludeAllEvents );

    cfg->mIncludeDescription->setChecked( mIncludeDescription );
    cfg->mSingleLineLimit->setChecked( mSingleLineLimit );
    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mColors->setChecked( mUseColors );
    cfg->mPrintFooter->setChecked( mPrintFooter );
    cfg->mShowNoteLines->setChecked( mShowNoteLines );
    cfg->mExcludeTime->setChecked( mExcludeTime );
    cfg->mExcludeConfidential->setChecked( mExcludeConfidential );
    cfg->mExcludePrivate->setChecked( mExcludePrivate );
  }
}

void CalPrintDay::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName() );
    QDate dt = QDate::currentDate(); // any valid QDate will do
    QTime tm1( dayStart() );
    QDateTime startTm( dt, tm1 );
    QDateTime endTm( dt, tm1.addSecs( 12 * 60 * 60 ) );
    mStartTime = grp.readEntry( "Start time", startTm ).time();
    mEndTime = grp.readEntry( "End time", endTm ).time();
    mIncludeDescription = grp.readEntry( "Include description", false );
    mIncludeTodos = grp.readEntry( "Include todos", false );
    mIncludeAllEvents = grp.readEntry( "Include all events", false );
    mDayPrintType = (eDayPrintType)( grp.readEntry( "Print type", (int)Timetable ) );
    mSingleLineLimit = grp.readEntry( "Single line limit", false );
    mShowNoteLines = grp.readEntry( "Note Lines", false );
    mExcludeTime = grp.readEntry( "Exclude time", false );
  }
  setSettingsWidget();
}

void CalPrintDay::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName() );
    QDateTime dt = QDateTime::currentDateTime(); // any valid QDateTime will do
    dt.setTime( mStartTime );
    grp.writeEntry( "Start time", dt );
    dt.setTime( mEndTime );
    grp.writeEntry( "End time", dt );
    grp.writeEntry( "Include description", mIncludeDescription );
    grp.writeEntry( "Include todos", mIncludeTodos );
    grp.writeEntry( "Include all events", mIncludeAllEvents );
    grp.writeEntry( "Print type", int( mDayPrintType ) );
    grp.writeEntry( "Single line limit", mSingleLineLimit );
    grp.writeEntry( "Note Lines", mShowNoteLines );
    grp.writeEntry( "Exclude time", mExcludeTime );
  }
}

void CalPrintDay::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintDayConfig *cfg =
      dynamic_cast<CalPrintDayConfig *>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintDay::print( QPainter &p, int width, int height )
{
  QDate curDay( mFromDate );

  KDateTime::Spec timeSpec = KSystemTimeZones::local();

  QRect headerBox( 0, 0, width, headerHeight() );
  QRect footerBox( 0, height - footerHeight(), width, footerHeight() );
  height -= footerHeight();

  KLocale *local = KGlobal::locale();

  switch ( mDayPrintType ) {
  case Filofax:
  case SingleTimetable:
    {
      QRect daysBox( headerBox );
      daysBox.setTop( headerBox.bottom() + padding() );
      daysBox.setBottom( height );
      QString line1 = local->formatDate( mFromDate );
      QString line2 = local->formatDate( mToDate );
      QString title;
      if ( orientation() == QPrinter::Landscape ) {
        title = i18nc( "date from-to", "%1 - %2", line1, line2 );
      } else {
        title = i18nc( "date from-\nto", "%1 -\n%2", line1, line2 );
      }
      drawHeader( p, title, mFromDate, QDate(), headerBox );
      if ( mDayPrintType == Filofax ) {
        drawDays( p, mFromDate, mToDate, mStartTime, mEndTime, daysBox,
                  mSingleLineLimit,  mShowNoteLines,
                  mIncludeDescription, mExcludeConfidential, mExcludePrivate );
      } else if ( mDayPrintType == SingleTimetable ) {
        drawTimeTable( p, mFromDate, mToDate,
                       mIncludeAllEvents, mStartTime, mEndTime, daysBox,
                       mIncludeDescription, mExcludeTime, mExcludeConfidential,
                       mExcludePrivate );
      }
      if ( mPrintFooter ) {
        drawFooter( p, footerBox );
      }
    }
    break;

  case Timetable:
  default:
  do {
    QTime curStartTime( mStartTime );
    QTime curEndTime( mEndTime );

    // For an invalid time range, simply show one hour, starting at the hour
    // before the given start time
    if ( curEndTime <= curStartTime ) {
      curStartTime = QTime( curStartTime.hour(), 0, 0 );
      curEndTime = curStartTime.addSecs( 3600 );
    }

    drawHeader( p, local->formatDate( curDay ), curDay, QDate(), headerBox );
    KCalCore::Event::List eventList = mCalendar->events( curDay, timeSpec,
                                                         KCalCore::EventSortStartDate,
                                                         KCalCore::SortDirectionAscending );

    // split out the all day events as they will be printed in a separate box
    KCalCore::Event::List alldayEvents, timedEvents;
    foreach( const KCalCore::Event::Ptr &event, eventList ) {
      if ( event->allDay() ) {
        alldayEvents.append( event );
      } else {
        timedEvents.append( event );
      }
    }

    int fontSize = 11;
    QFont textFont( "sans-serif", fontSize, QFont::Normal );
    p.setFont( textFont );
    int lineSpacing = p.fontMetrics().lineSpacing();

    int maxAllDayEvents = 8; // the max we allow to be printed, sorry.
    int allDayHeight = qMin( alldayEvents.count(), maxAllDayEvents ) * lineSpacing;
    allDayHeight = qMax( allDayHeight, ( 5 * lineSpacing ) ) + ( 2 * padding() );
    QRect allDayBox( TIMELINE_WIDTH + padding(), headerBox.bottom() + padding(),
                     width - TIMELINE_WIDTH - padding(), allDayHeight );
    if ( alldayEvents.count() > 0 ) {
      // draw the side bar for all-day events
      QFont oldFont( p.font() );
      p.setFont( QFont( "sans-serif", 9, QFont::Normal ) );
      drawVerticalBox( p,
                       BOX_BORDER_WIDTH,
                       QRect( 0, headerBox.bottom() + padding(), TIMELINE_WIDTH, allDayHeight ),
                       i18n( "Today's Events" ),
                       Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap );
      p.setFont( oldFont );

      // now draw at most maxAllDayEvents in the all-day box
      drawBox( p, BOX_BORDER_WIDTH, allDayBox );

      QRect eventBox( allDayBox );
      eventBox.setLeft( TIMELINE_WIDTH + ( 2 * padding() ) );
      eventBox.setTop( eventBox.top() + padding() );
      eventBox.setBottom( eventBox.top() + lineSpacing );
      int count = 0;
      foreach ( const KCalCore::Event::Ptr &event, alldayEvents ) {
        if ( count == maxAllDayEvents ) {
          break;
        }
        count++;
        QString str;
        if ( event->location().isEmpty() ) {
          str = cleanStr( event->summary() );
        } else {
          str = i18nc( "summary, location", "%1, %2",
                       cleanStr( event->summary() ), cleanStr( event->location() ) );
        }
        printEventString( p, eventBox, str );
        eventBox.setTop( eventBox.bottom() );
        eventBox.setBottom( eventBox.top() + lineSpacing );
      }
    } else {
      allDayBox.setBottom( headerBox.bottom() );
    }

    QRect dayBox( allDayBox );
    dayBox.setTop( allDayBox.bottom() + padding() );
    dayBox.setBottom( height );
    QList<QDate> workDays = KOGlobals::self()->workDays( curDay, curDay );
    drawAgendaDayBox( p, timedEvents, curDay, mIncludeAllEvents,
                      curStartTime, curEndTime, dayBox,
                      mIncludeDescription, mExcludeTime,
                      mExcludeConfidential, mExcludePrivate,
                      workDays );

    QRect tlBox( dayBox );
    tlBox.setLeft( 0 );
    tlBox.setWidth( TIMELINE_WIDTH );
    drawTimeLine( p, curStartTime, curEndTime, tlBox );

    if ( mPrintFooter ) {
      drawFooter( p, footerBox );
    }

    curDay = curDay.addDays( 1 );
    if ( curDay <= mToDate ) {
      mPrinter->newPage();
    }
  } while ( curDay <= mToDate );
  } //switch
}

/**************************************************************
 *           Print Week
 **************************************************************/

CalPrintWeek::CalPrintWeek() : CalPrintPluginBase()
{
}

CalPrintWeek::~CalPrintWeek()
{
}

QWidget *CalPrintWeek::createConfigWidget( QWidget *w )
{
  return new CalPrintWeekConfig( w );
}

void CalPrintWeek::readSettingsWidget()
{
  CalPrintWeekConfig *cfg = dynamic_cast<CalPrintWeekConfig *>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    if ( cfg->mPrintTypeFilofax->isChecked() ) {
      mWeekPrintType = Filofax;
    } else if ( cfg->mPrintTypeTimetable->isChecked() ) {
      mWeekPrintType = Timetable;
    } else if ( cfg->mPrintTypeSplitWeek->isChecked() ) {
      mWeekPrintType = SplitWeek;
    } else {
      mWeekPrintType = Timetable;
    }

    mStartTime = cfg->mFromTime->time();
    mEndTime = cfg->mToTime->time();

    mShowNoteLines = cfg->mShowNoteLines->isChecked();
    mSingleLineLimit = cfg->mSingleLineLimit->isChecked();
    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mUseColors = cfg->mColors->isChecked();
    mPrintFooter = cfg->mPrintFooter->isChecked();
    mIncludeDescription = cfg->mIncludeDescription->isChecked();
    mExcludeTime = cfg->mExcludeTime->isChecked();
    mExcludeConfidential = cfg->mExcludeConfidential->isChecked();
    mExcludePrivate = cfg->mExcludePrivate->isChecked();
  }
}

void CalPrintWeek::setSettingsWidget()
{
  CalPrintWeekConfig *cfg = dynamic_cast<CalPrintWeekConfig *>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mPrintTypeFilofax->setChecked( mWeekPrintType == Filofax );
    cfg->mPrintTypeTimetable->setChecked( mWeekPrintType == Timetable );
    cfg->mPrintTypeSplitWeek->setChecked( mWeekPrintType == SplitWeek );

    cfg->mFromTime->setTime( mStartTime );
    cfg->mToTime->setTime( mEndTime );

    cfg->mShowNoteLines->setChecked( mShowNoteLines );
    cfg->mSingleLineLimit->setChecked( mSingleLineLimit );
    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mColors->setChecked( mUseColors );
    cfg->mPrintFooter->setChecked( mPrintFooter );
    cfg->mIncludeDescription->setChecked( mIncludeDescription );
    cfg->mExcludeTime->setChecked( mExcludeTime );
    cfg->mExcludeConfidential->setChecked( mExcludeConfidential );
    cfg->mExcludePrivate->setChecked( mExcludePrivate );
  }
}

void CalPrintWeek::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName() );
    QDate dt = QDate::currentDate(); // any valid QDate will do
    QTime tm1( dayStart() );
    QDateTime startTm( dt, tm1 );
    QDateTime endTm( dt, tm1.addSecs( 43200 ) );
    mStartTime = grp.readEntry( "Start time", startTm ).time();
    mEndTime = grp.readEntry( "End time", endTm ).time();
    mShowNoteLines = grp.readEntry( "Note Lines", false );
    mSingleLineLimit = grp.readEntry( "Single line limit", false );
    mIncludeTodos = grp.readEntry( "Include todos", false );
    mWeekPrintType = (eWeekPrintType)( grp.readEntry( "Print type", (int)Filofax ) );
    mIncludeDescription = grp.readEntry( "Include Description", false );
    mExcludeTime = grp.readEntry( "Exclude Time", false );
  }
  setSettingsWidget();
}

void CalPrintWeek::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName() );
    QDateTime dt = QDateTime::currentDateTime(); // any valid QDateTime will do
    dt.setTime( mStartTime );
    grp.writeEntry( "Start time", dt );
    dt.setTime( mEndTime );
    grp.writeEntry( "End time", dt );
    grp.writeEntry( "Note Lines", mShowNoteLines );
    grp.writeEntry( "Single line limit", mSingleLineLimit );
    grp.writeEntry( "Include todos", mIncludeTodos );
    grp.writeEntry( "Print type", int( mWeekPrintType ) );
    grp.writeEntry( "Include Description", mIncludeDescription );
    grp.writeEntry( "Exclude Time", mExcludeTime );
  }
}

QPrinter::Orientation CalPrintWeek::defaultOrientation()
{
  if ( mWeekPrintType == Filofax ) {
    return QPrinter::Portrait;
  } else if ( mWeekPrintType == SplitWeek ) {
    return QPrinter::Portrait;
  } else {
    return QPrinter::Landscape;
  }
}

void CalPrintWeek::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintWeekConfig *cfg =
      dynamic_cast<CalPrintWeekConfig *>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mFromDate->setDate( from );
    cfg->mToDate->setDate( to );
  }
}

void CalPrintWeek::print( QPainter &p, int width, int height )
{
  QDate curWeek, fromWeek, toWeek;

  // correct begin and end to first and last day of week
  int weekdayCol = weekdayColumn( mFromDate.dayOfWeek() );
  fromWeek = mFromDate.addDays( -weekdayCol );
  weekdayCol = weekdayColumn( mToDate.dayOfWeek() );
  toWeek = mToDate.addDays( 6 - weekdayCol );

  curWeek = fromWeek.addDays( 6 );
  KLocale *local = KGlobal::locale();

  QString line1, line2, title;
  QRect headerBox( 0, 0, width, headerHeight() );
  QRect footerBox( 0, height - footerHeight(), width, footerHeight() );
  height -= footerHeight();

  QRect weekBox( headerBox );
  weekBox.setTop( headerBox.bottom() + padding() );
  weekBox.setBottom( height );

  switch ( mWeekPrintType ) {
  case Filofax:
    do {
      line1 = local->formatDate( curWeek.addDays( -6 ) );
      line2 = local->formatDate( curWeek );
      if ( orientation() == QPrinter::Landscape ) {
        title = i18nc( "date from-to", "%1 - %2", line1, line2 );
      } else {
        title = i18nc( "date from-\nto", "%1 -\n%2", line1, line2 );
      }
      drawHeader( p, title, curWeek.addDays( -6 ), QDate(), headerBox );

      drawWeek( p, curWeek, mStartTime, mEndTime, weekBox, mSingleLineLimit,
                mShowNoteLines, mIncludeDescription,
                mExcludeConfidential, mExcludePrivate );

      if ( mPrintFooter ) {
        drawFooter( p, footerBox );
      }

      curWeek = curWeek.addDays( 7 );
      if ( curWeek <= toWeek ) {
        mPrinter->newPage();
      }
    } while ( curWeek <= toWeek );
    break;

  case Timetable:
  default:
    do {
      line1 = local->formatDate( curWeek.addDays( -6 ) );
      line2 = local->formatDate( curWeek );
      if ( orientation() == QPrinter::Landscape ) {
        title = i18nc( "date from - to (week number)", "%1 - %2 (Week %3)",
                       line1, line2, curWeek.weekNumber() );
      } else {
        title = i18nc( "date from -\nto (week number)", "%1 -\n%2 (Week %3)",
                       line1, line2, curWeek.weekNumber() );
      }
      drawHeader( p, title, curWeek, QDate(), headerBox );

      drawTimeTable( p, fromWeek, curWeek,
                     false, mStartTime, mEndTime, weekBox,
                     mIncludeDescription, mExcludeTime, mExcludeConfidential,
                     mExcludePrivate );

      if ( mPrintFooter ) {
        drawFooter( p, footerBox );
      }

      fromWeek = fromWeek.addDays( 7 );
      curWeek = fromWeek.addDays( 6 );
      if ( curWeek <= toWeek ) {
        mPrinter->newPage();
      }
    } while ( curWeek <= toWeek );
    break;

  case SplitWeek:
  {
    QRect weekBox1( weekBox );
    // On the left side there are four days (mo-th) plus the timeline,
    // on the right there are only three days (fr-su) plus the timeline. Don't
    // use the whole width, but rather give them the same width as on the left.
    weekBox1.setRight( int( ( width - TIMELINE_WIDTH ) * 3. / 4. + TIMELINE_WIDTH ) );
    do {
      QDate endLeft( fromWeek.addDays( 3 ) );
      int hh = headerHeight();

      drawSplitHeaderRight( p, fromWeek, curWeek, QDate(), width, hh );
      drawTimeTable( p, fromWeek, endLeft,
                     false, mStartTime, mEndTime, weekBox,
                     mIncludeDescription, mExcludeTime,
                     mExcludeConfidential, mExcludePrivate );
      if ( mPrintFooter ) {
        drawFooter( p, weekBox1 );
      }
      mPrinter->newPage();
      drawSplitHeaderRight( p, fromWeek, curWeek, QDate(), width, hh );
      drawTimeTable( p, endLeft.addDays( 1 ), curWeek,
                     false, mStartTime, mEndTime, weekBox1,
                     mIncludeDescription, mExcludeTime,
                     mExcludeConfidential, mExcludePrivate );

      if ( mPrintFooter ) {
        drawFooter( p, footerBox );
      }

      fromWeek = fromWeek.addDays( 7 );
      curWeek = fromWeek.addDays( 6 );
      if ( curWeek <= toWeek ) {
        mPrinter->newPage();
      }
    } while ( curWeek <= toWeek );
  }
  break;
  }
}

/**************************************************************
 *           Print Month
 **************************************************************/

CalPrintMonth::CalPrintMonth() : CalPrintPluginBase()
{
}

CalPrintMonth::~CalPrintMonth()
{
}

QWidget *CalPrintMonth::createConfigWidget( QWidget *w )
{
  return new CalPrintMonthConfig( w );
}

void CalPrintMonth::readSettingsWidget()
{
  CalPrintMonthConfig *cfg = dynamic_cast<CalPrintMonthConfig *>( ( QWidget* )mConfigWidget );

  if ( cfg ) {
    mFromDate = QDate( cfg->mFromYear->value(), cfg->mFromMonth->currentIndex()+1, 1 );
    mToDate = QDate( cfg->mToYear->value(), cfg->mToMonth->currentIndex()+1, 1 );

    mWeekNumbers =  cfg->mWeekNumbers->isChecked();
    mRecurDaily = cfg->mRecurDaily->isChecked();
    mRecurWeekly = cfg->mRecurWeekly->isChecked();
    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mShowNoteLines = cfg->mShowNoteLines->isChecked();
    mSingleLineLimit = cfg->mSingleLineLimit->isChecked();
    mUseColors = cfg->mColors->isChecked();
    mPrintFooter = cfg->mPrintFooter->isChecked();
    mIncludeDescription = cfg->mIncludeDescription->isChecked();
    mExcludeConfidential = cfg->mExcludeConfidential->isChecked();
    mExcludePrivate = cfg->mExcludePrivate->isChecked();
  }
}

void CalPrintMonth::setSettingsWidget()
{
  CalPrintMonthConfig *cfg = dynamic_cast<CalPrintMonthConfig *>( ( QWidget* )mConfigWidget );

  if ( cfg ) {
    setDateRange( mFromDate, mToDate );

    cfg->mWeekNumbers->setChecked( mWeekNumbers );
    cfg->mRecurDaily->setChecked( mRecurDaily );
    cfg->mRecurWeekly->setChecked( mRecurWeekly );
    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mShowNoteLines->setChecked( mShowNoteLines );
    cfg->mSingleLineLimit->setChecked( mSingleLineLimit );
    cfg->mColors->setChecked( mUseColors );
    cfg->mPrintFooter->setChecked( mPrintFooter );
    cfg->mIncludeDescription->setChecked( mIncludeDescription );
    cfg->mExcludeConfidential->setChecked( mExcludeConfidential );
    cfg->mExcludePrivate->setChecked( mExcludePrivate );
  }
}

void CalPrintMonth::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName() );
    mWeekNumbers = grp.readEntry( "Print week numbers", true );
    mRecurDaily = grp.readEntry( "Print daily incidences", true );
    mRecurWeekly = grp.readEntry( "Print weekly incidences", true );
    mIncludeTodos = grp.readEntry( "Include todos", false );
    mSingleLineLimit = grp.readEntry( "Single line limit", false );
    mShowNoteLines = grp.readEntry( "Note Lines", false );
    mIncludeDescription = grp.readEntry( "Include description", false );
  }
  setSettingsWidget();
}

void CalPrintMonth::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName() );
    grp.writeEntry( "Print week numbers", mWeekNumbers );
    grp.writeEntry( "Print daily incidences", mRecurDaily );
    grp.writeEntry( "Print weekly incidences", mRecurWeekly );
    grp.writeEntry( "Include todos", mIncludeTodos );
    grp.writeEntry( "Single line limit", mSingleLineLimit );
    grp.writeEntry( "Note Lines", mShowNoteLines );
    grp.writeEntry( "Include description", mIncludeDescription );
  }
}

void CalPrintMonth::setDateRange( const QDate &from, const QDate &to )
{
  CalPrintPluginBase::setDateRange( from, to );
  CalPrintMonthConfig *cfg =
      dynamic_cast<CalPrintMonthConfig *>( ( QWidget* )mConfigWidget );
  const KCalendarSystem *calSys = calendarSystem();
  if ( cfg && calSys ) {
    cfg->mFromMonth->clear();
    for ( int i=0; i<calSys->monthsInYear( mFromDate ); ++i ) {
      cfg->mFromMonth->addItem( calSys->monthName( i+1, mFromDate.year() ) );
    }
    cfg->mToMonth->clear();
    for ( int i=0; i<calSys->monthsInYear( mToDate ); ++i ) {
      cfg->mToMonth->addItem( calSys->monthName( i+1, mToDate.year() ) );
    }
  }
  if ( cfg ) {
    cfg->mFromMonth->setCurrentIndex( from.month()-1 );
    cfg->mFromYear->setValue( to.year() );
    cfg->mToMonth->setCurrentIndex( mToDate.month()-1 );
    cfg->mToYear->setValue( mToDate.year() );
  }
}

void CalPrintMonth::print( QPainter &p, int width, int height )
{
  QDate curMonth, fromMonth, toMonth;

  fromMonth = mFromDate.addDays( -( mFromDate.day() - 1 ) );
  toMonth = mToDate.addDays( mToDate.daysInMonth() - mToDate.day() );

  curMonth = fromMonth;
  const KCalendarSystem *calSys = calendarSystem();
  if ( !calSys ) {
    return;
  }

  QRect headerBox( 0, 0, width, headerHeight() );
  QRect footerBox( 0, height - footerHeight(), width, footerHeight() );
  height -= footerHeight();

  QRect monthBox( 0, 0, width, height );
  monthBox.setTop( headerBox.bottom() + padding() );

  do {
    QString title( i18nc( "monthname year", "%1 <numid>%2</numid>",
                          calSys->monthName( curMonth ),
                          curMonth.year() ) );
    QDate tmp( fromMonth );
    int weekdayCol = weekdayColumn( tmp.dayOfWeek() );
    tmp = tmp.addDays( -weekdayCol );

    drawHeader( p, title, curMonth.addMonths( -1 ), curMonth.addMonths( 1 ),
                headerBox );
    drawMonthTable( p,
                    curMonth, QTime(), QTime(),
                    mWeekNumbers,
                    mRecurDaily, mRecurWeekly,
                    mSingleLineLimit, mShowNoteLines, mIncludeDescription,
                    mExcludeConfidential, mExcludePrivate, monthBox );

    if ( mPrintFooter ) {
      drawFooter( p, footerBox );
    }

    curMonth = curMonth.addDays( curMonth.daysInMonth() );
    if ( curMonth <= toMonth ) {
      mPrinter->newPage();
    }
  } while ( curMonth <= toMonth );

}

/**************************************************************
 *           Print Todos
 **************************************************************/

CalPrintTodos::CalPrintTodos() : CalPrintPluginBase()
{
  mTodoSortField = TodoFieldUnset;
  mTodoSortDirection = TodoDirectionUnset;
}

CalPrintTodos::~CalPrintTodos()
{
}

QWidget *CalPrintTodos::createConfigWidget( QWidget *w )
{
  return new CalPrintTodoConfig( w );
}

void CalPrintTodos::readSettingsWidget()
{
  CalPrintTodoConfig *cfg = dynamic_cast<CalPrintTodoConfig *>( ( QWidget* )mConfigWidget );

  if ( cfg ) {
    mPageTitle = cfg->mTitle->text();

    if ( cfg->mPrintAll->isChecked() ) {
      mTodoPrintType = TodosAll;
    } else if ( cfg->mPrintUnfinished->isChecked() ) {
      mTodoPrintType = TodosUnfinished;
    } else if ( cfg->mPrintDueRange->isChecked() ) {
      mTodoPrintType = TodosDueRange;
    } else {
      mTodoPrintType = TodosAll;
    }

    mFromDate = cfg->mFromDate->date();
    mToDate = cfg->mToDate->date();

    mIncludeDescription = cfg->mDescription->isChecked();
    mIncludePriority = cfg->mPriority->isChecked();
    mIncludeDueDate = cfg->mDueDate->isChecked();
    mIncludePercentComplete = cfg->mPercentComplete->isChecked();
    mConnectSubTodos = cfg->mConnectSubTodos->isChecked();
    mStrikeOutCompleted = cfg->mStrikeOutCompleted->isChecked();
    mExcludeConfidential = cfg->mExcludeConfidential->isChecked();
    mExcludePrivate = cfg->mExcludePrivate->isChecked();

    mTodoSortField = (eTodoSortField)cfg->mSortField->currentIndex();
    mTodoSortDirection = (eTodoSortDirection)cfg->mSortDirection->currentIndex();

    mPrintFooter = cfg->mPrintFooter->isChecked();
  }
}

void CalPrintTodos::setSettingsWidget()
{
  CalPrintTodoConfig *cfg =
      dynamic_cast<CalPrintTodoConfig *>( ( QWidget* )mConfigWidget );
  if ( cfg ) {
    cfg->mTitle->setText( mPageTitle );

    cfg->mPrintAll->setChecked( mTodoPrintType == TodosAll );
    cfg->mPrintUnfinished->setChecked( mTodoPrintType == TodosUnfinished );
    cfg->mPrintDueRange->setChecked( mTodoPrintType == TodosDueRange );

    cfg->mFromDate->setDate( mFromDate );
    cfg->mToDate->setDate( mToDate );

    cfg->mDescription->setChecked( mIncludeDescription );
    cfg->mPriority->setChecked( mIncludePriority );
    cfg->mDueDate->setChecked( mIncludeDueDate );
    cfg->mPercentComplete->setChecked( mIncludePercentComplete );
    cfg->mConnectSubTodos->setChecked( mConnectSubTodos );
    cfg->mStrikeOutCompleted->setChecked( mStrikeOutCompleted );
    cfg->mExcludeConfidential->setChecked( mExcludeConfidential );
    cfg->mExcludePrivate->setChecked( mExcludePrivate );

    if ( mTodoSortField != TodoFieldUnset ) {
      // do not insert if already done so.
      cfg->mSortField->addItem( i18nc( "@option sort by title", "Title" ) );
      cfg->mSortField->addItem( i18nc( "@option sort by start date/time", "Start Date" ) );
      cfg->mSortField->addItem( i18nc( "@option sort by due date/time", "Due Date" ) );
      cfg->mSortField->addItem( i18nc( "@option sort by priority", "Priority" ) );
      cfg->mSortField->addItem( i18nc( "@option sort by percent completed", "Percent Complete" ) );
      cfg->mSortField->setCurrentIndex( mTodoSortField );
    }

    if ( mTodoSortDirection != TodoDirectionUnset ) {
      // do not insert if already done so.
      cfg->mSortDirection->addItem( i18nc( "@option sort in increasing order", "Ascending" ) );
      cfg->mSortDirection->addItem( i18nc( "@option sort in descreasing order", "Descending" ) );
      cfg->mSortDirection->setCurrentIndex( mTodoSortDirection );
    }

    cfg->mPrintFooter->setChecked( mPrintFooter );
  }
}

void CalPrintTodos::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName() );
    mPageTitle = grp.readEntry( "Page title", i18n( "To-do list" ) );
    mTodoPrintType = (eTodoPrintType)grp.readEntry( "Print type", (int)TodosAll );
    mIncludeDescription = grp.readEntry( "Include description", true );
    mIncludePriority = grp.readEntry( "Include priority", true );
    mIncludeDueDate = grp.readEntry( "Include due date", true );
    mIncludePercentComplete = grp.readEntry( "Include percentage completed", true );
    mConnectSubTodos = grp.readEntry( "Connect subtodos", true );
    mStrikeOutCompleted = grp.readEntry( "Strike out completed summaries", true );
    mTodoSortField =
      (eTodoSortField)grp.readEntry( "Sort field", (int)TodoFieldSummary );
    mTodoSortDirection =
      (eTodoSortDirection)grp.readEntry( "Sort direction", (int)TodoDirectionAscending );
  }
  setSettingsWidget();
}

void CalPrintTodos::saveConfig()
{
  readSettingsWidget();
  if ( mConfig ) {
    KConfigGroup grp( mConfig, groupName());
    grp.writeEntry( "Page title", mPageTitle );
    grp.writeEntry( "Print type", int( mTodoPrintType ) );
    grp.writeEntry( "Include description", mIncludeDescription );
    grp.writeEntry( "Include priority", mIncludePriority );
    grp.writeEntry( "Include due date", mIncludeDueDate );
    grp.writeEntry( "Include percentage completed", mIncludePercentComplete );
    grp.writeEntry( "Connect subtodos", mConnectSubTodos );
    grp.writeEntry( "Strike out completed summaries", mStrikeOutCompleted );
    grp.writeEntry( "Sort field", (int)mTodoSortField );
    grp.writeEntry( "Sort direction", (int)mTodoSortDirection );
  }
}

void CalPrintTodos::print( QPainter &p, int width, int height )
{
  // TODO: Find a good way to guarantee a nicely designed output
  int pospriority = 0;
  int possummary = 100;
  int posdue = width - 65;
  int poscomplete = posdue - 70; //Complete column is to right of the Due column
  int lineSpacing = 15;
  //int fontHeight = 10;

  QRect headerBox( 0, 0, width, headerHeight() );
  QRect footerBox( 0, height - footerHeight(), width, footerHeight() );
  height -= footerHeight();

  // Draw the First Page Header
  drawHeader( p, mPageTitle, mFromDate, QDate(), headerBox );

  // Draw the Column Headers
  int mCurrentLinePos = headerHeight() + 5;
  QString outStr;
  QFont oldFont( p.font() );

  p.setFont( QFont( "sans-serif", 9, QFont::Bold ) );
  lineSpacing = p.fontMetrics().lineSpacing();
  mCurrentLinePos += lineSpacing;
  if ( mIncludePriority ) {
    outStr += i18n( "Priority" );
    p.drawText( pospriority, mCurrentLinePos - 2, outStr );
  } else {
    pospriority = -1;
  }

  outStr.truncate( 0 );
  outStr += i18nc( "@label to-do summary", "Title" );
  p.drawText( possummary, mCurrentLinePos - 2, outStr );

  if ( mIncludePercentComplete ) {
    if ( !mIncludeDueDate ) { //move Complete column to the right
      poscomplete = posdue; //if not print the Due Date column
    }
    outStr.truncate( 0 );
    outStr += i18nc( "@label to-do percentage complete", "Complete" );
    p.drawText( poscomplete, mCurrentLinePos - 2, outStr );
  } else {
    poscomplete = -1;
  }

  if ( mIncludeDueDate ) {
    outStr.truncate( 0 );
    outStr += i18nc( "@label to-do due date", "Due" );
    p.drawText( posdue, mCurrentLinePos - 2, outStr );
  } else {
    posdue = -1;
  }

  p.setFont( QFont( "sans-serif", 10 ) );
  //fontHeight = p.fontMetrics().height();

  KCalCore::Todo::List todoList;
  KCalCore::Todo::List tempList;

  KCalCore::SortDirection sortDirection = KCalCore::SortDirectionAscending;
  switch( mTodoSortDirection ) {
  case TodoDirectionAscending:
    sortDirection = KCalCore::SortDirectionAscending;
    break;
  case TodoDirectionDescending:
    sortDirection = KCalCore::SortDirectionDescending;
    break;
  case TodoDirectionUnset:
    break;
  }

  KCalCore::TodoSortField sortField = KCalCore::TodoSortSummary;
  switch( mTodoSortField ) {
  case TodoFieldSummary:
    sortField = KCalCore::TodoSortSummary;
    break;
  case TodoFieldStartDate:
    sortField = KCalCore::TodoSortStartDate;
    break;
  case TodoFieldDueDate:
    sortField = KCalCore::TodoSortDueDate;
    break;
  case TodoFieldPriority:
    sortField = KCalCore::TodoSortPriority;
    break;
  case TodoFieldPercentComplete:
    sortField = KCalCore::TodoSortPercentComplete;
    break;
  case TodoFieldUnset:
    break;
  }

  // Create list of to-dos which will be printed
  todoList = mCalendar->todos( sortField, sortDirection );
  switch( mTodoPrintType ) {
  case TodosAll:
    break;
  case TodosUnfinished:
    foreach( const KCalCore::Todo::Ptr& todo, todoList ) {
      Q_ASSERT( todo );
      if ( !todo->isCompleted() ) {
        tempList.append( todo );
      }
    }
    todoList = tempList;
    break;
  case TodosDueRange:
    foreach( const KCalCore::Todo::Ptr& todo, todoList ) {
      Q_ASSERT( todo );
      if ( todo->hasDueDate() ) {
        if ( todo->dtDue().date() >= mFromDate && todo->dtDue().date() <= mToDate ) {
          tempList.append( todo );
        }
      } else {
        tempList.append( todo );
      }
    }
    todoList = tempList;
    break;
  }

  // Print to-dos
  int count = 0;
  foreach( const KCalCore::Todo::Ptr& todo, todoList ) {
    if ( ( mExcludeConfidential && todo->secrecy() == Incidence::SecrecyConfidential ) ||
         ( mExcludePrivate      && todo->secrecy() == Incidence::SecrecyPrivate ) ) {
      continue;
    }
    // Skip sub-to-dos. They will be printed recursively in drawTodo()
    if ( todo->relatedTo().isEmpty() ) { //review(AKONADI_PORT)
      count++;
      drawTodo( count, todo, p,
                sortField, sortDirection,
                mConnectSubTodos,
                mStrikeOutCompleted, mIncludeDescription,
                pospriority, possummary, posdue, poscomplete,
                0, 0, mCurrentLinePos, width, height, todoList, 0,
                mExcludeConfidential, mExcludePrivate );
    }
  }

  if ( mPrintFooter ) {
    drawFooter( p, footerBox );
  }
  p.setFont( oldFont );
}
