// $Id$

#include <qcursor.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "event.h"

#include "koeventpopupmenu.h"
#include "koeventpopupmenu.moc"

KOEventPopupMenu::KOEventPopupMenu()
{
  mCurrentEvent = 0;
  mHasAdditionalItems = false;

  insertItem (i18n("&Show"),this,SLOT(popupShow()));
  mEditOnlyItems.append(insertItem (i18n("&Edit"),this,SLOT(popupEdit())));
  mEditOnlyItems.append(insertItem (SmallIcon("editdelete"),i18n("&Delete"),
                                   this,SLOT(popupDelete())));
}

void KOEventPopupMenu::showEventPopup(Event *event)
{
  mCurrentEvent = event;
  
  if (mCurrentEvent) {
    // Enable/Disabled menu items only valid for editable events.
    QValueList<int>::Iterator it;
    for( it = mEditOnlyItems.begin(); it != mEditOnlyItems.end(); ++it ) {
      setItemEnabled(*it,!mCurrentEvent->isReadOnly());
    }
    popup(QCursor::pos());
  } else {
    kdDebug() << "KOEventPopupMenu::showEventPopup(): No event selected" << endl;
  }
}

void KOEventPopupMenu::addAdditionalItem(const QIconSet &icon,const QString &text,
                                    const QObject *receiver, const char *member,
                                    bool editOnly)
{
  if (!mHasAdditionalItems) {
    mHasAdditionalItems = true;
    insertSeparator();
  }
  int id = insertItem(icon,text,receiver,member);
  if (editOnly) mEditOnlyItems.append(id);
}

void KOEventPopupMenu::popupShow()
{
  if (mCurrentEvent) emit showEventSignal(mCurrentEvent);
}

void KOEventPopupMenu::popupEdit()
{
  if (mCurrentEvent) emit editEventSignal(mCurrentEvent);
}

void KOEventPopupMenu::popupDelete()
{
  if (mCurrentEvent) emit deleteEventSignal(mCurrentEvent);
}
