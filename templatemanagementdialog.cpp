/******************************************************************************
**
** Filename   : templatemanagementdialog.cpp
** Created on : 05 June, 2005
** Copyright  : (c) 2005 Till Adam <adam@kde.org>
** Copyright (C) 2009  Allen Winter <winter@kde.org>
**
**
******************************************************************************/

/******************************************************************************
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
**   You should have received a copy of the GNU General Public License along
**   with this program; if not, write to the Free Software Foundation, Inc.,
**   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
******************************************************************************/

#include "templatemanagementdialog.h"

#include <KInputDialog>
#include <KLocale>
#include <KMessageBox>

#include <QStringList>
#include <QTimer>

TemplateManagementDialog::TemplateManagementDialog(
  QWidget *parent, const QStringList &templates, const QString &type )
  : KDialog( parent ), m_templates( templates ), m_type( type ), m_changed( false )
{
  QString m_type_translated = i18n( qPrintable( m_type ) );
  setCaption( i18n( "Manage %1 Templates", m_type_translated ) );
  setButtons( Ok | Cancel | Help );
  setObjectName( "template_management_dialog" );
  setHelp( "entering-data-events-template-buttons", "korganizer" );
  QWidget *widget = new QWidget( this );
  widget->setObjectName( "template_management_dialog_base" );
  m_base.setupUi( widget );
  setMainWidget( widget );

  m_base.m_listBox->addItems( m_templates );
  m_base.m_listBox->setSelectionMode( QAbstractItemView::SingleSelection );

  connect( m_base.m_buttonAdd, SIGNAL(clicked()),
           SLOT(slotAddTemplate()) );
  connect( m_base.m_buttonRemove, SIGNAL(clicked()),
           SLOT(slotRemoveTemplate()) );
  connect( m_base.m_buttonApply, SIGNAL(clicked()),
           SLOT(slotApplyTemplate()) );

  connect( m_base.m_listBox, SIGNAL(itemSelectionChanged()),
           SLOT(slotItemSelected()) );
  connect( m_base.m_listBox, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
           SLOT(slotApplyTemplate()) );
  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );

  m_base.m_buttonRemove->setEnabled( false );
  m_base.m_buttonApply->setEnabled( false );
}

void TemplateManagementDialog::slotItemSelected()
{
  m_base.m_buttonRemove->setEnabled( true );
  m_base.m_buttonApply->setEnabled( true );
}

void TemplateManagementDialog::slotAddTemplate()
{
  bool ok;
  bool duplicate = false;
  QString m_type_translated = i18n( qPrintable( m_type ) );
  const QString newTemplate = KInputDialog::getText(
    i18n( "Template Name" ),
    i18n( "Please enter a name for the new template:" ),
    i18n( "New %1 Template", m_type_translated ), &ok );
  if ( newTemplate.isEmpty() || !ok ) {
    return;
  }

  if ( m_templates.contains( newTemplate ) ) {
    int rc = KMessageBox::warningContinueCancel(
      this,
      i18n( "A template with that name already exists, do you want to overwrite it?" ),
      i18n( "Duplicate Template Name" ), KGuiItem( i18n( "Overwrite" ) ) );
    if ( rc == KMessageBox::Cancel ) {
      QTimer::singleShot( 0, this, SLOT(slotAddTemplate()) );
      return;
    }
    duplicate = true;
  }

  if ( !duplicate ) {
    int count = m_base.m_listBox->count();
    m_templates.append( newTemplate );
    m_base.m_listBox->addItem( newTemplate );
    QListWidgetItem *item = m_base.m_listBox->item( count );
    m_base.m_listBox->setItemSelected( item, true );
  }
  m_newTemplate = newTemplate;
  m_changed = true;

  // From this point on we need to keep the original event around until the
  // user has closed the dialog, applying a template would make little sense
  enableButtonApply( false );
  // neither does adding it again
  m_base.m_buttonAdd->setEnabled( false );
}

void TemplateManagementDialog::slotRemoveTemplate()
{
  QListWidgetItem *const item = m_base.m_listBox->selectedItems().first();
  if ( !item ) {
    return; // can't happen (TM)
  }

  int rc = KMessageBox::warningContinueCancel(
    this,
    i18n( "Are you sure that you want to remove the template <b>%1</b>?", item->text() ),
    i18n( "Remove Template" ), KGuiItem( i18n( "Remove" ) ) );

  if ( rc == KMessageBox::Cancel ) {
    return;
  }

  int current = m_base.m_listBox->row( item );

  m_templates.removeAll( item->text() );
  m_base.m_listBox->takeItem( current );

  m_base.m_listBox->setItemSelected(
    m_base.m_listBox->item( qMax( current - 1, 0 ) ), true );

  updateButtons();

  m_changed = true;
}

void TemplateManagementDialog::updateButtons()
{
  m_base.m_buttonAdd->setEnabled( true );
  m_base.m_buttonRemove->setEnabled( m_base.m_listBox->count() != 0 );
  m_base.m_buttonApply->setEnabled( m_base.m_listBox->count() != 0 );
}

void TemplateManagementDialog::slotApplyTemplate()
{
  // Once the user has applied the current template to the event,
  // it makes no sense to add it again
  m_base.m_buttonAdd->setEnabled( false );
  const QString &cur = m_base.m_listBox->currentItem()->text();
  if ( !cur.isEmpty() && cur != m_newTemplate ) {
    emit loadTemplate( cur );
  }
}

void TemplateManagementDialog::slotOk()
{
  // failure is not an option *cough*
  if ( !m_newTemplate.isEmpty() ) {
    emit saveTemplate( m_newTemplate );
  }
  if ( m_changed ) {
    emit templatesChanged( m_templates );
  }
}

#include "templatemanagementdialog.moc"
