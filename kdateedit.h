/*
    This file is part of KOrganizer.

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KDATEEDIT_H
#define _KDATEEDIT_H
// $Id$	

#include <qhbox.h>
#include <qvbox.h>
#include <qdatetime.h>

class QLineEdit;
class QPushButton;
class QObject;
class QEvent;
class KDatePicker;
class KDateValidator;

/** 
* A date editing widget that consists of a line edit followed by
* a small push button. The line edit contains the date in text form, 
* and the push button will display a 'popup' style date picker.
*
* This widget also supports advanced features like allowing the user
* to type in the day name to get the date. The following keywords
* are supported (in the native language): tomorrow, yesturday, today,
* monday, tuesday, wednesday, thursday, friday, saturday, sunday.
*
* @author Cornelius Schumacher <schumacher@kde.org>
* @author Mike Pilone <mpilone@slac.com>
*/
class KDateEdit : public QHBox
{
    Q_OBJECT
  public:
    KDateEdit(QWidget *parent=0, const char *name=0);
    virtual ~KDateEdit();

    /** @return True if the date in the text edit is valid,
    * false otherwise. This will not modify the display of the date,
    * but only check for validity.
    */
    bool inputIsValid();
    
    /** @return The date entered. This will not
    * modify the display of the date, but only return it.
    */
    QDate date() const;
    
    /** Checks for a focus out event. The display of the date is updated
    * to display the proper date when the focus leaves.
    */
    virtual bool eventFilter(QObject *o, QEvent *e);
    
  signals:
    /** This signal is emitted whenever the user modifies the date. This
    * may not get emitted until the user presses enter in the line edit or
    * focus leaves the widget (ie: the user confirms their selection).
    */
    void dateChanged(QDate);

  public slots:
    /** Sets the date.
    *
    * @param date The new date to display. This date must be valid or
    * it will not be displayed.
    */
    void setDate(QDate date);
    
    /** Sets the date edit to be enabled or disabled (grayed out)
    * 
    * @param on Enabled if true, disabled if false
    */
    void setEnabled(bool on);

  protected slots:
    void toggleDatePicker();
    void lineEnterPressed();
    void textChanged(const QString &);
    
  private:
    /** Reads the text from the line edit. If the text is a keyword, the
    * word will be translated to a date. If the text is not a keyword, the
    * text will be interpreted as a date.
    */
    QDate readDate() const;
    
    /** Maps the text that the user can enter to the offset in days from
    * today. For example, the text 'tomorrow' is mapped to +1.
    */
    QMap<QString, int> mKeywordMap;
    bool mTextChanged;
    
    QPushButton *mDateButton;
    QLineEdit *mDateEdit;
    KDatePicker *mDatePicker;
    QVBox *mDateFrame;
};

#endif
