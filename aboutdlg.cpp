/*
 * $Id$
 *
 * "About" dialog for KOrganizer
 *
 * (c) 1998 Preston Brown <pbrown@kde.org>
 */

#include <string.h>
#include <stdlib.h>

#include <qmovie.h>
#include <qlayout.h>
#include <qframe.h>
#include <qfile.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>

#include "version.h"
#include "aboutdlg.h"
#include "aboutdlg.moc"

PostcardDialog::PostcardDialog(QWidget *parent, const char *name)
  : QDialog(parent, name, TRUE)
{
  setCaption(i18n("KOrganizer Virtual Postcard"));

  QVBoxLayout *layout = new QVBoxLayout(this, 10);
  QLabel *label = new QLabel(this);
  label->setText(i18n("Please send a postcard to me, so I can see who is\n"
		      "using KOrganizer! Tell me how great the program is,\n"
		      "and how you just can't live without it.  Or, report\n"
		      "a bug which is stopping you from doing useful work.\n"
		      "The postcard will simply include anything you wish to\n"
		      "type in the comment area below.\n"));
  label->setMinimumSize(label->sizeHint());
  layout->addWidget(label);

  commentArea = new QMultiLineEdit(this);
  commentArea->setMinimumHeight(60);
  commentArea->setMinimumWidth(300);
  layout->addWidget(commentArea);

  QFrame *hLine = new QFrame(this);
  hLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  hLine->setMinimumWidth(commentArea->width());
  hLine->setMinimumHeight(hLine->frameWidth());
  layout->addWidget(hLine);

  buttonBox = new KButtonBox(this);
  buttonBox->addStretch();
  QPushButton *okButton = buttonBox->addButton(i18n("&OK"));
  okButton->setDefault(TRUE);
  buttonBox->setMinimumWidth(buttonBox->sizeHint().width());
  buttonBox->setFixedHeight(buttonBox->sizeHint().height());
  connect(okButton, SIGNAL(clicked()), SLOT(send()));
  QPushButton *cancelButton = buttonBox->addButton(i18n("&Cancel"));
  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  buttonBox->layout();

  layout->addWidget(buttonBox);
  
  layout->activate();

  resize(sizeHint());
  commentArea->setFocus();
}

PostcardDialog::~PostcardDialog()
{
}

void PostcardDialog::send()
{
  KConfig *config = kapp->config();
  config->setGroup("Personal Settings");
  QString mailCmd, name, tmpFileName;
  QFile tmpFile;

  if (!strcmp(commentArea->text(), "")) // is comment empty?
    return;

  mailCmd = "mail -s \"KOrganizer Postcard from ";
  name = config->readEntry("user_name", "");
  if (name.isEmpty())
    name = config->readEntry("user_email", "Unknown User");
  mailCmd += name.data();
  mailCmd += "\" pbrown@kde.org < ";
  
  tmpFileName = tmpnam(0L);
  tmpFile.setName(tmpFileName.data());
  tmpFile.open(IO_WriteOnly);
  
  name = "Version: v";
  name += korgVersion;
  name += "\n";
  tmpFile.writeBlock(name.data(), name.length());
  tmpFile.writeBlock(commentArea->text(), strlen(commentArea->text()));
  tmpFile.close();
  mailCmd += tmpFileName.data();
  system(mailCmd.data());
  tmpFile.remove();
  accept();
}

