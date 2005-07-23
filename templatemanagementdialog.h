/*******************************************************************************
**
** Filename   : templatemanagerdialog.h
** Created on : 05 June, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/
#ifndef TEMPLATEMANAGEMENTDIALOG_H
#define TEMPLATEMANAGEMENTDIALOG_H

#include <qstringlist.h>

#include "template_management_dialog_base.h"

#include <kdialogbase.h>

class TemplateManagementDialog: public KDialogBase {
Q_OBJECT
  public:
    TemplateManagementDialog( QWidget *parent, const QStringList& templates );

  signals:
    /* Emitted whenever the user hits apply, indicating that the currently selected template
    should be loaded into to the incidence editor which triggered this. */
    void loadTemplate( const QString& templateName );
    /* Emitted whenever the user wants to add teh current incidence as a template
    with the given name. */
    void saveTemplate( const QString& templateName );
    /* Emitted when the dialog changed the list of templates. Calling code can the replace
       the list that was handed in with the one this signal transports. */
    void templatesChanged( const QStringList& templates );
    
  protected slots:
    void slotAddTemplate();
    void slotDeleteTemplate();
    void slotApplyTemplate();
    void slotUpdateDeleteButton( QListBoxItem *item );
    void slotOk();
    
  private:
    TemplateManagementDialog_base *m_base;
    QStringList m_templates;
    QString m_newTemplate;
    bool m_changed;
};

#endif
