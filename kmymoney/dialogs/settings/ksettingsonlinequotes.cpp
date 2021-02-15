/*
    SPDX-FileCopyrightText: 2005-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ksettingsonlinequotes.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfig>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsonlinequotes.h"

#include "kmymoney/misc/webpricequote.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "icons/icons.h"

using namespace Icons;

class KSettingsOnlineQuotesPrivate
{
  Q_DISABLE_COPY(KSettingsOnlineQuotesPrivate)

public:
  KSettingsOnlineQuotesPrivate() :
    ui(new Ui::KSettingsOnlineQuotes),
    m_quoteInEditing(false)
  {
  }

  ~KSettingsOnlineQuotesPrivate()
  {
    delete ui;
  }

  Ui::KSettingsOnlineQuotes  *ui;
  QList<WebPriceQuoteSource>  m_resetList;
  WebPriceQuoteSource         m_currentItem;
  bool                        m_quoteInEditing;
};

KSettingsOnlineQuotes::KSettingsOnlineQuotes(QWidget *parent) :
  QWidget(parent),
  d_ptr(new KSettingsOnlineQuotesPrivate)
{
  Q_D(KSettingsOnlineQuotes);
  d->ui->setupUi(this);
  QStringList groups = WebPriceQuote::quoteSources();

  loadList(true /*updateResetList*/);

  d->ui->m_updateButton->setEnabled(false);

  d->ui->m_updateButton->setIcon(Icons::get(Icon::DialogOK));
  d->ui->m_deleteButton->setIcon(Icons::get(Icon::EditRemove));
  d->ui->m_newButton->setIcon(Icons::get(Icon::DocumentNew));

  d->ui->m_editIdentifyBy->addItem(i18nc("@item:inlistbox Stock", "Symbol"), WebPriceQuoteSource::identifyBy::Symbol);
  d->ui->m_editIdentifyBy->addItem(i18nc("@item:inlistbox Stock", "Identification number"), WebPriceQuoteSource::identifyBy::IdentificationNumber);
  d->ui->m_editIdentifyBy->addItem(i18nc("@item:inlistbox Stock", "Name"), WebPriceQuoteSource::identifyBy::Name);

  connect(d->ui->m_dumpCSVProfile, &QAbstractButton::clicked, this, &KSettingsOnlineQuotes::slotDumpCSVProfile);
  connect(d->ui->m_updateButton, &QAbstractButton::clicked, this, &KSettingsOnlineQuotes::slotUpdateEntry);
  connect(d->ui->m_newButton, &QAbstractButton::clicked, this, &KSettingsOnlineQuotes::slotNewEntry);
  connect(d->ui->m_deleteButton, &QAbstractButton::clicked, this, &KSettingsOnlineQuotes::slotDeleteEntry);

  connect(d->ui->m_quoteSourceList, &QListWidget::itemSelectionChanged, this, &KSettingsOnlineQuotes::slotLoadWidgets);
  connect(d->ui->m_quoteSourceList, &QListWidget::itemChanged, this, &KSettingsOnlineQuotes::slotEntryRenamed);
  connect(d->ui->m_quoteSourceList, &QListWidget::itemDoubleClicked, this, &KSettingsOnlineQuotes::slotStartRename);

  connect(d->ui->m_editURL, &QLineEdit::textChanged, this,                                                        static_cast<void (KSettingsOnlineQuotes::*)(const QString &)>(&KSettingsOnlineQuotes::slotEntryChanged));
  connect(d->ui->m_editCSVURL, &QLineEdit::textChanged, this,                                                     static_cast<void (KSettingsOnlineQuotes::*)(const QString &)>(&KSettingsOnlineQuotes::slotEntryChanged));
  connect(d->ui->m_editIdentifier, &QLineEdit::textChanged, this,                                                 static_cast<void (KSettingsOnlineQuotes::*)(const QString &)>(&KSettingsOnlineQuotes::slotEntryChanged));
  connect(d->ui->m_editIdentifyBy, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,  static_cast<void (KSettingsOnlineQuotes::*)(int)>(&KSettingsOnlineQuotes::slotEntryChanged));
  connect(d->ui->m_editDate, &QLineEdit::textChanged, this,                                                       static_cast<void (KSettingsOnlineQuotes::*)(const QString &)>(&KSettingsOnlineQuotes::slotEntryChanged));
  connect(d->ui->m_editDateFormat, &QLineEdit::textChanged, this,                                                 static_cast<void (KSettingsOnlineQuotes::*)(const QString &)>(&KSettingsOnlineQuotes::slotEntryChanged));
  connect(d->ui->m_editPrice, &QLineEdit::textChanged, this,                                                      static_cast<void (KSettingsOnlineQuotes::*)(const QString &)>(&KSettingsOnlineQuotes::slotEntryChanged));
  connect(d->ui->m_skipStripping, &QAbstractButton::toggled, this,                                                static_cast<void (KSettingsOnlineQuotes::*)(bool)>(&KSettingsOnlineQuotes::slotEntryChanged));
}

