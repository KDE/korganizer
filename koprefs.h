/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOPREFS_H
#define KOPREFS_H

#include <qdict.h>

#include <kdepimmacros.h>

#include "koprefs_base.h"

class KConfig;
class QFont;
class QColor;
class QStringList;

QColor getTextColor(const QColor &c);


class KDE_EXPORT KOPrefs : public KOPrefsBase
{
  public:
    virtual ~KOPrefs();

    /** Get instance of KOPrefs. It is made sure that there is only one
    instance. */
    static KOPrefs *instance();

    /** Set preferences to default values */
    void usrSetDefaults();

    /** Read preferences from config file */
    void usrReadConfig();

    /** Write preferences to config file */
    void usrWriteConfig();

  protected:
    void setCategoryDefaults();
    void setTimeZoneIdDefault();

    /** Fill empty mail fields with default values. */
    void fillMailDefaults();

  private:
    /** Constructor disabled for public. Use instance() to create a KOPrefs
    object. */
    KOPrefs();

    static KOPrefs *mInstance;

  public:
    // preferences data
    void setFullName( const QString & );
    QString fullName();
    void setEmail( const QString & );
    QString email();
    /// Returns all email addresses for the user.
    QStringList allEmails();
    /// Returns all email addresses together with the full username for the user.
    QStringList fullEmails();
    /// Return true if the given email belongs to the user
    bool thatIsMe( const QString& email );

    void setCategoryColor( const QString &cat, const QColor &color );
    QColor *categoryColor( const QString &cat );

    void setResourceColor ( const QString &, const QColor & );
    QColor* resourceColor( const QString & );
    
    QString mHtmlExportFile;

    // Groupware passwords
    QString mPublishPassword;
    QString mRetrievePassword;

  private:
    QDict<QColor> mCategoryColors;
    QColor mDefaultCategoryColor;

    QDict<QColor> mResourceColors;
    QColor mDefaultResourceColor;

    QFont mDefaultMonthViewFont;

  public: // Do not use - except in KOPrefsDialogMain
    QString mName;
    QString mEmail;
};

#endif
