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

#include <QCheckBox>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfig>
#include <KGuiItem>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney/converter/webpricequote.h"
#include "mymoneyfile.h"
#include "icons/icons.h"

using namespace Icons;

KSettingsOnlineQuotes::KSettingsOnlineQuotes(QWidget *parent)
    : KSettingsOnlineQuotesDecl(parent),
    m_quoteInEditing(false)
{
  QStringList groups = WebPriceQuote::quoteSources();

  loadList(true /*updateResetList*/);

  m_updateButton->setEnabled(false);

  KGuiItem updateButtenItem(i18nc("Accepts the entered data and stores it", "&Update"),
                            QIcon::fromTheme(g_Icons[Icon::DialogOK]),
                            i18n("Accepts the entered data and stores it"),
                            i18n("Use this to accept the modified data."));
  KGuiItem::assign(m_updateButton, updateButtenItem);

  KGuiItem deleteButtenItem(i18n("&Delete"),
                            QIcon::fromTheme(g_Icons[Icon::EditDelete]),
                            i18n("Delete the selected source entry"),
                            i18n("Use this to delete the selected online source entry"));
  KGuiItem::assign(m_deleteButton, deleteButtenItem);

  KGuiItem newButtenItem(i18nc("Create a new source entry for online quotes", "&New..."),
                         QIcon::fromTheme(g_Icons[Icon::DocumentNew]),
                         i18n("Create a new source entry for online quotes"),
                         i18n("Use this to create a new entry for online quotes"));
  KGuiItem::assign(m_newButton, newButtenItem);

  m_editIdentifyBy->addItem(i18n("Symbol"), WebPriceQuoteSource::identifyBy::Symbol);
  m_editIdentifyBy->addItem(i18n("Identification number"), WebPriceQuoteSource::identifyBy::IdentificationNumber);
  m_editIdentifyBy->addItem(i18n("Name"), WebPriceQuoteSource::identifyBy::Name);

  connect(m_dumpCSVProfile, SIGNAL(clicked()), this, SLOT(slotDumpCSVProfile()));
  connect(m_updateButton, SIGNAL(clicked()), this, SLOT(slotUpdateEntry()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNewEntry()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(slotDeleteEntry()));

  connect(m_quoteSourceList, SIGNAL(itemSelectionChanged()), this, SLOT(slotLoadWidgets()));
  connect(m_quoteSourceList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(slotEntryRenamed(QListWidgetItem*)));
  connect(m_quoteSourceList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotStartRename(QListWidgetItem*)));

  connect(m_editURL, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editCSVURL, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editIdentifier, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editIdentifyBy, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editDate, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editDateFormat, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_editPrice, SIGNAL(textChanged(QString)), this, SLOT(slotEntryChanged()));
  connect(m_skipStripping, SIGNAL(toggled(bool)), this, SLOT(slotEntryChanged()));
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
  m_editCSVURL->setEnabled(true);
  m_editIdentifier->setEnabled(true);
  m_editIdentifyBy->setEnabled(true);
  m_editPrice->setEnabled(true);
  m_editDate->setEnabled(true);
  m_editDateFormat->setEnabled(true);
  m_skipStripping->setEnabled(true);
  m_dumpCSVProfile->setEnabled(true);
  m_deleteButton->setEnabled(true);
  m_editURL->setText(QString());
  m_editCSVURL->setText(QString());
  m_editIdentifier->setText(QString());
  m_editIdentifyBy->setCurrentIndex(WebPriceQuoteSource::identifyBy::Symbol);
  m_editPrice->setText(QString());
  m_editDate->setText(QString());
  m_editDateFormat->setText(QString());

  if (item) {
    m_currentItem = WebPriceQuoteSource(item->text());
    m_editURL->setText(m_currentItem.m_url);
    m_editCSVURL->setText(m_currentItem.m_csvUrl);
    m_editIdentifier->setText(m_currentItem.m_webID);
    m_editIdentifyBy->setCurrentIndex(m_currentItem.m_webIDBy);
    m_editPrice->setText(m_currentItem.m_price);
    m_editDate->setText(m_currentItem.m_date);
    m_editDateFormat->setText(m_currentItem.m_dateformat);
    m_skipStripping->setChecked(m_currentItem.m_skipStripping);
  } else {
    m_editURL->setEnabled(false);
    m_editCSVURL->setEnabled(false);
    m_editIdentifier->setEnabled(false);
    m_editIdentifyBy->setEnabled(false);
    m_editPrice->setEnabled(false);
    m_editDate->setEnabled(false);
    m_editDateFormat->setEnabled(false);
    m_skipStripping->setEnabled(false);
    m_dumpCSVProfile->setEnabled(false);
    m_deleteButton->setEnabled(false);
  }

  m_updateButton->setEnabled(false);

}

