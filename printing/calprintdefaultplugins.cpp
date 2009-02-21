/*
  This file is part of KOrganizer.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2008 Ron Goodheart <rong.dev@gmail.com>

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

#ifndef KORG_NOPRINTER

#include "calprintdefaultplugins.h"
#include "koprefs.h"

#include <libkdepim/kdateedit.h>

#include <KCal/Calendar>
#include <KCal/Todo>
#include <KCal/IncidenceFormatter>

#include <KCalendarSystem>
#include <KDateTime>
#include <KConfigGroup>
#include <KLocale>

#include <QDateTime>
#include <QPainter>
#include <QPrinter>

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
    cfg->mShowDetails->setChecked(mShowOptions);
    cfg->mShowSubitemsNotes->setChecked(mShowSubitemsNotes);
    cfg->mShowAttendees->setChecked(mShowAttendees);
    cfg->mShowAttachments->setChecked(mShowAttachments);
    cfg->mShowNoteLines->setChecked(mShowNoteLines);
  }
}

void CalPrintIncidence::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, description() );
    mUseColors = grp.readEntry( "Use Colors", false );
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
    KConfigGroup grp( mConfig, description() );
    grp.writeEntry( "Show Options", mShowOptions );
    grp.writeEntry( "Show Subitems and Notes", mShowSubitemsNotes );
    grp.writeEntry( "Use Attendees", mShowAttendees );
    grp.writeEntry( "Use Attachments", mShowAttachments );
    grp.writeEntry( "Note Lines", mShowNoteLines );
  }
}

class TimePrintStringsVisitor : public IncidenceBase::Visitor
{
  public:
    TimePrintStringsVisitor() {}

    bool act( IncidenceBase *incidence )
    {
      return incidence->accept( *this );
    }
    QString mStartCaption, mStartString;
    QString mEndCaption, mEndString;
    QString mDurationCaption, mDurationString;

  protected:
    bool visit( Event *event ) {
      if ( event->dtStart().isValid() ) {
        mStartCaption =  i18n( "Start date: " );
        // Show date/time or only date, depending on whether it's an all-day event
        mStartString = event->allDay() ?
                       event->dtStartDateStr( false ) : event->dtStartStr( false );
      } else {
        mStartCaption = i18n( "No start date" );
        mStartString.clear();
      }

      if ( event->hasEndDate() ) {
        mEndCaption = i18n( "End date: " );
        mEndString = event->allDay() ?
                     event->dtEndDateStr( false ) : event->dtEndStr( false );
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
    bool visit( Todo *todo ) {
      if ( todo->hasStartDate() ) {
        mStartCaption =  i18n( "Start date: " );
        // Show date/time or only date, depending on whether it's an all-day event
        mStartString = todo->allDay() ?
                       todo->dtStartDateStr( false ) : todo->dtStartStr( false );
      } else {
        mStartCaption = i18n( "No start date" );
        mStartString.clear();
      }

      if ( todo->hasDueDate() ) {
        mEndCaption = i18n( "Due date: " );
        mEndString = todo->allDay() ?
                     todo->dtDueDateStr( false ) : todo->dtDueStr( false );
      } else {
        mEndCaption = i18n( "No due date" );
        mEndString.clear();
      }
      return true;
    }
    bool visit( Journal *journal ) {
      mStartCaption = i18n( "Start date: " );
      mStartString = journal->allDay() ?
                     journal->dtStartDateStr( false ) : journal->dtStartStr( false );
      mEndCaption.clear();
      mEndString.clear();
      return true;
    }
    bool visit( FreeBusy *fb ) {
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
  p.drawText( box, Qt::AlignLeft|Qt::AlignTop|Qt::SingleLine, caption );

  if ( !text.isEmpty() ) {
    textRect.setLeft( textRect.left() + textWd );
    p.setFont( textFont );
    p.drawText( textRect, Qt::AlignLeft|Qt::AlignTop|Qt::SingleLine, text );
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
    if ( stringVis.act(*it) ) {
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
      KCal::Recurrence *recurs = (*it)->recurrence();
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
      KCal::Alarm::List::ConstIterator it;
      for ( it = alarms.constBegin(); it != alarms.constEnd(); ++it ) {
        Alarm *alarm = *it;

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

    QRect organizerBox( timesBox.left() + padding(), h + padding(),
                        timesBox.right() - padding(), lineHeight );
    h = qMax( printCaptionAndText( p, organizerBox, i18n( "Organizer: " ),
                                   (*it)->organizer().fullName(), captionFont, textFont ), h );

    // Finally, draw the frame around the time information...
    timesBox.setBottom( qMax( timesBox.bottom(), h + padding() ) );
    drawBox( p, BOX_BORDER_WIDTH, timesBox );

    QRect locationBox( timesBox );
    locationBox.setTop( timesBox.bottom() + padding() );
    locationBox.setHeight( 0 );
    int locationBottom = drawBoxWithCaption( p, locationBox, i18n( "Location: " ),
                                             (*it)->location(), /*sameLine=*/true,
                                             /*expand=*/true, captionFont, textFont );
    locationBox.setBottom( locationBottom );

    // Now start constructing the boxes from the bottom:
    QRect categoriesBox( locationBox );
    categoriesBox.setBottom( box.bottom() );
    categoriesBox.setTop( categoriesBox.bottom() - lineHeight - 2 * padding() );

    QRect attendeesBox( box.left(), categoriesBox.top() - padding() - box.height() / 9,
                        box.width(), box.height() / 9 );
    QRect attachmentsBox( box.left(), attendeesBox.top() - padding() - box.height() / 9,
                          box.width() * 3 / 4 - padding(), box.height() / 9 );
    QRect optionsBox( attachmentsBox.right() + padding(), attachmentsBox.top(), 0, 0 );
    optionsBox.setRight( box.right() );
    optionsBox.setBottom( attachmentsBox.bottom() );
    QRect notesBox( optionsBox.left(), locationBox.bottom() + padding(),
                    optionsBox.width(), 0 );
    notesBox.setBottom( optionsBox.top() - padding() );
    QRect descriptionBox( notesBox );
    descriptionBox.setLeft( box.left() );
    descriptionBox.setRight( attachmentsBox.right() );

    // Adjust boxes depending on the show options...
    if ( !mShowSubitemsNotes ) {
      descriptionBox.setRight( box.right() );
    }
    if ( !mShowAttachments || !mShowAttendees ) {
        descriptionBox.setBottom( attachmentsBox.bottom() );
        optionsBox.setTop( attendeesBox.top() );
        optionsBox.setBottom( attendeesBox.bottom() );
        notesBox.setBottom( attachmentsBox.bottom() );
        if (mShowOptions) {
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
    if ( mShowAttachments ) {
      if ( !mShowOptions ) {
        attachmentsBox.setRight( box.right() );
        attachmentsBox.setRight( box.right() );
      }
      if ( !mShowAttendees ) {
        attachmentsBox.setTop( attendeesBox.top() );
        attachmentsBox.setBottom( attendeesBox.bottom() );
      }
    }

    QString description;
    if ( (*it)->descriptionIsRich() ) {
      description = toPlainText( (*it)->description() );
    } else {
      description = (*it)->description();
    }
    int newBottom = drawBoxWithCaption( p, descriptionBox, i18n( "Description:" ),
                                        description, /*sameLine=*/false,
                                        /*expand=*/false, captionFont, textFont );

    if ( mShowNoteLines ) {
      drawNoteLines( p, descriptionBox, newBottom );
    }

    if ( mShowSubitemsNotes ) {
      if ( (*it)->relations().isEmpty() || (*it)->type() != "Todo" ) {
        int notesPosition = drawBoxWithCaption( p, notesBox, i18n( "Notes:" ),
                                                QString(), /*sameLine=*/false,
                                                /*expand=*/false, captionFont, textFont );
        drawNoteLines( p, notesBox, notesPosition );
      } else {
        Incidence::List relations = (*it)->relations();
        QString subitemCaption;
        if ( relations.count() == 0 ) {
          subitemCaption = i18n( "No Subitems" );
          txt.clear();
        } else {
          subitemCaption = i18np( "1 Subitem:",
                                  "%1 Subitems:",
                                  relations.count() );
        }
        Incidence::List::ConstIterator rit;
        QString subitemString;
        QString statusString;
        QString datesString;
        int count = 0;
        for ( rit = relations.constBegin(); rit != relations.constEnd(); ++rit ) {
          ++count;
          if ( !(*rit) ) { // defensive, skip any zero pointers
            continue;
          }
          // format the status
          statusString = (*rit)->statusStr();
          if ( statusString.isEmpty() ) {
            if ( (*rit)->status() == Incidence::StatusNone ) {
              statusString = i18nc( "no status", "none" );
            } else {
              statusString = i18nc( "unknown status", "unknown" );
            }
          }
          // format the dates if provided
          datesString.clear();
          KDateTime::Spec spec = KOPrefs::instance()->timeSpec();
          if ( (*rit)->dtStart().isValid() ) {
            datesString += i18nc(
              "subitem start date", "Start Date: %1\n",
              KGlobal::locale()->formatDate( (*rit)->dtStart().toTimeSpec( spec ).date(),
                                             KLocale::ShortDate ) );
            if ( !(*rit)->allDay() ) {
              datesString += i18nc(
                "subitem start time", "Start Time: %1\n",
                KGlobal::locale()->formatTime( (*rit)->dtStart().toTimeSpec( spec ).time(),
                                               false, false ) );
            }
          }
          if ( (*rit)->dtEnd().isValid() ) {
            subitemString += i18nc(
              "subitem due date", "Due Date: %1\n",
              KGlobal::locale()->formatDate( (*rit)->dtEnd().toTimeSpec( spec ).date(),
                                             KLocale::ShortDate ) );
            if ( !(*rit)->allDay() ) {
              subitemString += i18nc(
                "subitem due time", "Due Time: %1\n",
                KGlobal::locale()->formatTime( (*rit)->dtEnd().toTimeSpec( spec ).time(),
                                               false, false ) );
            }
          }
          subitemString += i18nc( "subitem counter", "%1: ", count );
          subitemString += (*rit)->summary();
          subitemString += '\n';
          if ( !datesString.isEmpty() ) {
            subitemString += datesString;
            subitemString += '\n';
          }
          subitemString += i18nc( "subitem Status: statusString",
                                  "Status: %1\n",
                                   statusString );
          subitemString += IncidenceFormatter::recurrenceString((*rit)) + '\n';
          subitemString += i18nc( "subitem Priority: N",
                                  "Priority: <numid>%1</numid>\n",
                                  (*rit)->priority() );
          subitemString += i18nc( "subitem Secrecy: secrecyString",
                                  "Secrecy: %1\n",
                                  (*rit)->secrecyStr() );
          subitemString += '\n';
        }
        drawBoxWithCaption( p, notesBox, subitemCaption,
                  subitemString, /*sameLine=*/false,
                            /*expand=*/false, captionFont, textFont );
      }
    }

    if ( mShowAttachments ) {
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
                (*ait)->fullName(), (*ait)->roleStr(), (*ait)->statusStr() );
      }
      drawBoxWithCaption( p, attendeesBox, attendeeCaption, attendeeString,
                          /*sameLine=*/false, /*expand=*/false,
                          captionFont, textFont );
    }

    if ( mShowOptions ) {
      QString optionsString;
      if ( !(*it)->statusStr().isEmpty() ) {
        optionsString += i18n( "Status: %1", (*it)->statusStr() );
        optionsString += '\n';
      }
      if ( !(*it)->secrecyStr().isEmpty() ) {
        optionsString += i18n( "Secrecy: %1", (*it)->secrecyStr() );
        optionsString += '\n';
      }
      if ( (*it)->type() == "Event" ) {
        Event *e = static_cast<Event*>(*it);
        if ( e->transparency() == Event::Opaque ) {
          optionsString += i18n( "Show as: Busy" );
        } else {
          optionsString += i18n( "Show as: Free" );
        }
        optionsString += '\n';
      } else if ( (*it)->type() == "Todo" ) {
        Todo *t = static_cast<Todo*>(*it);
        if ( t->isOverdue() ) {
          optionsString += i18n( "This task is overdue!" );
          optionsString += '\n';
        }
      } else if ( (*it)->type() == "Journal" ) {
        //TODO: Anything Journal-specific?
      }
      drawBoxWithCaption( p, optionsBox, i18n( "Settings: " ),
             optionsString, /*sameLine=*/false, /*expand=*/false,
             captionFont, textFont );
    }

    drawBoxWithCaption( p, categoriesBox, i18n( "Categories: " ),
           (*it)->categories().join( i18nc( "Spacer for the joined list of categories", ", " ) ),
           /*sameLine=*/true, /*expand=*/false, captionFont, textFont );
    drawFooter( p, box );
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
      mDayPrintType = Timetable;
    }

    mStartTime = cfg->mFromTime->time();
    mEndTime = cfg->mToTime->time();
    mIncludeAllEvents = cfg->mIncludeAllEvents->isChecked();

    mIncludeDescription = cfg->mIncludeDescription->isChecked();
    mSingleLineLimit = cfg->mSingleLineLimit->isChecked();
    mIncludeTodos = cfg->mIncludeTodos->isChecked();
    mUseColors = cfg->mColors->isChecked();
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

    cfg->mFromTime->setTime( mStartTime );
    cfg->mToTime->setTime( mEndTime );
    cfg->mIncludeAllEvents->setChecked( mIncludeAllEvents );

    cfg->mIncludeDescription->setChecked( mIncludeDescription );
    cfg->mSingleLineLimit->setChecked( mSingleLineLimit );
    cfg->mIncludeTodos->setChecked( mIncludeTodos );
    cfg->mColors->setChecked( mUseColors );
    cfg->mShowNoteLines->setChecked( mShowNoteLines );
    cfg->mExcludeTime->setChecked( mExcludeTime );
    cfg->mExcludeConfidential->setChecked( mExcludeConfidential );
    cfg->mExcludePrivate->setChecked( mExcludePrivate );
  }
}