KSettingsOnlineQuotes::~KSettingsOnlineQuotes()
{
  Q_D(KSettingsOnlineQuotes);
  delete d;
}

void KSettingsOnlineQuotes::loadList(const bool updateResetList)
{
  Q_D(KSettingsOnlineQuotes);
  //disconnect the slot while items are being loaded and reconnect at the end
  disconnect(d->ui->m_quoteSourceList, &QListWidget::itemChanged, this, &KSettingsOnlineQuotes::slotEntryRenamed);
  d->m_quoteInEditing = false;
  QStringList groups = WebPriceQuote::quoteSources();

  if (updateResetList)
    d->m_resetList.clear();
  d->ui->m_quoteSourceList->clear();
  QStringList::Iterator it;
  for (it = groups.begin(); it != groups.end(); ++it) {
    QListWidgetItem* item = new QListWidgetItem(*it);
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    d->ui->m_quoteSourceList->addItem(item);
    if (updateResetList)
      d->m_resetList += WebPriceQuoteSource(*it);
  }
  d->ui->m_quoteSourceList->sortItems();

  QListWidgetItem* first = d->ui->m_quoteSourceList->item(0);
  if (first)
    d->ui->m_quoteSourceList->setCurrentItem(first);
  slotLoadWidgets();

  d->ui->m_newButton->setEnabled((d->ui->m_quoteSourceList->findItems(i18n("New Quote Source"), Qt::MatchExactly)).count() == 0);
  connect(d->ui->m_quoteSourceList, &QListWidget::itemChanged, this, &KSettingsOnlineQuotes::slotEntryRenamed);
}

void KSettingsOnlineQuotes::resetConfig()
{
  Q_D(KSettingsOnlineQuotes);
  QStringList::ConstIterator it;
  QStringList groups = WebPriceQuote::quoteSources();

  // delete all currently defined entries
  for (it = groups.constBegin(); it != groups.constEnd(); ++it) {
    WebPriceQuoteSource(*it).remove();
  }

  // and write back the one's from the reset list
  QList<WebPriceQuoteSource>::ConstIterator itr;
  for (itr = d->m_resetList.constBegin(); itr != d->m_resetList.constEnd(); ++itr) {
    (*itr).write();
  }

  loadList();
}