void KSettingsOnlineQuotes::slotEntryChanged()
{
  bool modified = m_editURL->text() != m_currentItem.m_url
                  || m_editCSVURL->text() != m_currentItem.m_csvUrl
                  || m_editIdentifier->text() != m_currentItem.m_webID
                  || m_editIdentifyBy->currentData().toInt() != static_cast<int>(m_currentItem.m_webIDBy)
                  || m_editDate->text() != m_currentItem.m_date
                  || m_editDateFormat->text() != m_currentItem.m_dateformat
                  || m_editPrice->text() != m_currentItem.m_price
                  || m_skipStripping->isChecked() != m_currentItem.m_skipStripping;

  m_updateButton->setEnabled(modified);
}

void KSettingsOnlineQuotes::slotDumpCSVProfile()
{
  KSharedConfigPtr config = CSVImporter::configFile();
  PricesProfile profile;
  profile.m_profileName = m_currentItem.m_name;
  profile.m_profileType = Profile::StockPrices;
  bool profileExists = false;
  bool writeProfile = true;

  if (profile.readSettings(config))
    profileExists = true;
  else {
    profile.m_profileType = Profile::CurrencyPrices;
    if (profile.readSettings(config))
      profileExists = true;
  }

  if (profileExists)
    writeProfile = (KMessageBox::questionYesNoCancel(this,
                                                     i18n("CSV profile <b>%1</b> already exists.<br>"
                                                          "Do you want to overwrite it?",
                                                          m_currentItem.m_name),
                                                     i18n("CSV Profile Already Exists")) == KMessageBox::Yes ? true : false);

  if (writeProfile) {
    QMap<QString, PricesProfile> quoteSources = WebPriceQuote::defaultCSVQuoteSources();
    profile = quoteSources.value(m_currentItem.m_name);
    if (profile.m_profileName.compare(m_currentItem.m_name, Qt::CaseInsensitive) == 0) {
      profile.writeSettings(config);
      CSVImporter::profilesAction(profile.type(), ProfileAction::Add, profile.m_profileName, profile.m_profileName);
    }
  }
  CSVImporter::profilesAction(profile.type(), ProfileAction::UpdateLastUsed, profile.m_profileName, profile.m_profileName);
}

void KSettingsOnlineQuotes::slotUpdateEntry()
{
  m_currentItem.m_url = m_editURL->text();
  m_currentItem.m_csvUrl = m_editCSVURL->text();
  m_currentItem.m_webID = m_editIdentifier->text();
  m_currentItem.m_webIDBy = static_cast<WebPriceQuoteSource::identifyBy>(m_editIdentifyBy->currentData().toInt());
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

void KSettingsOnlineQuotes::slotDeleteEntry()
{
  // first check if no security is using this online source
  QList<MyMoneySecurity> securities = MyMoneyFile::instance()->securityList();
  foreach(const auto security, securities) {
    if (security.value(QStringLiteral("kmm-online-source")).compare(m_currentItem.m_name) == 0) {
      if (KMessageBox::questionYesNo(this,
                                     i18n("Security <b>%1</b> uses this quote source.<br>"
                                          "Do you really want to remove it?", security.name()),
                                     i18n("Delete quote source")) == KMessageBox::Yes)
        break;  // webpricequote can handle missing online quotes, so proceed without any extra action
      else
        return;
    }
  }

  // remove online source from webpricequote...
  m_currentItem.remove();

  // ...and from setting's list
  int row = m_quoteSourceList->currentRow();
  QListWidgetItem *item = m_quoteSourceList->takeItem(row);
  if (item)
    delete item;
  item = nullptr;

  int count = m_quoteSourceList->count();
  if (row < count)                        // select next available entry...
    item = m_quoteSourceList->item(row);
  else if (row >= count && count > 0)    // ...or last entry if this was the last entry...
    item = m_quoteSourceList->item(count - 1);

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
  m_quoteSourceList->sortItems();
  m_newButton->setEnabled(m_quoteSourceList->findItems(i18n("New Quote Source"), Qt::MatchExactly).count() == 0);
}
