#ifndef __DATEEDIT_H
#define __DATEEDIT_H

#include <qlineedit.h>
#include "datevalidator.h"


/*!
  \class DateEdit
  \brief Line Edit for dates

  The DateEdit class looks and feels like a QLineEdit.

  It automagically creates
  a DateValidator and provides reading and writing with QDates.
*/

class DateEdit : public QLineEdit
{
  Q_OBJECT
public:

  /*!
    constructor.

    does also create a DateValidator and uses it.

    if \a name is not \c NULL, the DateValidator will get a similar name.

    \a default_m, \a default_y and \a time_window are passed to DateValidator
  */
  DateEdit(QWidget *parent=0, int default_m=-1, int default_y=-1,
	   int time_window=1970, const char *name=0);


  /*!
    Returns a pointer to a QDate object.

    That object could (and will) be deleted by DateEdit at any time, so do not keep
    reference of it.

    If the result is NULL, then the DateEdit currently does not
    contain a valid date.
  */
  const QDate *date(void);


  /*!
    Returns the format currently in use.

    This may also be \c DateValidator::Unknown.
  */
  DateValidator::DateFormat currentFormat(void);


  /*!
    Returns a pointer to the DateEdit's DateValidator.

    You can use it to set some flags, i.e.
    \code
    myDateEdit -> dateValidator() -> setDefaultYear(2001);
    \endcode

    \warning Never delete the returned object. You would get
    core dumps.
  */
  DateValidator *dateValidator(void);


public slots:


  /*!
    Tries to complete the input (default month and year, time window) and re-formats
    it.
  */
  void lookNice();


  /*!
    Fills the DateEdit with a new value.

    \a f is a hint what format to use. It must not be \c DateValidator::Unknown!

    If \a prior is true, \a dat will be formatted as specified by \a f.

    If \a prior is false, \a dat will be formatted like the current input, and as
    specified by \a f, if the current input has no format.
  */
  void setDate(const QDate &dat, DateValidator::DateFormat f, bool prior=TRUE);


  /*!
    This will re-format the current input to look like the given Format.

    If the current input is incomplete, it will be completed first.

    If it can't be completed or is invalid, nothing will happen.

    All this does not prevent the user from using another format when he
    continues typing.
  */
  void setFormat(DateValidator::DateFormat);


signals:


  /*!
    This signal is emitted whenever the input date changes.

    The parameter may also be \c NULL, if the input changes from any valid date
    to garbage.

    If the user changes the text field in a way that does not changes the date
    (like deleting leading zeroes), only one dateChanged() signal will be emitted
    in total, although there will be several textChanged() signals.
  */
  void dateChanged(const QDate*);

protected slots:


  /*!
    A private slot that will be connected to textChanged() by the constructor.

    It decides whether a textChanged() signal should be followed by
    a dateChanged() signal and emits that signal if necessary

    \internal
  */
  void catchTextChanged(const QString &);


protected:


  /*!
    Same function as in QLineEdit.<br>
    Made protected so that nobody can do ugly things.
  */
  void setValidator(QValidator *);


  /*!
    Stores a pointer to a QDate containing the date currently in the line buffer.

    May be NULL;

    Is changed by catchTextChanged().
    \internal
  */
  QDate *lastValue;


  /*!
    Stores a pointer to the DateValidator.
    Initiated by the constructor.
    \internal
  */
  DateValidator *dvalidator;
};

#endif
