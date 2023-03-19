/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005,2008,2011 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include "kcmutils_version.h"
#include <KCModule>
#include <KConfigSkeleton>
#include <KFile>
#include <KLineEdit>
#include <KPageDialog>

#include <QVariantList>

class KColorButton;
class KComboBox;
class KDateComboBox;
class KTimeComboBox;
class KUrlRequester;

class QCheckBox;
class QLabel;
class QSpinBox;
class QTimeEdit;
class QButtonGroup;
class QGroupBox;

namespace Korganizer
{
/**
  @short Base class for GUI control elements used by @ref KPrefsDialog.
  @author Cornelius Schumacher
  @see KPrefsDialog

  This class provides the interface for the GUI control elements used by
  KPrefsDialog. The control element consists of a set of widgets for handling
  a certain type of configuration information.
*/
class KPrefsWid : public QObject
{
    Q_OBJECT
public:
    /**
      This function is called to read value of the setting from the
      stored configuration and display it in the widget.
    */
    virtual void readConfig() = 0;
    /**
      This function is called to write the current setting of the widget to the
      stored configuration.
    */
    virtual void writeConfig() = 0;

    /**
      Return a list of widgets used by this control element.
    */
    virtual QList<QWidget *> widgets() const;

Q_SIGNALS:
    /**
      Emitted when widget value has changed.
    */
    void changed();
};

/**
  @short Widgets for bool settings in @ref KPrefsDialog.

  This class provides a control element for configuring bool values. It is meant
  to be used by KPrefsDialog. The user is responsible for the layout management.
*/
class KPrefsWidBool : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a bool value control element consisting of a QCheckbox.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    explicit KPrefsWidBool(KConfigSkeleton::ItemBool *item, QWidget *parent = nullptr);

    /**
      Return the QCheckbox used by this control element.
    */
    QCheckBox *checkBox();

    void readConfig() override;
    void writeConfig() override;

    QList<QWidget *> widgets() const override;

private:
    KConfigSkeleton::ItemBool *mItem = nullptr;

    QCheckBox *mCheck = nullptr;
};

/**
  @short Widgets for int settings in @ref KPrefsDialog.

  This class provides a control element for configuring integer values. It is
  meant to be used by KPrefsDialog. The user is responsible for the layout
  management.
*/
class KPrefsWidInt : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a integer value control element consisting of a label and a
      spinbox.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    explicit KPrefsWidInt(KConfigSkeleton::ItemInt *item, QWidget *parent = nullptr);

    /**
      Return QLabel used by this control element.
    */
    QLabel *label() const;

    /**
      Return the QSpinBox used by this control element.
    */
    QSpinBox *spinBox();

    void readConfig() override;
    void writeConfig() override;

    QList<QWidget *> widgets() const override;

private:
    KConfigSkeleton::ItemInt *mItem = nullptr;

    QLabel *mLabel = nullptr;
    QSpinBox *mSpin = nullptr;
};

/**
  @short Widgets for time settings in @ref KPrefsDialog.

  This class provides a control element for configuring time values. It is
  meant to be used by KPrefsDialog. The user is responsible for the layout
  management.
*/
class KPrefsWidTime : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a time value control element consisting of a label and a spinbox.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    explicit KPrefsWidTime(KConfigSkeleton::ItemDateTime *item, QWidget *parent = nullptr);

    /**
      Return QLabel used by this widget.
    */
    QLabel *label();

    /**
      Return KTimeComboBox used by this widget.
    */
    KTimeComboBox *timeEdit();

    void readConfig() override;
    void writeConfig() override;

private:
    KConfigSkeleton::ItemDateTime *mItem = nullptr;

    QLabel *mLabel = nullptr;
    KTimeComboBox *mTimeEdit = nullptr;
};

/**
  @short Widgets for duration settings in @ref KPrefsDialog.

  This class provides a control element for configuring duration values. It is
  meant to be used by KPrefsDialog. The user is responsible for the layout
  management.
*/
class KPrefsWidDuration : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a duration value control element consisting of a label and a
      spinbox.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param format  display format. default is "hh:mm:ss"
      @param parent  Parent widget.
    */
    explicit KPrefsWidDuration(KConfigSkeleton::ItemDateTime *item, const QString &format, QWidget *parent = nullptr);

    /**
      Return QLabel used by this widget.
    */
    QLabel *label();
    /**
      Return QSpinBox used by this widget.
    */
    QTimeEdit *timeEdit();

    void readConfig() override;
    void writeConfig() override;

private:
    KConfigSkeleton::ItemDateTime *mItem = nullptr;

    QLabel *mLabel = nullptr;
    QTimeEdit *mTimeEdit = nullptr;
};