AboutDialog::AboutDialog(QWidget *parent, const char *name)
  : QDialog(parent, name, TRUE)
{

  setCaption(i18n("About KOrganizer"));

  QVBoxLayout *layout = new QVBoxLayout(this, 10);

  QLabel *titleLabel = new QLabel(this);
  QFont tmpFont = titleLabel->font();
  tmpFont.setStyleHint(QFont::Helvetica);
  tmpFont.setPointSize(36);
  tmpFont.setBold(TRUE);
  titleLabel->setFont(tmpFont);
  QString tmpStr;
  tmpStr.sprintf("KOrganizer v%s",korgVersion);
  titleLabel->setText(tmpStr.data());
  titleLabel->setMinimumSize(titleLabel->sizeHint());
  titleLabel->setAlignment(AlignCenter);
  layout->addWidget(titleLabel);
  
  /*  movieLabel = new QLabel(this);
  movieLabel->setFrameStyle(QFrame::Box | QFrame::Plain);
  QString filePath(KApplication::kde_datadir());
  filePath += "/korganizer/pics/knlogo1.gif";
  QMovie movie(filePath.data());
  //  movie.connectStatus(this, SLOT(movieStatus(int)));
  movie.connectUpdate(this, SLOT(movieUpdate(const QRect&))); 
  movieLabel->setMovie(movie);
  movieLabel->setFixedSize(174+movieLabel->frameWidth()*2,
			     62+movieLabel->frameWidth()*2);
			     layout->addWidget(movieLabel);*/

  tmpFont.setPointSize(24);
  tmpFont.setBold(FALSE);
  titleLabel = new QLabel(this);
  titleLabel->setFont(tmpFont);
  titleLabel->setText(i18n("by Preston Brown and"));
  titleLabel->setMinimumSize(titleLabel->sizeHint());
  titleLabel->setAlignment(AlignCenter);
  layout->addWidget(titleLabel);

  contribLabel = new QLabel(this);
  contribLabel->setFont(QFont("helvetica", 18));
  contribLabel->setAlignment(AlignCenter);
  contribLabel->setMinimumWidth(400);
  contribLabel->setText("Foo");
  contribLabel->setFixedHeight(contribLabel->sizeHint().height());
  contribLabel->setText("");
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()),
	  SLOT(updateContributers()));
  timer->start(3000);
  layout->addWidget(contribLabel);

  QLabel *licenseLabel = new QLabel(this);
  licenseLabel->setText(i18n("License Agreement:"));
  licenseLabel->setFixedHeight(licenseLabel->sizeHint().height());
  layout->addWidget(licenseLabel);

  QString licenseText;
  licenseText = 
    i18n("\n\n"
	 "This program is free software; you can redistribute it and/or modify\n"
	 "it under the terms of the GNU General Public License as published by\n"
	 "the Free Software Foundation; either version 2 of the License, or\n"
	 "(at your option) any later version.\n"
	 "\n"
	 "This program is distributed in the hope that it will be useful,\n"
	 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	 "GNU General Public License for more details.\n"
	 "\n"
	 "You should have received a copy of the GNU General Public License\n"
	 "along with this program; if not, write to the Free Software\n"
	 "Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n");
  QMultiLineEdit *licenseTextArea = new QMultiLineEdit(this);
  licenseTextArea->setReadOnly(TRUE);
  licenseTextArea->setText(licenseText.data());
  licenseTextArea->setMinimumHeight(150);
  licenseTextArea->setMinimumWidth(460);
  layout->addWidget(licenseTextArea);

  layout->addStretch();  

  QFrame *hLine = new QFrame(this, "hLine");
  hLine->setFrameStyle(QFrame::HLine | QFrame::Sunken);
  hLine->setMinimumWidth(licenseTextArea->width());
  hLine->setMinimumHeight(hLine->frameWidth());
  layout->addWidget(hLine);

  buttonBox = new KButtonBox(this);
  buttonBox->addStretch();
  okButton = buttonBox->addButton(i18n("&OK"));
  okButton->setDefault(TRUE);
  buttonBox->setFixedHeight(buttonBox->sizeHint().height());
  connect(okButton, SIGNAL(clicked()), SLOT(accept()));
  mailButton = buttonBox->addButton(i18n("&Send Postcard"));
  connect(mailButton, SIGNAL(clicked()), SLOT(sendPostcard()));
  buttonBox->layout();

  layout->addWidget(buttonBox);

  // start the layout
  layout->activate();

  resize(sizeHint());

  QRect prect = parent->geometry();
  move(prect.x() + prect.width()/4,
       prect.y() + prect.height()/4);
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::updateContributers()
{
  const char *contributers[] = {
    "Laszlo Boloni",
    "Barry Benowitz",
    "Christopher Beard",
    "Ian Dawes",
    "Neil Hart",
    "Hans-Jürgen Husel",
    "Christian Kirsch",
    "Uwe Koloska",
    "Glen Parker",
    "Dan Pilone",
    "Roman Rohr",
    "Cornelius Schumacher",
    "Herwin Jan Steehouwer",
    "Fester Zigterman",
    i18n("many more, thank you!"),
    0L,
  };

  static int which=0;

  contribLabel->setText(contributers[which]);
  ++which;
  if (contributers[which] == 0L)
    which = 0;
}

void AboutDialog::movieUpdate( const QRect& )
{
    // Uncomment this to test animated icons on your window manager
    //setIcon( movie.framePixmap() );
}

void AboutDialog::movieStatus( int s )
{
  switch ( s ) {
  case QMovie::SourceEmpty:
    movieLabel->setText(i18n("Could not load movie"));
    movieLabel->setAlignment( AlignCenter );
    movieLabel->setBackgroundColor( backgroundColor() );
    break;
  default:
    if ( movieLabel->movie() )              // for flicker-free animation:
      movieLabel->setBackgroundMode( NoBackground );
  }
}

void AboutDialog::sendPostcard()
{
  PostcardDialog *pcd = new PostcardDialog(this);

  pcd->show();
  delete pcd;
}
