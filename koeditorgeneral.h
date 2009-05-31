/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOEDITORGENERAL_H
#define KOEDITORGENERAL_H

#include <KCal/Alarm>
#include <KLineEdit>

class KOEditorAttachments;

class KComboBox;
class KToolBar;
class KRichTextWidget;
class KSqueezedTextLabel;
class KUrl;

class QBoxLayout;
class QCheckBox;
class QFocusEvent;
class QLabel;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QToolButton;
class QWidget;

namespace KCal {
  class Calendar;
  class Incidence;
}
using namespace KCal;

class FocusLineEdit : public KLineEdit
{
  Q_OBJECT
  public:
    explicit FocusLineEdit( QWidget *parent );

  signals:
    void focusReceivedSignal();

  protected:
    void focusInEvent ( QFocusEvent *e );

  private:
    bool mFirst;
};

class KOEditorGeneral : public QObject
{
  Q_OBJECT
  public:
    explicit KOEditorGeneral( Calendar *calendar, QObject *parent=0 );
    virtual ~KOEditorGeneral();

    void initHeader( QWidget *parent, QBoxLayout *topLayout );
    void initDescription( QWidget *, QBoxLayout * );
    void initSecrecy( QWidget *, QBoxLayout * );
    void initAlarm( QWidget *, QBoxLayout * );
    void initAttachments( QWidget *, QBoxLayout * );

    /** Set widgets to default values */
    void setDefaults( bool allDay );

    /** Read event object and setup widgets accordingly */
    void readIncidence( Incidence *incidence );

    /** Write event settings to event object */
    void fillIncidence( Incidence * );

    /** Check if the input is valid. */
    bool validateInput();

    void enableAlarm( bool enable );
    void toggleAlarm( bool on );

    void setSummary( const QString & );
    void setDescription( const QString &, bool isRich );

    QObject *typeAheadReceiver() const;

  public slots:
    void setCategories( const QStringList &categories );
    void selectCategories();
    void addAttachments( const QStringList &attachments,
                         const QStringList &mimeTypes = QStringList(),
                         bool inlineAttachment = false );

  protected slots:
    void editAlarms();
    void updateAlarmWidgets();
    void updateDefaultAlarmTime();
    void updateAttendeeSummary( int count );
    void setDescriptionRich( bool rich );

  signals:
    void openCategoryDialog();
    void updateCategoryConfig();
    void focusReceivedSignal();
    void openURL( const KUrl & );

  protected:
    Alarm *alarmFromSimplePage() const;

    virtual bool setAlarmOffset( Alarm *alarm, int value ) const = 0;

    QWidget                 *mParent;
    KLineEdit               *mSummaryEdit;
    KLineEdit               *mLocationEdit;
    QLabel                  *mAttendeeSummaryLabel;
    QLabel                  *mAlarmBell;
    QStackedWidget          *mAlarmStack;
    QLabel                  *mAlarmInfoLabel;
    QCheckBox               *mAlarmButton;
    QSpinBox                *mAlarmTimeEdit;
    KComboBox               *mAlarmIncrCombo;
    QPushButton             *mAlarmEditButton;
    KRichTextWidget         *mDescriptionEdit;
    QLabel                  *mOwnerLabel;
    KComboBox               *mSecrecyCombo;
    QPushButton             *mCategoriesButton;
    KSqueezedTextLabel      *mCategoriesLabel;
    KToolBar                *mEditToolBar;
    KToolBar                *mFormatToolBar;
    QCheckBox               *mRichDescription;
    KOEditorAttachments     *mAttachments;
    Calendar                *mCalendar;
    QStringList              mCategories;

    enum AlarmStackPages {
      SimpleAlarmPage,
      AdvancedAlarmLabel
    };

  private:
    void toggleDescriptionRichButtons( bool rich );
    KCal::Alarm::List mAlarmList;
};

#endif