/**
  @short Widgets for time settings in @ref KPrefsDialog.

  This class provides a control element for configuring date values. It is
  meant to be used by KPrefsDialog. The user is responsible for the layout
  management.
*/
class KPrefsWidDate : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a time value control element consisting of a label and a date box.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    explicit KPrefsWidDate(KConfigSkeleton::ItemDateTime *item, QWidget *parent = nullptr);

    /**
      Return QLabel used by this widget.
    */
    QLabel *label();

    /**
      Return KDateComboBox used by this widget.
    */
    KDateComboBox *dateEdit();

    void readConfig() override;
    void writeConfig() override;

private:
    KConfigSkeleton::ItemDateTime *mItem = nullptr;

    QLabel *mLabel = nullptr;
    KDateComboBox *mDateEdit = nullptr;
};

/**
  @short Widgets for color settings in @ref KPrefsDialog.

  This class provides a control element for configuring color values. It is
  meant to be used by KPrefsDialog. The user is responsible for the layout
  management.
*/
class KPrefsWidColor : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a color value control element consisting of a test field and a
      button for opening a color dialog.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    explicit KPrefsWidColor(KConfigSkeleton::ItemColor *item, QWidget *parent = nullptr);

    /**
      Destruct color setting widget.
    */
    ~KPrefsWidColor() override;

    /**
      Return QLabel for the button
    */
    QLabel *label();
    /**
      Return button opening the color dialog.
    */
    KColorButton *button();

    void readConfig() override;
    void writeConfig() override;

private:
    KConfigSkeleton::ItemColor *mItem = nullptr;

    QLabel *mLabel = nullptr;
    KColorButton *mButton = nullptr;
};

/**
  @short Widgets for font settings in @ref KPrefsDialog.

  This class provides a control element for configuring font values. It is meant
  to be used by KPrefsDialog. The user is responsible for the layout management.
*/
class KPrefsWidFont : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a font value control element consisting of a test field and a
      button for opening a font dialog.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
      @param sampleText Sample text for previewing the selected font.
    */
    explicit KPrefsWidFont(KConfigSkeleton::ItemFont *item, QWidget *parent = nullptr, const QString &sampleText = QString());
    /**
      Destruct font setting widget.
    */
    ~KPrefsWidFont() override;

    /**
      Return QLabel.
    */
    QLabel *label();

    /**
      Return QFrame used as preview field.
    */
    QFrame *preview();

    /**
      Return button opening the font dialog.
    */
    QPushButton *button();

    void readConfig() override;
    void writeConfig() override;

protected Q_SLOTS:
    void selectFont();

private:
    KConfigSkeleton::ItemFont *mItem = nullptr;

    QLabel *mLabel = nullptr;
    QLabel *mPreview = nullptr;
    QPushButton *mButton = nullptr;
};

/**
  @short Widgets for settings represented by a group of radio buttons in
  @ref KPrefsDialog.

  This class provides a control element for configuring selections. It is meant
  to be used by KPrefsDialog. The user is responsible for the layout management.

  The setting is interpreted as an int value, corresponding to the position of
  the radio button. The position of the button is defined by the sequence of
  @ref addRadio() calls, starting with 0.
*/
class KPrefsWidRadios : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a control element for selection of an option. It consists of a box
      with several radio buttons.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    explicit KPrefsWidRadios(KConfigSkeleton::ItemEnum *item, QWidget *parent = nullptr);
    ~KPrefsWidRadios() override;

    /**
      Add a radio button.

      @param value The enum value represented by this radio button.
      @param text Text of the button.
      @param toolTip ToolTip help for the button.
      @param whatsThis What's This help for the button.
    */
    void addRadio(int value, const QString &text, const QString &toolTip = QString(), const QString &whatsThis = QString());

    /**
      Return the box widget used by this widget.
    */
    QGroupBox *groupBox() const;

    void readConfig() override;
    void writeConfig() override;

    QList<QWidget *> widgets() const override;

private:
    KConfigSkeleton::ItemEnum *mItem = nullptr;

    QGroupBox *mBox = nullptr;
    QButtonGroup *mGroup = nullptr;
};

/**
  @short Widgets for settings represented by a combo box in
  @ref KPrefsDialog.

  This class provides a control element for configuring selections. It is meant
  to be used by KPrefsDialog. The user is responsible for the layout management.

  The setting is interpreted as an int value, corresponding to the index in
  the combo box.
*/
class KPrefsWidCombo : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a control element for selection of an option. It consists of a
      combo box.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    explicit KPrefsWidCombo(KConfigSkeleton::ItemEnum *item, QWidget *parent);
    ~KPrefsWidCombo() override;

    void readConfig() override;
    void writeConfig() override;

    KComboBox *comboBox();

    QList<QWidget *> widgets() const override;
    /**
      Return QLabel used by this control element.
    */
    QLabel *label() const;

