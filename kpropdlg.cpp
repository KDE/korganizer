/***************************************************************

     $Id$

     Requires the Qt and KDE widget libraries, available at no cost at
     http://www.troll.no and http://www.kde.org respectively

     Copyright (C) 1997, 1998 Fester Zigterman ( fzr@dds.nl )

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


***************************************************************/

#include <klocale.h>
#include <kconfig.h>

#include "kpropdlg.h"
#include "kpropdlg.moc"
#include "kapp.h"
//#include "kpanner.h"
#include <qsplitter.h>
#include <qvbox.h>

KPropDlg::KPropDlg( int dlgtype, int buttons, const char *title, 
		    QWidget *parent, const char *name, bool modal )
  : QDialog( parent, name, modal )
{
  // Set window props
  setCaption( title );
  
  // Set some characteristics
  PageList = new QList<QWidget>;
  TreeItemList = new QList<QListViewItem>;
  DlgType = dlgtype;
  Buttons = buttons;
  TreeWidth = 150;
  ActivePage = 0;
  
  QFontMetrics fm( font() );
  //	int bbsize = fm.height() + fm.ascent() + 32;
  
  if( DlgType == TREE ) {
    QSplitter *splitter = new QSplitter( this, "propDlgPanner" );
//    KPanner *panner = new KPanner( this, "propDlgPanner" );
//    panner->setSeparator( 30 );

//    QVBox *lpane = new QVBox(splitter,"lpane");
//    TreeList = new QListView( lpane );
    TreeList = new QListView( splitter );
    TreeList->setRootIsDecorated(true);
    TreeList->addColumn(i18n("Categories"));
    TreeList->setMinimumWidth(100);
//    TreeList = new KTreeList( panner->child0() );
//    (new QHBoxLayout( lpane ))->addWidget( TreeList );
//    connect(TreeList, SIGNAL(highlighted(int)),this,SLOT(showPage(int)));
    connect(TreeList, SIGNAL(selectionChanged(QListViewItem *)),
            this,SLOT(showPage(QListViewItem *)));
    
//  rpane = panner->child1() ;
    rpane = new QFrame(splitter,"rpane");
    VLayout = new QVBoxLayout( rpane, 0, -1, "topVLayout" );
      
    // Page title label
    Title = new QLabel( "Unnamed Dialog", rpane );
    Title->setFrameStyle( QFrame::Panel|QFrame::Raised );
    Title->setFixedHeight( 20 );
    Title->setText( title );
    VLayout->addWidget( Title );
      
    PageFrame = new QVBox( rpane, "PageFrame" );
//    PageFrame = new QFrame( rpane, "PageFrame" );
    PageFrame->setFrameStyle( QFrame::Panel|QFrame::Raised);
    PageFrame->installEventFilter( this );
    VLayout->addWidget( PageFrame );


    ButtonLayout = new QVBoxLayout( this );
//    ButtonLayout->addWidget( panner, 0, AlignRight );
    ButtonLayout->addWidget( splitter );
    QFrame *hline = new QFrame(this);
    hline->setFrameStyle(QFrame::Sunken|QFrame::HLine);
    hline->setFixedHeight(hline->sizeHint().height());
    //	hline->setMinimumWidth(hline->sizeHint().width());
    ButtonLayout->addWidget( hline );
    ButtonWidget = new QLabel( this );
    ButtonLayout->addWidget( ButtonWidget, 0, AlignRight );
      
      
  } else if( DlgType == TABS ) {
    TabControl = new KTabCtl( this );
    ButtonWidget = new QLabel( this );
    ButtonLayout = new QVBoxLayout( this );
    ButtonLayout->addWidget( TabControl, 0, AlignRight );
    ButtonLayout->addWidget( ButtonWidget, 0, AlignRight );
  } else {
    VLayout = new QVBoxLayout( this, 0, -1, "topVLayout" );
    
    // Page title label
    Title = new QLabel( "Unnamed Dialog", this );
    Title->setFrameStyle( QFrame::Panel|QFrame::Raised );
    Title->setFixedHeight( 20 );
    Title->setText( title );
    VLayout->addWidget( Title );
    
    PageFrame = new QFrame( this, "PageFrame" );
    PageFrame->setFrameStyle( QFrame::Panel|QFrame::Raised);
    PageFrame->installEventFilter( this );
    VLayout->addWidget( PageFrame );
    
    ButtonWidget = new QLabel( this );
    //		ButtonLayout = new QVBoxLayout( this );
    //		ButtonLayout->addWidget( PageFrame, 0, AlignRight );
    //		ButtonLayout->addWidget( ButtonWidget, 0, AlignRight );
    QFrame *hline = new QFrame(this);
    hline->setFrameStyle(QFrame::Sunken|QFrame::HLine);
    hline->setFixedHeight(hline->sizeHint().height());
    //	hline->setMinimumWidth(hline->sizeHint().width());
    ButtonLayout->addWidget( hline );
    VLayout->addWidget( ButtonWidget, 0, AlignRight );
  }
  
  // fill the button layout with widgets
  
  PrevButton = new QPushButton( i18n("Prev"), ButtonWidget );
  NextButton = new QPushButton( i18n("Next"), ButtonWidget  );
  ApplyButton = new QPushButton( i18n("Apply"), ButtonWidget );
  OKButton = new QPushButton( i18n("OK"), ButtonWidget );
  CancelButton = new QPushButton( i18n("Cancel"), ButtonWidget );
  
  resizeButtons();
  connectButtons();
  
  OKButton->setDefault(TRUE);
  DefaultButton = OKButton;

}