void KSettingsOnlineQuotes::slotLoadWidgets()
{
  Q_D(KSettingsOnlineQuotes);
  d->m_quoteInEditing = false;
  QListWidgetItem* item = d->ui->m_quoteSourceList->currentItem();

  d->ui->m_editURL->setEnabled(true);
  d->ui->m_editCSVURL->setEnabled(true);
  d->ui->m_editIdentifier->setEnabled(true);
  d->ui->m_editIdentifyBy->setEnabled(true);
  d->ui->m_editPrice->setEnabled(true);
  d->ui->m_editDate->setEnabled(true);
  d->ui->m_editDateFormat->setEnabled(true);
  d->ui->m_skipStripping->setEnabled(true);
  d->ui->m_dumpCSVProfile->setEnabled(true);
  d->ui->m_deleteButton->setEnabled(true);
  d->ui->m_editURL->setText(QString());
  d->ui->m_editCSVURL->setText(QString());
  d->ui->m_editIdentifier->setText(QString());
  d->ui->m_editIdentifyBy->setCurrentIndex(WebPriceQuoteSource::identifyBy::Symbol);
  d->ui->m_editPrice->setText(QString());
  d->ui->m_editDate->setText(QString());
  d->ui->m_editDateFormat->setText(QString());

  if (item) {
    d->m_currentItem = WebPriceQuoteSource(item->text());
    d->ui->m_editURL->setText(d->m_currentItem.m_url);
    d->ui->m_editCSVURL->setText(d->m_currentItem.m_csvUrl);
    d->ui->m_editIdentifier->setText(d->m_currentItem.m_webID);
    d->ui->m_editIdentifyBy->setCurrentIndex(d->m_currentItem.m_webIDBy);
    d->ui->m_editPrice->setText(d->m_currentItem.m_price);
    d->ui->m_editDate->setText(d->m_currentItem.m_date);
    d->ui->m_editDateFormat->setText(d->m_currentItem.m_dateformat);
    d->ui->m_skipStripping->setChecked(d->m_currentItem.m_skipStripping);
  } else {
    d->ui->m_editURL->setEnabled(false);
    d->ui->m_editCSVURL->setEnabled(false);
    d->ui->m_editIdentifier->setEnabled(false);
    d->ui->m_editIdentifyBy->setEnabled(false);
    d->ui->m_editPrice->setEnabled(false);
    d->ui->m_editDate->setEnabled(false);
    d->ui->m_editDateFormat->setEnabled(false);
    d->ui->m_skipStripping->setEnabled(false);
    d->ui->m_dumpCSVProfile->setEnabled(false);
    d->ui->m_deleteButton->setEnabled(false);
  }

  d->ui->m_updateButton->setEnabled(false);

}

void KSettingsOnlineQuotes::slotEntryChanged()
{
  Q_D(KSettingsOnlineQuotes);
  bool modified = d->ui->m_editURL->text() != d->m_currentItem.m_url
                  || d->ui->m_editCSVURL->text() != d->m_currentItem.m_csvUrl
                  || d->ui->m_editIdentifier->text() != d->m_currentItem.m_webID
                  || d->ui->m_editIdentifyBy->currentData().toInt() != static_cast<int>(d->m_currentItem.m_webIDBy)
                  || d->ui->m_editDate->text() != d->m_currentItem.m_date
                  || d->ui->m_editDateFormat->text() != d->m_currentItem.m_dateformat
                  || d->ui->m_editPrice->text() != d->m_currentItem.m_price
                  || d->ui->m_skipStripping->isChecked() != d->m_currentItem.m_skipStripping;

  d->ui->m_updateButton->setEnabled(modified);
}

void KSettingsOnlineQuotes::slotEntryChanged(int)
{
  slotEntryChanged();
}

void KSettingsOnlineQuotes::slotEntryChanged(const QString&)
{
  slotEntryChanged();
}

void KSettingsOnlineQuotes::slotEntryChanged(bool)
{
  slotEntryChanged();
}

void KSettingsOnlineQuotes::slotDumpCSVProfile()
{
  Q_D(KSettingsOnlineQuotes);
  KSharedConfigPtr config = CSVImporterCore::configFile();
  PricesProfile profile;
  profile.m_profileName = d->m_currentItem.m_name;
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
                                                          d->m_currentItem.m_name),
                                                     i18n("CSV Profile Already Exists")) == KMessageBox::Yes ? true : false);

  if (writeProfile) {
    QMap<QString, PricesProfile> quoteSources = WebPriceQuote::defaultCSVQuoteSources();
    profile = quoteSources.value(d->m_currentItem.m_name);
    if (profile.m_profileName.compare(d->m_currentItem.m_name, Qt::CaseInsensitive) == 0) {
      profile.writeSettings(config);
      CSVImporterCore::profilesAction(profile.type(), ProfileAction::Add, profile.m_profileName, profile.m_profileName);
    }
  }
  CSVImporterCore::profilesAction(profile.type(), ProfileAction::UpdateLastUsed, profile.m_profileName, profile.m_profileName);
}

