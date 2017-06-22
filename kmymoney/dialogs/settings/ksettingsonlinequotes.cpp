/***************************************************************************
                          ksettingsonlinequotes.cpp
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

#include "ksettingsonlinequotes.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
#include <QCheckBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney/converter/webpricequote.h"

KSettingsOnlineQuotes::KSettingsOnlineQuotes(QWidget *parent)
    : KSettingsOnlineQuotesDecl(parent),
    m_quoteInEditing(false)
{
  QStringList groups = WebPriceQuote::quoteSources();

  loadList(true /*updateResetList*/);

  m_updateButton->setEnabled(false);

  KGuiItem updateButtenItem(i18nc("Accepts the entered data and stores it", "&Update"),
                            KIcon("dialog-ok"),
                            i18n("Accepts the entered data and stores it"),
                            i18n("Use this to accept the modified data."));
  m_updateButton->setGuiItem(updateButtenItem);

  KGuiItem deleteButtenItem(i18n("&Delete"),
                            KIcon("edit-delete"),
                            i18n("Delete the selected source entry"),
                            i18n("Use this to delete the selected online source entry"));
  m_deleteButton->setGuiItem(deleteButtenItem);

  KGuiItem newButtenItem(i18nc("Create a new source entry for online quotes", "&New..."),
                         KIcon("document-new"),
                         i18n("Create a new source entry for online quotes"),
                         i18n("Use this to create a new entry for online quotes"));
  m_newButton->setGuiItem(newButtenItem);

  connect(m_updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateEntry()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNewEntry()));

  connect(m_quoteSourceList, SIGNAL(itemSelectionChanged()), this, SLOT(slotLoadWidgets()));
  connect(m_quoteSourceList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(slotEntryRenamed(QListWidgetItem*)));
  connect(m_quoteSourceList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotStartRename(QListWidgetItem*)));

  connect(m_editURL, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editSymbol, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editDate, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editDateFormat, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editPrice, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_skipStripping, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));

  // FIXME deleting a source is not yet implemented
  m_deleteButton->setEnabled(false);
}

void KSettingsOnlineQuotes::loadList(const bool updateResetList)
{
  //disconnect the slot while items are being loaded and reconnect at the end
  disconnect(m_quoteSourceList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(slotEntryRenamed(QListWidgetItem*)));
  m_quoteInEditing = false;
  QStringList groups = WebPriceQuote::quoteSources();

  if (updateResetList)
    m_resetList.clear();
  m_quoteSourceList->clear();
  QStringList::Iterator it;
  for (it = groups.begin(); it != groups.end(); ++it) {
    QListWidgetItem* item = new QListWidgetItem(*it);
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    m_quoteSourceList->addItem(item);
    if (updateResetList)
      m_resetList += WebPriceQuoteSource(*it);
  }
  m_quoteSourceList->sortItems();

  QListWidgetItem* first = m_quoteSourceList->item(0);
  if (first)
    m_quoteSourceList->setCurrentItem(first);
  slotLoadWidgets();

  m_newButton->setEnabled((m_quoteSourceList->findItems(i18n("New Quote Source"), Qt::MatchExactly)).count() == 0);
  connect(m_quoteSourceList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(slotEntryRenamed(QListWidgetItem*)));
}

void KSettingsOnlineQuotes::resetConfig()
{
  QStringList::ConstIterator it;
  QStringList groups = WebPriceQuote::quoteSources();

  // delete all currently defined entries
  for (it = groups.constBegin(); it != groups.constEnd(); ++it) {
    WebPriceQuoteSource(*it).remove();
  }

  // and write back the one's from the reset list
  QList<WebPriceQuoteSource>::ConstIterator itr;
  for (itr = m_resetList.constBegin(); itr != m_resetList.constEnd(); ++itr) {
    (*itr).write();
  }

  loadList();
}

