#ifndef __DATE_VALIDATOR_H
#define __DATE_VALIDATOR_H

#include <qvalidator.h>
#include <qstring.h>
#include <qdatetime.h>


/*!
  \class DateValidator
  \brief QValidator for date input

  It ranges from 1/1/1753 to 12/31/8000, which seems to be QDate's range.<br>
  The DateValidator class currently understands three different date formats:<br>
  31.12.1999, the format used for example in germany,<br>
  12/31/1999 and 12-31-1999, used in other countries.

  It does <strong>not</strong> deal with other formats like "2000/1/1" !!!

  It is able to complete partial input, i.e. reads "10/31" as "10/31/2005" if told
  to have a default year of 2005, or recognize "10-31-5" to be "10-31-2005" if an
  appropriate time window has been set.

  The class provides a new data type, called \c DateFormat, which can have the
  following values:<br>
  <ul>
  <li>\c DateValidator::Unknown</li>
  <li>\c DateValidator::German for dd.mm.yyyy</li>
  <li>\c DateValidator::English_Slash for mm/dd/yyyy</li>
  <li>\c DateValidator::English_Minus for mm-dd-yyyy</li>
  </ul>
*/
class DateValidator : public QValidator
{
  Q_OBJECT

public:

  /*!
    constructor.<br>
    \a default_m, \a default_y and \a time_window are initial values for defaultMonth(),
    defaultYear() and timeWindow().
  */
  DateValidator(QWidget *parent,
		int default_m=-1, int default_y=-1,
		int time_window=1970,
		const char *name=0);


  /*!
    The DateValidator's destructor
  */
  ~DateValidator(void);


  /*!
    validate() returns one of \c QValidator::Invalid, \c QValidator::Valid and
    \c QValidator::Acceptable.<br>
    <ul>
    <li>If it is possible to generate a date from \a str, the result is Acceptable.<br>
        This does <strong>not</strong> mean that the date does really exist. (i.e., if you
        enter 02/31/2000, this will be \c Acceptable.)<br>
	Eventually given values for default month and year and time window are considered.</li>
    <li>If the input is absolutely no date, but some changes to it may lead to a date, the
        result is \c Valid. (i.e. an empty input will cause this)</li>
    <li>If the input contain nonsense, the result will be \c Invalid.</li>
    </ul>
  */
  //  enum QValidator::State validate(QString &str, int &cursor) const;
  enum State validate(QString &str, int &cursor) const;


  /*!
    This function is similar to validate().<br>
    It tries to understand the the given string and makes a QDate out of it, if possible.

    If the return value is \c Invalid or \c Valid, then the date has not been altered.
    If this function returns \c Acceptable, then the date has been written to \a dat.

    In case the input string \a str is correct syntax, but the date does not exist, the
    result will
    be \c Valid and \a dat not being altered, although validate() returns
    \c Acceptable in
    such a case. This is the only possibility for validate() and translateToDate() to have
    different results.<br>
    The reason is that a user does not need to know whether 2/29/3042 is existant or not.
    An application using this should display a "that date doesn't exist, dumbo!" message box
    only if the user clicks on "OK" or "Accept", and not while he's typing.
  */
  State translateToDate(QString &str, QDate &dat);


  /*! DateFormat
  */
  enum DateFormat
  {
    Unknown, German, English_Slash, English_Minus
  };


  /*!
    This function returns the default month.

    If the result equals -1, then no default month has been specified.

    The default month will be inserted if the user enters an incomplete date without a
    month. This means, that if "01/ /2000" is entered and default month is april, then
    the input will be taken as "01/04/2000".

    \sa setDefaultMonth()
  */
  int defaultMonth(void);


  /*!
    This function returns the default year.

    If the result equals -1, then no default year has been specified.

    The default year will be inserted if the user enters an incomplete date without a
    year. This means, that if "01/04" is entered and default year is 2000, then
    the input will be taken as "01/04/2000".

    \sa setDefaultYear()
  */
  int defaultYear(void);


  /*!
    This function returns the time window.

    If the result equals -1, then no time window has been specified.

    The time window is the first year of a 100-year-period where two-digit years
    are placed in.<br>
    For example, if the time window is 1970, then years from 00 to 69 will result
    to dates in the 21st century, while 70 to 99 will become 1970 to 1999.

    If no time window is specified (-1), then two-digit years will be called Invalid.

    \sa setTimeWindow()
  */
  int timeWindow(void);

public slots:
  /*!
    Sets the default month.

    Any number outside the range from 1 to 12 is interpreted as "No default month, please."

    \sa defaultMonth()
  */
  void setDefaultMonth(int);


  /*!
    Sets the default year.

    Any number outside the range from 1753 to 8000 is interpreted as "No default year, please."<br>
    This does also apply to two-digit values!

    \sa defaultYear()
  */
  void setDefaultYear(int);


