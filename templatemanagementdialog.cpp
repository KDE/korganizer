/*******************************************************************************
**
** Filename   : templatemanagementdialog.cpp
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
#include "templatemanagementdialog.h"

#include <qstringlist.h>
#include <qtimer.h>

#include <kpushbutton.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

TemplateManagementDialog::TemplateManagementDialog(QWidget *parent, const QStringList &templates )
    :KDialogBase( parent, "template_management_dialog", true,
                        i18n("Manage Templates"), Ok|Cancel, Ok, true , i18n("Apply Template")),
      m_templates( templates ), m_newTemplate( QString::null ), m_changed( false )
{
  m_base = new TemplateManagementDialog_base( this, "template_management_dialog_base" );
  setMainWidget( m_base );
  connect( m_base->m_buttonAdd, SIGNAL( clicked() ),
           SLOT( slotAddTemplate() ) );
  connect( m_base->m_buttonDelete, SIGNAL( clicked() ),
           SLOT( slotDeleteTemplate() ) );
  m_base->m_listBox->insertStringList( m_templates );
  connect( m_base->m_listBox, SIGNAL( selectionChanged( QListBoxItem * ) ),
           SLOT( slotUpdateDeleteButton( QListBoxItem * ) ) );
  connect( m_base->m_buttonApply, SIGNAL( clicked() ),
           SLOT( slotApplyTemplate() ) );

}

void TemplateManagementDialog::slotAddTemplate()
{
  bool ok;
  bool duplicate = false;
  const QString newTemplate = KInputDialog::getText( i18n("Template Name"),
                                       i18n("Please enter a name for the new template:"),
                                       i18n("New Template"), &ok );
  if ( newTemplate.isEmpty() || !ok ) return;
  if ( m_templates.find( newTemplate) != m_templates.end() ) {
    int rc = KMessageBox::warningContinueCancel( this, i18n("A template with that name already exists, do you want to overwrite it?."), i18n("Duplicate Template Name"), i18n("Overwrite"));
    if ( rc == KMessageBox::Cancel ) {
      QTimer::singleShot(0, this, SLOT( slotAddTemplate() ) );
      return;
    }
    duplicate = true;
  }
  if ( !duplicate ) {
    m_templates.append( newTemplate );
    m_base->m_listBox->clear();
    m_base->m_listBox->insertStringList( m_templates );
  }
  m_newTemplate = newTemplate;
  m_changed = true;
  // From this point on we need to keep the original event around until the user has
  // closed the dialog, applying a template would make little sense
  m_base->m_buttonApply->setEnabled( false );
  // neither does adding it again
  m_base->m_buttonAdd->setEnabled( false );
}

void TemplateManagementDialog::slotDeleteTemplate()
{
  QListBoxItem *const item = m_base->m_listBox->selectedItem();
  if ( !item ) return; // can't happen (TM)
  unsigned int current = m_base->m_listBox->index(item);
  m_templates.remove( item->text() );
  m_base->m_listBox->removeItem( m_base->m_listBox->currentItem() );
  m_changed = true;
  m_base->m_listBox->setSelected(QMAX(current -1, 0), true);
}

void TemplateManagementDialog::slotUpdateDeleteButton( QListBoxItem *item )
{
  m_base->m_buttonDelete->setEnabled( item != 0 );
}

void TemplateManagementDialog::slotApplyTemplate()
{
  // Once the user has applied the current template to the event, it makes no sense to add it again
  m_base->m_buttonAdd->setEnabled( false );
  const QString &cur = m_base->m_listBox->currentText();
  if ( !cur.isEmpty() && cur != m_newTemplate )
    emit loadTemplate( cur );
}

void TemplateManagementDialog::slotOk()
{
  // failure is not an option *cough*
  if ( !m_newTemplate.isEmpty() )
    emit saveTemplate( m_newTemplate );
  if ( m_changed )
    emit templatesChanged( m_templates );
  KDialogBase::slotOk();
}


#include "templatemanagementdialog.moc"