private:
    KConfigSkeleton::ItemEnum *mItem = nullptr;
    KComboBox *mCombo = nullptr;
    QLabel *mLabel = nullptr;
};

/**
  @short Widgets for string settings in @ref KPrefsDialog.

  This class provides a control element for configuring string values. It is
  meant to be used by KPrefsDialog. The user is responsible for the layout
  management.
*/
class KPrefsWidString : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a string value control element consisting of a test label and a
      line edit.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
      @param echomode  Describes how a line edit should display its contents.
    */
    explicit KPrefsWidString(KConfigSkeleton::ItemString *item, QWidget *parent = nullptr, KLineEdit::EchoMode echomode = KLineEdit::Normal);
    /**
      Destructor.
    */
    ~KPrefsWidString() override;

    /**
      Return QLabel used by this widget.
    */
    QLabel *label();
    /**
      Return KLineEdit used by this widget.
    */
    KLineEdit *lineEdit();

    void readConfig() override;
    void writeConfig() override;

    QList<QWidget *> widgets() const override;

private:
    KConfigSkeleton::ItemString *mItem = nullptr;

    QLabel *mLabel = nullptr;
    KLineEdit *mEdit = nullptr;
};

/**
  @short Widgets for string settings in @ref KPrefsDialog.

  This class provides a control element for configuring string values. It is
  meant to be used by KPrefsDialog. The user is responsible for the layout
  management.
*/
class KPrefsWidPath : public KPrefsWid
{
    Q_OBJECT
public:
    /**
      Create a string value control element consisting of a test label and a
      line edit.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
      @param filter URLRequester filter
      @param mode  Describes how a line edit should display its contents.
    */
    explicit KPrefsWidPath(KConfigSkeleton::ItemPath *item, QWidget *parent = nullptr, const QString &filter = QString(), KFile::Modes = KFile::File);

    /**
      Destructor.
    */
    ~KPrefsWidPath() override;

    /**
      Return QLabel used by this widget.
    */
    QLabel *label();

    /**
      Return KUrlRequester used by this widget.
    */
    KUrlRequester *urlRequester();

    void readConfig() override;
    void writeConfig() override;

    QList<QWidget *> widgets() const override;

private:
    KConfigSkeleton::ItemPath *mItem = nullptr;

    QLabel *mLabel = nullptr;
    KUrlRequester *mURLRequester = nullptr;
};

/**
  @short Class for managing KPrefsWid objects.

  This class manages standard configuration widgets provided bz the KPrefsWid
  subclasses. It handles creation, loading, saving and default values in a
  transparent way. The user has to add the widgets by the corresponding addWid
  functions and KPrefsWidManager handles the rest automatically.
*/
class KPrefsWidManager
{
public:
    /**
      Create a KPrefsWidManager object for a KPrefs object.

      @param prefs  KPrefs object used to access te configuration.
    */
    explicit KPrefsWidManager(KConfigSkeleton *prefs);

    /**
      Destructor.
    */
    virtual ~KPrefsWidManager();

    KConfigSkeleton *prefs() const
    {
        return mPrefs;
    }

    /**
      Register a custom KPrefsWid object.
    */
    virtual void addWid(KPrefsWid *);

    /**
      Register a @ref KPrefsWidBool object.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    KPrefsWidBool *addWidBool(KConfigSkeleton::ItemBool *item, QWidget *parent = nullptr);

    /**
      Register a @ref KPrefsWidInt object.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    KPrefsWidInt *addWidInt(KConfigSkeleton::ItemInt *item, QWidget *parent = nullptr);

    /**
      Register a @ref KPrefsWidDate object.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    KPrefsWidDate *addWidDate(KConfigSkeleton::ItemDateTime *item, QWidget *parent = nullptr);

    /**
      Register a @ref KPrefsWidTime object.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    KPrefsWidTime *addWidTime(KConfigSkeleton::ItemDateTime *item, QWidget *parent = nullptr);

    /**
      Register a @ref KPrefsWidDuration object.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param format  display format. default is "hh:mm:ss"
      @param parent  Parent widget.
    */
    KPrefsWidDuration *addWidDuration(KConfigSkeleton::ItemDateTime *item, const QString &format, QWidget *parent = nullptr);

