/***************************************************************************
                          KStartDlg.cpp  -  description
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config-kmymoney.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <q3vbox.h>
#include <QLayout>
#include <q3buttongroup.h>
#include <QPixmap>
#include <q3textview.h>
#include <QLabel>
//Added by qt3to4:
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes



#include <kstandarddirs.h>

#include <kiconloader.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kstartdlg.h"
#include "krecentfileitem.h"
#include "kmymoney2.h"

#include <QToolTip>

KStartDlg::KStartDlg(QWidget *parent, const char *name, bool modal)
    : KPageDialog(parent)
{
    setCaption( i18n("Start Dialog") );
    setModal( true );
    setFaceType( List );
    setButtons( Ok | Cancel|Help );
    setDefaultButton( Ok );
  setPage_Template();
  setPage_Documents();

  isnewfile = false;
  isopenfile = false;

  readConfig();
}

KStartDlg::~KStartDlg()
{
}

/** Set the font  Page of the preferences dialog */
void KStartDlg::setPage_Template()
{
  KIconLoader *ic = KIconLoader::global();
  templateMainFrame = new KVBox;

  KPageWidgetItem *page = new KPageWidgetItem( templateMainFrame , i18n("Select templates") );
  page->setHeader( i18n("Templates") );
  page->setIcon( KIcon("tools-wizard"));
  addPage(page);


  view_wizard = new K3IconView( templateMainFrame, "view_options" );
  (void)new Q3IconViewItem( view_wizard, i18n("New KMyMoney document"), ic->loadIcon("mime_empty.png", KIconLoader::Desktop, KIconLoader::SizeLarge)/*QPixmap( locate("icon","hicolor/48x48/mimetypes/mime_empty.png") )*/ );
  connect(view_wizard, SIGNAL(executed(Q3IconViewItem *) ), this, SLOT(slotTemplateClicked(Q3IconViewItem *) ) );
  connect(view_wizard, SIGNAL(selectionChanged(Q3IconViewItem*)),
    this, SLOT(slotTemplateSelectionChanged(Q3IconViewItem*)));
  connect(this, SIGNAL(aboutToShowPage(QWidget*)), this, SLOT(slotAboutToShowPage(QWidget*)));
}

/** Set the Misc options Page of the preferences dialog */
void KStartDlg::setPage_Documents()
{


  recentMainFrame = new KVBox;

  KPageWidgetItem *page = new KPageWidgetItem( recentMainFrame , i18n("Open a KMyMoney document") );
  page->setHeader( i18n("Open") );
  page->setIcon( KIcon("fileopen"));
  addPage(page);

  QVBoxLayout *mainLayout = new QVBoxLayout( recentMainFrame );

  kurlrequest = new KUrlRequester( recentMainFrame );

  //allow user to select either a .kmy file, or any generic file.
  kurlrequest->fileDialog()->setFilter( i18n("*.kmy|KMyMoney files (*.kmy)\n*.*|All files (*.*)") );
  kurlrequest->fileDialog()->setMode(KFile::File | KFile::ExistingOnly);
  kurlrequest->fileDialog()->setUrl(KUrl(kmymoney2->readLastUsedDir()));//kurlrequest->fileDialog()->setURL(KUrl(KGlobalSettings::documentPath()));
  mainLayout->addWidget( kurlrequest );

  QLabel *label1 = new QLabel( recentMainFrame );
  label1->setText( i18n("Recent Files") );
  mainLayout->addWidget( label1 );
  view_recent = new K3IconView( recentMainFrame, "view_recent" );
  connect( view_recent, SIGNAL( executed(Q3IconViewItem *) ), this, SLOT( slotRecentClicked(Q3IconViewItem *) ) );
  mainLayout->addWidget( view_recent );
  view_recent->setArrangement(K3IconView::LeftToRight/*TopToBottom*/);
  view_recent->setItemTextPos(K3IconView::Bottom);

  connect(view_recent, SIGNAL(selectionChanged(Q3IconViewItem*)),
    this, SLOT(slotRecentSelectionChanged(Q3IconViewItem*)));
}

