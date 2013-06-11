/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KORG_EXPORTWEBDIALOG_H
#define KORG_EXPORTWEBDIALOG_H

#include <libkdepim/prefs/kprefsdialog.h>

namespace KOrg {
  class HTMLExportSettings;
}

/**
  ExportWebDialog is a class that provides the dialog and functions to export a
  calendar as web page.
*/
class ExportWebDialog : public KPageDialog, public KPIM::KPrefsWidManager
{
  Q_OBJECT
  public:
    explicit ExportWebDialog( KOrg::HTMLExportSettings *settings, QWidget *parent = 0 );
    virtual ~ExportWebDialog();

  public slots:
    void slotTextChanged( const QString & _text );

  protected:
    void setupGeneralPage();
    void setupEventPage();
    void setupTodoPage();
//    void setupJournalPage();
//    void setupFreeBusyPage();
//    void setupAdvancedPage();

  public slots:
    void setDefaults();
    void readConfig();
    void writeConfig();

  signals:
    void configChanged();
    void exportHTML( KOrg::HTMLExportSettings * );

  protected slots:
    void slotOk();
    void slotApply();
    void slotDefault();

  protected:
    virtual void usrReadConfig() {}
    virtual void usrWriteConfig() {}

  private slots:
    void updateState();

  private:
    KOrg::HTMLExportSettings *mSettings;
    QFrame *mGeneralPage;
    QFrame *mEventPage;
    QFrame *mTodoPage;
//    QFrame *mJournalPage;
//    QFrame *mFreeBusyPage;
//    QFrame *mAdvancedPage;
    QGroupBox *mDateRangeGroup;
    QCheckBox *mMonthViewCheckBox;
    QCheckBox *mEventListCheckBox;
};

#endif // _EXPORTWEBDIALOG_H