void KPropDlg::connectButtons( int buttons )
{
  // connect the button signals to their handlers
  
  connect (PrevButton, SIGNAL(clicked()), SLOT(receivePrev()));
  connect (NextButton, SIGNAL(clicked()), SLOT(receiveNext()));
  connect (ApplyButton, SIGNAL(clicked()), SLOT(receiveApply()));
  connect( OKButton, SIGNAL(clicked()), SLOT(receiveOK()));
  connect (CancelButton, SIGNAL(clicked()), SLOT(receiveCancel()));
  
  // connect the handler signals to their standard methods
  
  if( buttons&OK )
    connect(this, SIGNAL(OKClicked()), SLOT(acceptConfig()));
  if( buttons&CANCEL )
    connect(this, SIGNAL(CancelClicked()), SLOT(cancelConfig()));
  if( buttons&APPLY )	
    connect(this, SIGNAL(ApplyClicked()), SLOT(applyConfig()));
  
  if( buttons&PREV)
    connect( this, SIGNAL(PrevClicked() ), SLOT( slotPrev() ) );
  if( buttons&NEXT )
    connect( this, SIGNAL(NextClicked() ), SLOT( slotNext() ) );
  
}

KPropDlg::~KPropDlg()
{
}

QFrame *KPropDlg::addPage( int parentindex, const char *title, int position )
{
  //  QVBoxLayout *templayout;
  QFrame *page; 
  
  
  if( DlgType == TABS )
    page = new QFrame( TabControl, title );
  else
    page = new QFrame( PageFrame, title );
  
  
  page->setFrameStyle( QFrame::NoFrame);
  page->setEnabled( FALSE );
  page->hide();
  
  //	debug("addPage");
  if( DlgType == TABS ) {
      TabControl->addTab( page, title );
  } else {
    if( DlgType == TREE ) {
      TreeItemList->append(new QListViewItem(TreeList,title));
//	TreeList->insertItem( title, 0, parentindex );
    }
    /*    templayout = new QVBoxLayout( PageFrame,2 );
    templayout->addWidget( page );
    templayout->activate();*/
    
    page->lower();
  }
  PageList->append( page );
  return page;
}