void KSettingsOnlineQuotes::slotLoadWidgets()
{
  m_quoteInEditing = false;
  QListWidgetItem* item = m_quoteSourceList->currentItem();

  m_editURL->setEnabled(true);
  m_editSymbol->setEnabled(true);
  m_editPrice->setEnabled(true);
  m_editDate->setEnabled(true);
  m_editDateFormat->setEnabled(true);
  m_skipStripping->setEnabled(true);
  m_editURL->setText(QString());
  m_editSymbol->setText(QString());
  m_editPrice->setText(QString());
  m_editDate->setText(QString());
  m_editDateFormat->setText(QString());

  if (item) {
    m_currentItem = WebPriceQuoteSource(item->text());
    m_editURL->setText(m_currentItem.m_url);
    m_editSymbol->setText(m_currentItem.m_sym);
    m_editPrice->setText(m_currentItem.m_price);
    m_editDate->setText(m_currentItem.m_date);
    m_editDateFormat->setText(m_currentItem.m_dateformat);
    m_skipStripping->setChecked(m_currentItem.m_skipStripping);
  } else {
    m_editURL->setEnabled(false);
    m_editSymbol->setEnabled(false);
    m_editPrice->setEnabled(false);
    m_editDate->setEnabled(false);
    m_editDateFormat->setEnabled(false);
    m_skipStripping->setEnabled(false);
  }

  m_updateButton->setEnabled(false);

}

void KSettingsOnlineQuotes::slotEntryChanged()
{
  bool modified = m_editURL->text() != m_currentItem.m_url
                  || m_editSymbol->text() != m_currentItem.m_sym
                  || m_editDate->text() != m_currentItem.m_date
                  || m_editDateFormat->text() != m_currentItem.m_dateformat
                  || m_editPrice->text() != m_currentItem.m_price
                  || m_skipStripping->isChecked() != m_currentItem.m_skipStripping;

  m_updateButton->setEnabled(modified);
}

void KSettingsOnlineQuotes::slotUpdateEntry()
{
  m_currentItem.m_url = m_editURL->text();
  m_currentItem.m_sym = m_editSymbol->text();
  m_currentItem.m_date = m_editDate->text();
  m_currentItem.m_dateformat = m_editDateFormat->text();
  m_currentItem.m_price = m_editPrice->text();
  m_currentItem.m_skipStripping = m_skipStripping->isChecked();
  m_currentItem.write();
  slotEntryChanged();
}

void KSettingsOnlineQuotes::slotNewEntry()
{
  WebPriceQuoteSource newSource(i18n("New Quote Source"));
  newSource.write();
  loadList();
  QListWidgetItem* item = m_quoteSourceList->findItems(i18n("New Quote Source"), Qt::MatchExactly).at(0);
  if (item) {
    m_quoteSourceList->setCurrentItem(item);
    slotLoadWidgets();
  }
}

void KSettingsOnlineQuotes::slotStartRename(QListWidgetItem* item)
{
  m_quoteInEditing = true;
  m_quoteSourceList->editItem(item);
}

void KSettingsOnlineQuotes::slotEntryRenamed(QListWidgetItem* item)
{
  //if there is no current item selected, exit
  if (m_quoteInEditing == false || !m_quoteSourceList->currentItem() || item != m_quoteSourceList->currentItem())
    return;

  m_quoteInEditing = false;
  QString text = item->text();
  int nameCount = 0;
  for (int i = 0; i < m_quoteSourceList->count(); ++i) {
    if (m_quoteSourceList->item(i)->text() == text)
      ++nameCount;
  }

  // Make sure we get a non-empty and unique name
  if (text.length() > 0 && nameCount == 1) {
    m_currentItem.rename(text);
  } else {
    item->setText(m_currentItem.m_name);
  }
  m_newButton->setEnabled(m_quoteSourceList->findItems(i18n("New Quote Source"), Qt::MatchExactly).count() == 0);
}