void CalPrintDay::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, description() );
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
    KConfigGroup grp( mConfig, description() );
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

  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();

  switch ( mDayPrintType ) {
  case Filofax:
    {
      QString line1, line2, title;
      QRect headerBox( 0, 0, width, headerHeight() );
      QRect daysBox( headerBox );
      KLocale *local = KGlobal::locale();
      daysBox.setTop( headerBox.bottom() + padding() );
      daysBox.setBottom( height );
      line1 = local->formatDate( mFromDate );
      line2 = local->formatDate( mToDate );
      if ( orientation() == QPrinter::Landscape ) {
        title = i18nc( "date from-to", "%1 - %2", line1, line2 );
      } else {
        title = i18nc( "date from-\nto", "%1 -\n%2", line1, line2 );
      }
      drawHeader( p, title, mFromDate, QDate(), headerBox );
      drawDays( p, mFromDate, mToDate, daysBox, mSingleLineLimit,
                mShowNoteLines, mIncludeDescription,
                mExcludeConfidential, mExcludePrivate );
      drawFooter( p, daysBox );
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

    KLocale *local = KGlobal::locale();
    QRect headerBox( 0, 0, width, headerHeight() );

    drawHeader( p, local->formatDate( curDay ), curDay, QDate(), headerBox );
    Event::List eventList = mCalendar->events( curDay, timeSpec,
                                               EventSortStartDate,
                                               SortDirectionAscending );

    p.setFont( QFont( "sans-serif", 12 ) );

    // TODO: Find a good way to determine the height of the all-day box
    QRect allDayBox( TIMELINE_WIDTH + padding(), headerBox.bottom() + padding(),
        0, height - headerBox.bottom() - padding() );
    allDayBox.setRight( width );

    QRect dayBox( allDayBox );
    drawAgendaDayBox( p, eventList, curDay, mIncludeAllEvents,
                        curStartTime, curEndTime, dayBox,
                        mIncludeDescription, mExcludeTime,
                        mExcludeConfidential, mExcludePrivate );

    QRect tlBox( dayBox );
    tlBox.setLeft( 0 );
    tlBox.setWidth( TIMELINE_WIDTH );
    drawTimeLine( p, curStartTime, curEndTime, tlBox );
    drawFooter( p, dayBox );
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
    cfg->mIncludeDescription->setChecked( mIncludeDescription );
    cfg->mExcludeTime->setChecked( mExcludeTime );
    cfg->mExcludeConfidential->setChecked( mExcludeConfidential );
    cfg->mExcludePrivate->setChecked( mExcludePrivate );
  }
}