void KSettingsOnlineQuotes::slotUpdateEntry()
{
  Q_D(KSettingsOnlineQuotes);
  d->m_currentItem.m_url = d->ui->m_editURL->text();
  d->m_currentItem.m_csvUrl = d->ui->m_editCSVURL->text();
  d->m_currentItem.m_webID = d->ui->m_editIdentifier->text();
  d->m_currentItem.m_webIDBy = static_cast<WebPriceQuoteSource::identifyBy>(d->ui->m_editIdentifyBy->currentData().toInt());
  d->m_currentItem.m_date = d->ui->m_editDate->text();
  d->m_currentItem.m_dateformat = d->ui->m_editDateFormat->text();
  d->m_currentItem.m_price = d->ui->m_editPrice->text();
  d->m_currentItem.m_skipStripping = d->ui->m_skipStripping->isChecked();
  d->m_currentItem.write();
  slotEntryChanged();
}

void KSettingsOnlineQuotes::slotNewEntry()
{
  Q_D(KSettingsOnlineQuotes);
  WebPriceQuoteSource newSource(i18n("New Quote Source"));
  newSource.write();
  loadList();
  QListWidgetItem* item = d->ui->m_quoteSourceList->findItems(i18n("New Quote Source"), Qt::MatchExactly).at(0);
  if (item) {
    d->ui->m_quoteSourceList->setCurrentItem(item);
    slotLoadWidgets();
  }
}

void KSettingsOnlineQuotes::slotDeleteEntry()
{
  Q_D(KSettingsOnlineQuotes);
  // first check if no security is using this online source
  auto securities = MyMoneyFile::instance()->securityList();
  foreach(const auto security, securities) {
    if (security.value(QStringLiteral("kmm-online-source")).compare(d->m_currentItem.m_name) == 0) {
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
  d->m_currentItem.remove();

  // ...and from setting's list
  auto row = d->ui->m_quoteSourceList->currentRow();
  QListWidgetItem *item = d->ui->m_quoteSourceList->takeItem(row);
  if (item)
    delete item;
  item = nullptr;

  int count = d->ui->m_quoteSourceList->count();
  if (row < count)                        // select next available entry...
    item = d->ui->m_quoteSourceList->item(row);
  else if (row >= count && count > 0)    // ...or last entry if this was the last entry...
    item = d->ui->m_quoteSourceList->item(count - 1);

  if (item) {
    d->ui->m_quoteSourceList->setCurrentItem(item);
    slotLoadWidgets();
  }
}

void KSettingsOnlineQuotes::slotStartRename(QListWidgetItem* item)
{
  Q_D(KSettingsOnlineQuotes);
  d->m_quoteInEditing = true;
  d->ui->m_quoteSourceList->editItem(item);
}

void KSettingsOnlineQuotes::slotEntryRenamed(QListWidgetItem* item)
{
  Q_D(KSettingsOnlineQuotes);
  //if there is no current item selected, exit
  if (d->m_quoteInEditing == false || !d->ui->m_quoteSourceList->currentItem() || item != d->ui->m_quoteSourceList->currentItem())
    return;

  d->m_quoteInEditing = false;
  QString text = item->text();
  int nameCount = 0;
  for (auto i = 0; i < d->ui->m_quoteSourceList->count(); ++i) {
    if (d->ui->m_quoteSourceList->item(i)->text() == text)
      ++nameCount;
  }

  // Make sure we get a non-empty and unique name
  if (text.length() > 0 && nameCount == 1) {
    d->m_currentItem.rename(text);
  } else {
    item->setText(d->m_currentItem.m_name);
  }
  d->ui->m_quoteSourceList->sortItems();
  d->ui->m_newButton->setEnabled(d->ui->m_quoteSourceList->findItems(i18n("New Quote Source"), Qt::MatchExactly).count() == 0);
}
