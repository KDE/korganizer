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

#include <kcal/alarm.h>

#include <KLineEdit>

#include <QLabel>
#include <QFocusEvent>
#include <QBoxLayout>

class QWidget;
class QBoxLayout;
class QLabel;
class QCheckBox;
class QStackedWidget;
class QSpinBox;
class QPushButton;
class KComboBox;
class KLineEdit;
class KTextEdit;
class KSqueezedTextLabel;

namespace KCal {
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
    bool mSkipFirst;
};

class KOEditorGeneral : public QObject
{
  Q_OBJECT
  public:
    explicit KOEditorGeneral (QObject* parent=0,const char* name=0);
    virtual ~KOEditorGeneral();

    void initHeader(QWidget *,QBoxLayout *);
    void initDescription(QWidget *,QBoxLayout *);
    void initSecrecy(QWidget *,QBoxLayout *);
    void initCategories(QWidget *,QBoxLayout *);
    void initAlarm(QWidget *,QBoxLayout *);

    /** Set widgets to default values */
    void setDefaults(bool allDay);
    /** Read event object and setup widgets accordingly */
    void readIncidence(Incidence *);
    /** Write event settings to event object */
    void writeIncidence(Incidence *);

    /** Check if the input is valid. */
    bool validateInput() { return true; }

    void enableAlarm( bool enable );

    void setSummary( const QString & );
    void setDescription( const QString &, bool isRich );

    QObject *typeAheadReceiver() const;

  public slots:
    void setCategories(const QStringList &categories);
    void selectCategories();

  protected slots:
    void editAlarms();
    void updateAlarmWidgets();
    void updateDefaultAlarmTime();
    void toggleDescriptionBold();
    void toggleDescriptionItalic();
    void toggleDescriptionUnderline();
    void toggleDescriptionStrikethrough();
    void toggleDescriptionLeftAlign();
    void toggleDescriptionCentreAlign();
    void toggleDescriptionRightAlign();

  signals:
    void openCategoryDialog();
    void focusReceivedSignal();

  protected:
    Alarm *alarmFromSimplePage() const;
    void setAlignment( Qt::Alignment alignment );

    KLineEdit               *mSummaryEdit;
    KLineEdit               *mLocationEdit;
    QLabel                  *mAlarmBell;
    QStackedWidget            *mAlarmStack;
    QLabel                  *mAlarmInfoLabel;
    QCheckBox               *mAlarmButton;
    QSpinBox                *mAlarmTimeEdit;
    KComboBox               *mAlarmIncrCombo;
    QPushButton             *mAlarmEditButton;
    KTextEdit               *mDescriptionEdit;
    QLabel                  *mOwnerLabel;
    KComboBox               *mSecrecyCombo;
    QPushButton             *mCategoriesButton;
    KSqueezedTextLabel      *mCategoriesLabel;
    QPushButton             *mDescriptionBoldButton;
    QPushButton             *mDescriptionItalicButton;
    QPushButton             *mDescriptionUnderlineButton;
    QPushButton             *mDescriptionStrikethroughButton;
    QPushButton             *mDescriptionLeftAlignButton;
    QPushButton             *mDescriptionCentreAlignButton;
    QPushButton             *mDescriptionRightAlignButton;
    bool                    mRichDescription;

    enum AlarmStackPages { SimpleAlarmPage, AdvancedAlarmLabel };

  private:
    QStringList mCategories;
    KCal::Alarm::List mAlarmList;
};

#endif