void CalPrintWeek::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, description() );
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
    KConfigGroup grp( mConfig, description() );
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
      drawWeek( p, curWeek, weekBox, mSingleLineLimit,
                mShowNoteLines, mIncludeDescription,
                mExcludeConfidential, mExcludePrivate );
      drawFooter( p, weekBox );
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
      QRect weekBox( headerBox );
      weekBox.setTop( headerBox.bottom() + padding() );
      weekBox.setBottom( height );
      drawTimeTable( p, fromWeek, curWeek, mStartTime, mEndTime, weekBox,
                     mIncludeDescription, mExcludeTime, mExcludeConfidential,
                     mExcludePrivate );
      drawFooter( p, weekBox );
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
      drawTimeTable( p, fromWeek, endLeft, mStartTime, mEndTime, weekBox,
                     mIncludeDescription, mExcludeTime,
                     mExcludeConfidential, mExcludePrivate );
      drawFooter( p, weekBox1 );
      mPrinter->newPage();
      drawSplitHeaderRight( p, fromWeek, curWeek, QDate(), width, hh );
      drawTimeTable( p, endLeft.addDays( 1 ), curWeek, mStartTime, mEndTime,
                     weekBox1, mIncludeDescription, mExcludeTime,
                     mExcludeConfidential, mExcludePrivate );
      drawFooter( p, weekBox1 );
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
    cfg->mIncludeDescription->setChecked( mIncludeDescription );
    cfg->mExcludeConfidential->setChecked( mExcludeConfidential );
    cfg->mExcludePrivate->setChecked( mExcludePrivate );
  }
}