  /*!
    Sets the time window.

    Any number outside the range from 1753 to 7900 is interpreted as "No time window, please."<br>
    This does also apply to two-digit values!
  */
  void setTimeWindow(int);


  /*!
    Expands \a str and/or translates it to another format.<br>
    It alters \a str and returns a reference to it.

    If \a form is \c Unknown, the type of date will be kept, else it will be translated.

    Examples:

    \code
    reFormat("1/31/88") == "01/31/1988"
    reFormat("1/31/88",German) == "31.01.1988"
    reFormat("31.1.88",English_Minus) == "01-31-1988"
    reFormat("funny, funny",English_Slash) == "funny, funny"
    \endcode

    Valid Parameters for \a form are \c Unknown (the default), \c English_Slash,
    \c English_Minus and \c German.
  */
  QString &reFormat(QString &str, DateFormat form=Unknown);


  /*!
    Similar to reFormat(QString &, DateFormat).

    It writes the given date \a dat into the string \a str using the specified
    format \a f, and returns a
    reference to the string \a str.

    If \a f is \c Unknown or \a dat is QDate::inValid(), then the \a str will not
    be altered.
  */
  QString &reFormat(QString &str, const QDate &dat, DateFormat f);


  /*!
    Returns the date format of the given string.

    If the result is \c Unknown, then the string is \c Invalid, but if the
    result is anything else, it means that its possible to find out what format is used,
    <strong>not</strong> that the input is \c Acceptable!.
  */
  DateFormat format(const QString &);


protected:

  struct DateDescription
  {
    DateFormat format;
    int day, month, year;
  };


  /*!
    \a str_length stores the length of a string while its being parsed, so that
    QString::length() is called only once.

    Initiated in parsedate() and used in skipWhiteSpace(), findNumber() and findSeparator().
    \internal
  */
  int str_length;


  /*!
    Increases \a pos to the next index of none-whitespace in \a str or beyond the
    length of \a str.

    depends on \c str_length.

    \sa parsedate()

    \internal
  */
  void skipWhiteSpace(const QString &str, int &pos);


  /*!
    Starts looking in \str at position \pos for whitespace, followed by a positive
    integer. \pos is set to index of the first char beyond the integer, and \a startpos
    to the first integer character's index.

    Return value is the found number, or
    \arg -1, if \a str contains whitespace only after \a pos
    \arg -2, at \a pos (after calling this function) is neither whitespace nor a number.

    depends on \c str_length

    \sa parsedate()

    \internal
  */
  int findNumber(const QString &str, int &pos, int &startpos);


  /*!
    Starts looking in \str at position \pos for whitespace, followed by
    any non-whitespace character.
    \pos is set to index of the first char beyond the one found, and \a seppos
    to the found character's index.

    Return value is the found character, or 0 if there was only whitespace.

    depends on \c str_length

    \sa parsedate()

    \internal
  */
  char findSeparator(const QString &str, int &pos, int &seppos);


  /*!
    Tries to fill up \a ddescr according to defaultMonth(), defaultYear() and
    timeWindow().

    It does not check whether the resulting date  exists.

    \internal
  */
  void insertMissing(DateDescription &ddescr);


  /*!
    This is DateValidator's central function. It parses \a str,
    sets up \a ddescr and modifies \a cursorpos, if necessary.

    The member functions translateToDate(), validate(), reFormat() and
    format() are using parsedate().

    Sets up \a str_length.

    \internal
  */
  State parsedate(const QString &str, int &cursorpos, DateDescription &ddescr);


  /*!
    Decides whether \a ddescr is \c Invalid, \c Valid or \c Acceptable.

    The result is \c Invalid, if
    \arg \c ddescr.day is greater than 31,
    \arg \c ddescr.month is greater than 12 or
    \arg \c ddescr.year is greater than 8000.
    
    The result is \c Valid, if
    \arg \c ddescr.format is \c Unknown,
    \arg \c ddescr.day is lower than 1,
    \arg \c ddescr.month is lower than 1 or
    \arg \c ddescr.year is lower than 1574.
    
    Else the result is \c Acceptable

    \internal
  */
  State makeState(DateDescription &ddescr);


  /*!
    This function prints the date given by \a d, \a m and \a y into
    \a str according to \a f, or leaves \a str unchanged if
    \a f is \c Unknown.

    \internal
  */
  void dprintf(QString &str, DateFormat f, int d, int m, int y);


  /*!
    Stores the default month.
    \internal
  */
  int default_month;


  /*!
    Stores the default year.
    \internal
  */
  int default_year;


  /*!
    Stores the time window.
    \internal
  */
  int time_win;
};


#endif