void KPropDlg::showPage(QWidget *w)
{
  int stat=0;
    
  // bring the new page up front
  w->raise();
  
  // deactivate old page 
  if( ActivePage ) {
    ActivePage->setEnabled( FALSE );
    ActivePage->hide();
  } 
  // and select new one
  ActivePage=w;
  ActivePage->setEnabled( TRUE );
  ActivePage->show();
  
  // change the header for the dialog
  // highlight the item in the tree view, most important for
  // first instantiation
  
  int index = PageList->findRef(w);
  if( DlgType == TREE ) {
    Title->setText(w->name());
    TreeList->setSelected(TreeItemList->at(index),true);
//    TreeList->setCurrentItem(index);
  }
  
  stat = pageListStatus();
  //	debug("stat %X, %d, %d",stat, (stat&LASTPAGE)==0, !(stat&FIRSTPAGE)==0 );
  
  if( Buttons&NEXT ) NextButton->setEnabled( (stat&LASTPAGE)==0  );
  if( Buttons&PREV ) PrevButton->setEnabled( (stat&FIRSTPAGE)==0 );
  
}

bool KPropDlg::showPage(int index)
{
	// find the page in our list
  QWidget *page = PageList->at( index );
  
  // debug("display page %d, %X", index, page);
  
  if( page == 0 ) {
    debug("error in page num %d", index);
    return FALSE;
  }
  
  // request that it be brought up front
  showPage(page);
  
  return TRUE;
}

bool KPropDlg::showPage(QListViewItem *item)
{
  // find the page in our list  
  uint i;
  for ( i=0; i < TreeItemList->count(); ++i )
    if ( TreeItemList->at(i) == item ) {
      showPage( PageList->at(i) );
      return true;
    }

  qDebug("KPropDlg::showPage(QListViewItem *): item not found.");
  return false;
}

bool KPropDlg::removePage( char *page, int index )
{
  debug("Removing pages is not yet implemented.");
  return FALSE;
}

void KPropDlg::setButtonsEnabled( int types )
{
  ActiveButtons = types;
  PrevButton->setEnabled(  types&PREV  != 0 );
  NextButton->setEnabled(  types&NEXT  != 0 );
  ApplyButton->setEnabled( types&APPLY != 0 );
  OKButton->setEnabled(    types&OK    != 0 );
  CancelButton->setEnabled(types&CANCEL!= 0 );
}

int  KPropDlg::buttonsEnabled()
{
  return ActiveButtons;
}

void KPropDlg::setButton( int type, const char *text )
{
}

void KPropDlg::setDefaultButton( int type )
{
  DefaultButton->setDefault( FALSE );
  switch( type ) {
  case OK:
    DefaultButton = OKButton;
    break;
  case CANCEL:
    DefaultButton = CancelButton;
    break;
  case PREV:
		DefaultButton = PrevButton;
		break;
  case NEXT:
    DefaultButton = NextButton;
    break;
  case APPLY:
    DefaultButton = ApplyButton;
    break;
  default:	
    ;
  }
  DefaultButton->setDefault( TRUE );
  resizeButtons();
}

