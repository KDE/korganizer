/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KNEWSTUFF_UPLOADDIALOG_H
#define KNEWSTUFF_UPLOADDIALOG_H

#include <kdialogbase.h>

class QLineEdit;
class QSpinBox;
class KURLRequester;
class QTextEdit;
class QComboBox;

namespace KNS {

class Engine;
class Entry;

class UploadDialog : public KDialogBase
{
    Q_OBJECT
  public:
    UploadDialog( Engine *, QWidget *parent );
    ~UploadDialog();

    void setPreviewFile( const QString &previewFile );

  protected slots:
    void slotOk();

  private:
    Engine *mEngine;

    QLineEdit *mNameEdit;
    QLineEdit *mAuthorEdit;
    QLineEdit *mVersionEdit;
    QSpinBox *mReleaseSpin;
    KURLRequester *mPreviewUrl;
    QTextEdit *mSummaryEdit;
    QComboBox *mLanguageCombo;
    QComboBox *mLicenceCombo;

    QPtrList<Entry> mEntryList;
};

}

#endif
