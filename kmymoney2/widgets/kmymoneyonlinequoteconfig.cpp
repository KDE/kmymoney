/***************************************************************************
                          kmymoneyonlinequoteconfig.cpp  -  description
                             -------------------
    begin                : Thu Dec 30 2004
    copyright            : (C) 2004 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <qregexp.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <k3listview.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyonlinequoteconfig.h"
#include "../converter/webpricequote.h"

kMyMoneyOnlineQuoteConfig::kMyMoneyOnlineQuoteConfig(QWidget *parent )
  : kMyMoneyOnlineQuoteConfigDecl(parent)
{
#if 1
  QStringList groups = WebPriceQuote::quoteSources();

  loadList(true /*updateResetList*/);

  m_updateButton->setEnabled(false);

  KIconLoader* il = KIconLoader::global();
  KGuiItem updateButtenItem( i18n("&Update" ),
                    KIcon(il->loadIcon("button_ok", KIconLoader::Small, KIconLoader::SizeSmall)),
                    i18n("Accepts the entered data and stores it"),
                    i18n("Use this to accept the modified data."));
  m_updateButton->setGuiItem(updateButtenItem);

  KGuiItem deleteButtenItem( i18n( "&Delete" ),
                      KIcon(il->loadIcon("editdelete", KIconLoader::Small, KIconLoader::SizeSmall)),
                      i18n("Delete the selected source entry"),
                      i18n("Use this to delete the selected online source entry"));
  m_deleteButton->setGuiItem(deleteButtenItem);

  KGuiItem newButtenItem( i18n( "&New..." ),
                      KIcon(il->loadIcon("filenew", KIconLoader::Small, KIconLoader::SizeSmall)),
                      i18n("Create a new source entry for online quotes"),
                      i18n("Use this to create a new entry for online quotes"));
  m_newButton->setGuiItem(newButtenItem);

  connect(m_updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateEntry()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNewEntry()));

  connect(m_quoteSourceList, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotLoadWidgets(Q3ListViewItem*)));
  connect(m_quoteSourceList, SIGNAL(clicked(Q3ListViewItem*)), this, SLOT(slotLoadWidgets(Q3ListViewItem*)));
  connect(m_quoteSourceList, SIGNAL(itemRenamed(Q3ListViewItem*,const QString&,int)), this, SLOT(slotEntryRenamed(Q3ListViewItem*,const QString&,int)));

  connect(m_editURL, SIGNAL(textChanged(const QString&)), this, SLOT(slotEntryChanged()));
  connect(m_editSymbol, SIGNAL(textChanged(const QString&)), this, SLOT(slotEntryChanged()));
  connect(m_editDate, SIGNAL(textChanged(const QString&)), this, SLOT(slotEntryChanged()));
  connect(m_editDateFormat, SIGNAL(textChanged(const QString&)), this, SLOT(slotEntryChanged()));
  connect(m_editPrice, SIGNAL(textChanged(const QString&)), this, SLOT(slotEntryChanged()));

  // FIXME deleting a source is not yet implemented
  m_deleteButton->setEnabled(false);
#endif
}

void kMyMoneyOnlineQuoteConfig::loadList(const bool updateResetList)
{
  QStringList groups = WebPriceQuote::quoteSources();

  if(updateResetList)
    m_resetList.clear();
  m_quoteSourceList->clear();
  QStringList::Iterator it;
  for(it = groups.begin(); it != groups.end(); ++it) {
    new Q3ListViewItem(m_quoteSourceList, *it);
    if(updateResetList)
      m_resetList += WebPriceQuoteSource(*it);
  }

  Q3ListViewItem* first = m_quoteSourceList->firstChild();
  if(first)
    m_quoteSourceList->setSelected(first, true);
  slotLoadWidgets(first);

  m_newButton->setEnabled(m_quoteSourceList->findItem(i18n("New Quote Source"), 0) == 0);
}

void kMyMoneyOnlineQuoteConfig::resetConfig(void)
{
  QStringList::ConstIterator it;
  QStringList groups = WebPriceQuote::quoteSources();

  // delete all currently defined entries
  for(it = groups.begin(); it != groups.end(); ++it) {
    WebPriceQuoteSource(*it).remove();
  }

  // and write back the one's from the reset list
  Q3ValueList<WebPriceQuoteSource>::ConstIterator itr;
  for(itr = m_resetList.begin(); itr != m_resetList.end(); ++itr) {
    (*itr).write();
  }

  loadList();
}

void kMyMoneyOnlineQuoteConfig::slotLoadWidgets(Q3ListViewItem* item)
{
  m_editURL->setEnabled(true);
  m_editSymbol->setEnabled(true);
  m_editPrice->setEnabled(true);
  m_editDate->setEnabled(true);
  m_editURL->setText(QString());
  m_editSymbol->setText(QString());
  m_editPrice->setText(QString());
  m_editDate->setText(QString());
  m_editDateFormat->setText(QString());

  if(item) {
    m_currentItem = WebPriceQuoteSource(item->text(0));
    m_editURL->setText(m_currentItem.m_url);
    m_editSymbol->setText(m_currentItem.m_sym);
    m_editPrice->setText(m_currentItem.m_price);
    m_editDate->setText(m_currentItem.m_date);
    m_editDateFormat->setText(m_currentItem.m_dateformat);

  } else {
    m_editURL->setEnabled(false);
    m_editSymbol->setEnabled(false);
    m_editPrice->setEnabled(false);
    m_editDate->setEnabled(false);
    m_editDateFormat->setEnabled(false);
  }

  m_updateButton->setEnabled(false);

}

void kMyMoneyOnlineQuoteConfig::slotEntryChanged(void)
{
  bool modified = m_editURL->text() != m_currentItem.m_url
               || m_editSymbol->text() != m_currentItem.m_sym
               || m_editDate->text() != m_currentItem.m_date
               || m_editDateFormat->text() != m_currentItem.m_dateformat
               || m_editPrice->text() != m_currentItem.m_price;

  m_updateButton->setEnabled(modified);
}

void kMyMoneyOnlineQuoteConfig::slotUpdateEntry(void)
{
  m_currentItem.m_url = m_editURL->text();
  m_currentItem.m_sym = m_editSymbol->text();
  m_currentItem.m_date = m_editDate->text();
  m_currentItem.m_dateformat = m_editDateFormat->text();
  m_currentItem.m_price = m_editPrice->text();
  m_currentItem.write();
  slotEntryChanged();
}

void kMyMoneyOnlineQuoteConfig::slotNewEntry(void)
{
  WebPriceQuoteSource newSource(i18n("New Quote Source"));
  newSource.write();
  loadList();
  Q3ListViewItem* item = m_quoteSourceList->findItem(i18n("New Quote Source"), 0);
  if(item) {
    m_quoteSourceList->setSelected(item, true);
    slotLoadWidgets(item);
  }
}

void kMyMoneyOnlineQuoteConfig::slotEntryRenamed(Q3ListViewItem* item, const QString& text, int /* col */)
{
  int nameCount = 0;
  Q3ListViewItemIterator it(m_quoteSourceList);
  while(it.current()) {
    if(it.current()->text(0) == text)
      ++nameCount;
    ++it;
  }

  // Make sure we get a non-empty and unique name
  if(text.length() > 0 && nameCount == 1) {
    m_currentItem.rename(text);
  } else {
    item->setText(0, m_currentItem.m_name);
  }
  m_newButton->setEnabled(m_quoteSourceList->findItem(i18n("New Quote Source"), 0) == 0);
}