void KStartDlg::slotTemplateClicked(Q3IconViewItem *item)
{
  if(!item) return;

  // If the item is the blank document turn isnewfile variable true, else is template or wizard
  if( item->text() == i18n("New KMyMoney document") )
     isnewfile = true;
   else
     templatename = item->text();

  isopenfile = false;
  // Close the window if the user pressed an icon
  slotOk();
}

/** Read config window */
void KStartDlg::readConfig()
{
  QString value;
  unsigned int i = 1;

  KSharedConfigPtr config = KGlobal::config();
  KIconLoader *il = KIconLoader::global();

  // read file list
  do {
    // for some reason, I had to setup the group to get reasonable results
    // after program startup. If the wizard was opened the second time,
    // it does not make a difference, if you call setGroup() outside of
    // this loop. The first time it does make a difference!
    KConfigGroup grp= config->group("Recent Files");
    value = grp.readEntry( QString( "File%1" ).arg( i ));
    if( !value.isNull() && fileExists(value) )
    {
      QString file_name = value.mid(value.findRev('/')+1);
      (void)new KRecentFileItem( value, view_recent, file_name, il->loadIcon("kmy", KIconLoader::Desktop, KIconLoader::SizeLarge));
    }
    i++;
  } while( !value.isNull() );

  KConfigGroup grp = config->group("Start Dialog");
  QSize defaultSize(400,300);
  this->resize( grp.readEntry("Geometry", defaultSize ) );

  // Restore the last page viewed
  // default to the recent files page if no entry exists but files have been found
  // otherwise, default to template page
#warning "port to kde4"
#if 0
  if(view_recent->count() > 0)
    setCurrentPage(grp.readEntry("LastPage", this->pageIndex(recentMainFrame)));
  else {
    setCurrentPage(grp.readEntry("LastPage", this->pageIndex(templateMainFrame)));
    slotAboutToShowPage(templateMainFrame);
  }
#endif
}

/** Write config window */
void KStartDlg::writeConfig()
{
  KSharedConfigPtr config = KGlobal::config();

  KConfigGroup grp = config->group("Start Dialog");
  grp.writeEntry("Geometry", this->size() );
#warning "port to kde4"
  //grp.writeEntry("LastPage", this->activePageIndex());
  config->sync();
}

/** slot to recent view */
void KStartDlg::slotRecentClicked(Q3IconViewItem *item)
{
  KRecentFileItem *kitem = (KRecentFileItem*)item;
  if(!kitem) return;

  isopenfile = true;
  kurlrequest->setUrl( kitem->fileURL() );
  // Close the window if the user press an icon
  slotOk();
}

/** No descriptions */
void KStartDlg::slotOk()
{
  writeConfig();
  this->accept();
}

bool KStartDlg::fileExists(KUrl url)
{
  return KIO::NetAccess::exists(url, true, this);
}

void KStartDlg::slotTemplateSelectionChanged(Q3IconViewItem* item)
{
  if(!item) return;

  // Clear the other selection
  view_recent->clearSelection();

  // If the item is the blank document turn isnewfile
  // variable true, else is template or wizard
  if( item->text() == i18n("Blank Document") )
    isnewfile = true;
  else
    templatename = item->text();

  isopenfile = false;
}

void KStartDlg::slotRecentSelectionChanged(Q3IconViewItem* item)
{
  KRecentFileItem *kitem = (KRecentFileItem*)item;
  if(!kitem) return;

  // Clear the other selection
  view_wizard->clearSelection();

  isnewfile = false;
  isopenfile = true;
  kurlrequest->setUrl( kitem->fileURL() );
}

void KStartDlg::slotAboutToShowPage(QWidget* page)
{
  enableButtonOk(page == recentMainFrame);
}

#include "kstartdlg.moc"