    /**
      Register a @ref KPrefsWidColor object.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    KPrefsWidColor *addWidColor(KConfigSkeleton::ItemColor *item, QWidget *parent = nullptr);

    /**
      Register a @ref KPrefsWidRadios object. The choices represented by the
      given item object are automatically added as radio buttons.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    KPrefsWidRadios *addWidRadios(KConfigSkeleton::ItemEnum *item, QWidget *parent = nullptr);

    /**
      Register a @ref KPrefsWidCombo object. The choices represented by the
      given item object are automatically added to the combo box.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    KPrefsWidCombo *addWidCombo(KConfigSkeleton::ItemEnum *item, QWidget *parent = nullptr);

    /**
      Register a @ref KPrefsWidString object.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    KPrefsWidString *addWidString(KConfigSkeleton::ItemString *item, QWidget *parent = nullptr);

    /**
      Register a path @ref KPrefsWidPath object.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
      @param filter URLRequester filter
      @param mode URLRequester mode
    */
    KPrefsWidPath *addWidPath(KConfigSkeleton::ItemPath *item, QWidget *parent = nullptr, const QString &filter = QString(), KFile::Modes mode = KFile::File);

    /**
      Register a password @ref KPrefsWidString object, with echomode set to KLineEdit::Password.

      @param item    The KConfigSkeletonItem representing the preferences entry.
      @param parent  Parent widget.
    */
    KPrefsWidString *addWidPassword(KConfigSkeleton::ItemString *item, QWidget *parent = nullptr);

    /**
      Register a @ref KPrefsWidFont object.

      @param item       The KConfigSkeletonItem representing the preferences
                        entry.
      @param parent     Parent widget.
      @param sampleText Sample text for previewing the selected font.
    */
    KPrefsWidFont *addWidFont(KConfigSkeleton::ItemFont *item, QWidget *parent = nullptr, const QString &sampleText = QString());

    /** Set all widgets to default values. */
    void setWidDefaults();

    /** Read preferences from config file. */
    void readWidConfig();

    /** Write preferences to config file. */
    void writeWidConfig();

private:
    KConfigSkeleton *mPrefs = nullptr;

    QList<KPrefsWid *> mPrefsWids;
};

/**
  @short Base class for a preferences dialog.

  This class provides the framework for a preferences dialog. You have to
  subclass it and add the code to create the actual configuration widgets and
  do the layout management.

  KPrefsDialog provides functions to add subclasses of @ref KPrefsWid via
  KPrefsWidManager. For these widgets the reading, writing and setting to
  default values is handled automatically. Custom widgets have to be handled in
  the functions @ref usrReadConfig() and @ref usrWriteConfig().
*/
class KPrefsDialog : public KPageDialog, public KPrefsWidManager
{
    Q_OBJECT
public:
    /**
      Create a KPrefsDialog for a KPrefs object.

      @param prefs  KPrefs object used to access te configuration.
      @param parent Parent widget.
      @param name   Widget name.
      @param modal  true, if dialog has to be modal, false for non-modal.
    */
    explicit KPrefsDialog(KConfigSkeleton *prefs, QWidget *parent = nullptr, bool modal = false);

    /**
      Destructor.
    */
    ~KPrefsDialog() override;

    void autoCreate();

public Q_SLOTS:
    /** Set all widgets to default values. */
    void setDefaults();

    /** Read preferences from config file. */
    void readConfig();

    /** Write preferences to config file. */
    void writeConfig();

Q_SIGNALS:
    /** Emitted when the a changed configuration has been stored. */
    void configChanged();

protected Q_SLOTS:
    /** Apply changes to preferences */
    void slotApply();

    /** Accept changes to preferences and close dialog */
    void slotOk();

    /** Set preferences to default values */
    void slotDefault();

protected:
    /** Implement this to read custom configuration widgets. */
    virtual void usrReadConfig()
    {
    }

    /** Implement this to write custom configuration widgets. */
    virtual void usrWriteConfig()
    {
    }
};

class KPrefsModule : public KCModule, public KPrefsWidManager
{
    Q_OBJECT
public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KPrefsModule(KConfigSkeleton *, QWidget *parent = nullptr, const QVariantList &args = QVariantList());
#else
    explicit KPrefsModule(KConfigSkeleton *, QObject *parent, const KPluginMetaData &data, const QVariantList &args = QVariantList());
#endif

    void addWid(KPrefsWid *) override;

    void load() override;
    void save() override;
    void defaults() override;

public Q_SLOTS:
    void slotWidChanged();

protected:
    /** Implement this to read custom configuration widgets. */
    virtual void usrReadConfig()
    {
    }

    /** Implement this to write custom configuration widgets. */
    virtual void usrWriteConfig()
    {
    }
};
}