void KPropDlg::resizeButtons()
{
  QFontMetrics fm( font() );
  int bheight = fm.height() + fm.ascent();
  int bwid = 0;
  int bcount = 0;
  
  bwid = QMAX( PrevButton->sizeHint().width(), bwid );
  bwid = QMAX( NextButton->sizeHint().width(), bwid );
  bwid = QMAX( ApplyButton->sizeHint().width(), bwid );
  bwid = QMAX( OKButton->sizeHint().width(), bwid );
  bwid = QMAX( CancelButton->sizeHint().width(), bwid );
  
  if( Buttons&PREV ) {
    PrevButton->move( (bcount++)*(bwid+4)+8, 8 );
  } else
    PrevButton->hide();
	
  if( Buttons&NEXT ) {
    NextButton->move( (bcount++)*(bwid+4)+8, 8 );
  } else
    NextButton->hide();

  if( Buttons&APPLY ) {
    ApplyButton->move( (bcount++)*(bwid+4)+12, 8 );
  } else 
    ApplyButton->hide();

  if( Buttons&OK ) {
    OKButton->move( (bcount++)*(bwid+4)+12, 8 );
  } else
    OKButton->hide();
  
  if( Buttons&CANCEL ) {
    CancelButton->move( (bcount++)*(bwid+4)+16, 8 );
  } else
    CancelButton->hide();
	
  ButtonWidget->setFixedSize( bcount*(bwid+4) + 20, bheight + 16 );
  
  OKButton->setFixedSize( bwid, bheight );
  CancelButton->setFixedSize( bwid, bheight );
  ApplyButton->setFixedSize( bwid, bheight );
  PrevButton->setFixedSize( bwid, bheight );
  NextButton->setFixedSize( bwid, bheight );
}	

void KPropDlg::receiveOK()
{
  emit OKClicked();
}

void KPropDlg::receiveCancel() {
  emit CancelClicked();
}

void KPropDlg::receiveNext() {
  emit NextClicked();
}

void KPropDlg::receivePrev() {
  emit PrevClicked();
}

void KPropDlg::receiveApply() {
  emit ApplyClicked();
}

void KPropDlg::slotNext()
{
  int stat = pageListStatus();
  // debug("statnext %X, %d, %d",stat, (stat&LASTPAGE)==0, !(stat&FIRSTPAGE)==0 );
  
  if( stat < NOCURRENT &&stat !=0 ) {
    debug("NOCURRENT");
    return;
  }
  PageList->next();
  // debug("count:%d at:%d", PageList->count(), PageList->at() );
  showPage( PageList->at() );
}

void KPropDlg::slotPrev()
{
  int stat = pageListStatus();
  
  if( stat < NOCURRENT && stat !=0 ) return;
  PageList->prev();
  
  showPage( PageList->at() );
}


int KPropDlg::pageListStatus()
{
  int status = 0;
  if( ! PageList ) return NOLIST;
  if( ! PageList->count() ) return NOPAGES;
  if( ! PageList->current() ) return NOCURRENT;
  if( PageList->current() == PageList->getLast() ) status = LASTPAGE;
  if( PageList->current() == PageList->getFirst() ) status = FIRSTPAGE;
  return status;
}

void KPropDlg::acceptConfig()
{
  // Write configuration and exit dialog cleanly.
  
  emit setConfig();
  kapp->config()->sync();
  emit configChanged();
  accept();
}

void KPropDlg::cancelConfig()
{
  // Don't write configuration and cancel dialog.
  reject();
}

void KPropDlg::applyConfig()
{
  // Write configuration and return to dialog.
  //debug("applyConfig");
  emit setConfig();
  emit configChanged();
}

void KPropDlg::connectConfig( QObject * propconf )
{
  connect( this, SIGNAL( setConfig() ), 
	   propconf, SLOT( setConfig() ));
  connect( this, SIGNAL( getConfig() ), 
	   propconf, SLOT( getConfig() ));
}

bool KPropDlg::eventFilter( QObject *obj, QEvent *ev )
{
  if( ev->type() == QEvent::Resize ) {
    adjustPageHeight();
    if( ( (QResizeEvent*)ev )->size().height() < minimumPageHeight )
      PageFrame->setMinimumHeight( minimumPageHeight );
  }
  return FALSE;
}

void KPropDlg::adjustPageHeight()
{
  // calculate the minimum size
  int index = PageList->at();
  minimumPageHeight = 0;

  for( PageList->first(); PageList->current(); PageList->next() ) {
    minimumPageHeight = QMAX( minimumPageHeight, 
			      PageList->current()->minimumSize().height() );
  }

  PageFrame->setMinimumHeight( (minimumPageHeight += 10) );
  PageList->at(index);
}