void CalPrintMonth::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, description() );
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
    KConfigGroup grp( mConfig, description() );
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
    drawMonthTable( p, curMonth, mWeekNumbers, mRecurDaily, mRecurWeekly,
                    mSingleLineLimit, mShowNoteLines, mIncludeDescription,
                    mExcludeConfidential, mExcludePrivate, monthBox );
    drawFooter( p, monthBox );
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
      cfg->mSortField->addItem( i18n( "Summary" ) );
      cfg->mSortField->addItem( i18n( "Start Date" ) );
      cfg->mSortField->addItem( i18n( "Due Date" ) );
      cfg->mSortField->addItem( i18n( "Priority" ) );
      cfg->mSortField->addItem( i18n( "Percent Complete" ) );
      cfg->mSortField->setCurrentIndex( mTodoSortField );
    }

    if ( mTodoSortDirection != TodoDirectionUnset ) {
      // do not insert if already done so.
      cfg->mSortDirection->addItem( i18nc( "@option sort in increasing order", "Ascending" ) );
      cfg->mSortDirection->addItem( i18nc( "@option sort in descreasing order", "Descending" ) );
      cfg->mSortDirection->setCurrentIndex( mTodoSortDirection );
    }
  }
}

void CalPrintTodos::loadConfig()
{
  if ( mConfig ) {
    KConfigGroup grp( mConfig, description() );
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
    KConfigGroup grp( mConfig, description() );
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
  int possummary = 60;
  int posdue = width - 65;
  int poscomplete = posdue - 70; //Complete column is to right of the Due column
  int lineSpacing = 15;
  int fontHeight = 10;

  // Draw the First Page Header
  drawHeader( p, mPageTitle, mFromDate, QDate(),
                       QRect( 0, 0, width, headerHeight() ) );

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
    possummary = 10;
    pospriority = -1;
  }

  outStr.truncate( 0 );
  outStr += i18n( "Summary" );
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
  fontHeight = p.fontMetrics().height();

  Todo::List todoList;
  Todo::List tempList;
  Todo::List::ConstIterator it;

  // Convert sort options to the corresponding enums
  TodoSortField sortField = TodoSortSummary;
  switch( mTodoSortField ) {
  case TodoFieldSummary:
    sortField = TodoSortSummary;
    break;
  case TodoFieldStartDate:
    sortField = TodoSortStartDate;
    break;
  case TodoFieldDueDate:
    sortField = TodoSortDueDate;
    break;
  case TodoFieldPriority:
    sortField = TodoSortPriority;
    break;
  case TodoFieldPercentComplete:
    sortField = TodoSortPercentComplete;
    break;
  case TodoFieldUnset:
    break;
  }

  SortDirection sortDirection = SortDirectionAscending;
  switch( mTodoSortDirection ) {
  case TodoDirectionAscending:
    sortDirection = SortDirectionAscending;
    break;
  case TodoDirectionDescending:
    sortDirection = SortDirectionDescending;
    break;
  case TodoDirectionUnset:
    break;
  }

  // Create list of to-dos which will be printed
  todoList = mCalendar->todos( sortField, sortDirection );
  switch( mTodoPrintType ) {
  case TodosAll:
    break;
  case TodosUnfinished:
    for ( it = todoList.constBegin(); it != todoList.constEnd(); ++it ) {
      if ( !(*it)->isCompleted() ) {
        tempList.append( *it );
      }
    }
    todoList = tempList;
    break;
  case TodosDueRange:
    for ( it = todoList.constBegin(); it != todoList.constEnd(); ++it ) {
      if ( (*it)->hasDueDate() ) {
        if ( (*it)->dtDue().date() >= mFromDate && (*it)->dtDue().date() <= mToDate ) {
          tempList.append( *it );
        }
      } else {
        tempList.append( *it );
      }
    }
    todoList = tempList;
    break;
  }

  // Print to-dos
  int count = 0;
  for ( it = todoList.constBegin(); it != todoList.constEnd(); ++it ) {
    Todo *currEvent = *it;
    if ( ( mExcludeConfidential && currEvent->secrecy() == Incidence::SecrecyConfidential ) ||
         ( mExcludePrivate      && currEvent->secrecy() == Incidence::SecrecyPrivate ) ) {
      continue;
    }
    // Skip sub-to-dos. They will be printed recursively in drawTodo()
    if ( !currEvent->relatedTo() ) {
      count++;
      drawTodo( count, currEvent, p,
                sortField, sortDirection,
                mConnectSubTodos,
                mStrikeOutCompleted, mIncludeDescription,
                pospriority, possummary, posdue, poscomplete,
                0, 0, mCurrentLinePos, width, height, todoList );
    }
  }
  p.setFont( oldFont );
}

#endif
