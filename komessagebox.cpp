/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Large code parts were copied from kmessagebox.cpp:
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

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

#include <kmessagebox.h>
#include <qmessagebox.h>
#include <kdeversion.h>
#include <kdialogbase.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kactivelabel.h>
#include <klocale.h>

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qstylesheet.h>
#include <qsimplerichtext.h>
#include <qguardedptr.h>
#include <qvgroupbox.h>
#include <qtextedit.h>
#include <qpushbutton.h>

#include "komessagebox.h"

static QPixmap themedMessageBoxIcon(QMessageBox::Icon icon)
{
    QString icon_name;

    switch(icon)
    {
    case QMessageBox::NoIcon:
        return QPixmap();
        break;
    case QMessageBox::Information:
        icon_name = "messagebox_info";
        break;
    case QMessageBox::Warning:
        icon_name = "messagebox_warning";
        break;
    case QMessageBox::Critical:
        icon_name = "messagebox_critical";
        break;
    default:
        break;
    }

   QPixmap ret = KApplication::kApplication()->iconLoader()->loadIcon(icon_name, KIcon::NoGroup, KIcon::SizeMedium, KIcon::DefaultState, 0, true);

   if (ret.isNull())
       return QMessageBox::standardIcon(icon);
   else
       return ret;
}

static QString qrichtextify( const QString& text )
{
  if ( text.isEmpty() || text[0] == '<' )
    return text;

  QStringList lines = QStringList::split('\n', text);
  for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
  {
    *it = QStyleSheet::convertFromPlainText( *it, QStyleSheetItem::WhiteSpaceNormal );
  }

  return lines.join(QString::null);
}

static int createKOMessageBox( KDialogBase *dialog, QMessageBox::Icon icon,
                               const QString &text, int options)
{
    QVBox *topcontents = new QVBox (dialog);
    topcontents->setSpacing(KDialog::spacingHint()*2);
    topcontents->setMargin(KDialog::marginHint());

    QWidget *contents = new QWidget(topcontents);
    QHBoxLayout * lay = new QHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint());

    QLabel *label1 = new QLabel( contents );

    if (icon != QMessageBox::NoIcon)
        label1->setPixmap(themedMessageBoxIcon(icon));

    lay->addWidget( label1, 0, Qt::AlignCenter );
    lay->addSpacing(KDialog::spacingHint());
    // Enforce <p>text</p> otherwise the word-wrap doesn't work well
    QString qt_text = qrichtextify( text );

    int pref_width = 0;
    int pref_height = 0;
    // Calculate a proper size for the text.
    {
       QSimpleRichText rt(qt_text, dialog->font());
       QRect d = KGlobalSettings::desktopGeometry(dialog);

       pref_width = d.width() / 3;
       rt.setWidth(pref_width);
       int used_width = rt.widthUsed();
       pref_height = rt.height();
       if (used_width <= pref_width)
       {
          while(true)
          {
             int new_width = (used_width * 9) / 10;
             rt.setWidth(new_width);
             int new_height = rt.height();
             if (new_height > pref_height)
                break;
             used_width = rt.widthUsed();
             if (used_width > new_width)
                break;
          }
          pref_width = used_width;
       }
       else
       {
          if (used_width > (pref_width *2))
             pref_width = pref_width *2;
          else
             pref_width = used_width;
       }
    }
    KActiveLabel *label2 = new KActiveLabel( qt_text, contents );
    if ((options & KMessageBox::AllowLink) == 0)
    {
       QObject::disconnect(label2, SIGNAL(linkClicked(const QString &)),
                  label2, SLOT(openLink(const QString &)));
    }

    // We add 10 pixels extra to compensate for some KActiveLabel margins.
    // @TODO: find out why this is 10.
    label2->setFixedSize(QSize(pref_width+10, pref_height));
    lay->addWidget( label2 );
    lay->addStretch();

    dialog->setMainWidget(topcontents);
    dialog->enableButtonSeparator(false);

    const KDialogBase::ButtonCode buttons[] = {
        KDialogBase::Help,
        KDialogBase::Default,
        KDialogBase::Ok,
        KDialogBase::Apply,
        KDialogBase::Try,
        KDialogBase::Cancel,
        KDialogBase::Close,
        KDialogBase::User1,
        KDialogBase::User2,
        KDialogBase::User3,
        KDialogBase::No,
        KDialogBase::Yes,
        KDialogBase::Details };
    for( unsigned int i = 0;
	 i < sizeof( buttons )/sizeof( buttons[ 0 ] );
	 ++i )
	if( QPushButton* btn = dialog->actionButton( buttons[ i ] ))
	    if( btn->isDefault())
		btn->setFocus();

    
    // We use a QGuardedPtr because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the guarded ptr will reset to 0.
    QGuardedPtr<KDialogBase> guardedDialog = dialog;

    int result = guardedDialog->exec();
    delete (KDialogBase *) guardedDialog;
    return result;
}


int KOMessageBox::fourBtnMsgBox( QWidget *parent, QMessageBox::Icon type, 
            const QString &text, const QString &caption, 
            const KGuiItem &button1, const KGuiItem &button2, 
            const KGuiItem &button3, int options)
{
  KDialogBase *dialog= new KDialogBase( parent, "KOMessageBox", true,
                     caption.isEmpty() ? "" : caption,
                     KDialogBase::Yes | KDialogBase::No | KDialogBase::Ok | KDialogBase::Cancel,
                     KDialogBase::Yes, 
                     true/*, button1, button2, button3*/);
  dialog->setButtonOK( button3 );
  dialog->setButtonText( KDialogBase::Yes, button1.text() );
  dialog->setButtonText( KDialogBase::No, button2.text() );
  QObject::connect( dialog->actionButton( KDialogBase::Yes ), SIGNAL( clicked() ), dialog, SLOT(slotYes()));
  QObject::connect( dialog->actionButton( KDialogBase::No ), SIGNAL( clicked() ), dialog, SLOT(slotNo()));
//  QObject::connect( dialog, SIGNAL( noClicked() ), dialog, SLOT(slotNo()));
  

#if KDE_IS_VERSION(3,2,90)
  bool checkboxResult = false;
  int result = KMessageBox::createKMessageBox(dialog, type, text, QStringList(),
                     QString::null, &checkboxResult, options);
#else
  int result = createKOMessageBox( dialog, type, text, options );
#endif
  switch (result) {
    case KDialogBase::Yes: result = KMessageBox::Yes; break;
    case KDialogBase::No: result = KMessageBox::No; break;
    case KDialogBase::Ok: result = KMessageBox::Continue; break;
    case KDialogBase::Cancel: result = KMessageBox::Cancel; break;
    default: break;
  }

  return result;
}


