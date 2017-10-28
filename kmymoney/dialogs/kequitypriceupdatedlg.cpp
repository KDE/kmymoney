/***************************************************************************
                          kequitypriceupdatedlg.cpp  -  description
                             -------------------
    begin                : Mon Sep 1 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
                           2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kequitypriceupdatedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QTimer>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KTextEdit>
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KStandardGuiItem>
#include <KLocalizedString>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"

#define WEBID_COL       0
#define NAME_COL        1
#define PRICE_COL       2
#define DATE_COL        3
#define KMMID_COL       4
#define SOURCE_COL      5

KEquityPriceUpdateDlg::KEquityPriceUpdateDlg(QWidget *parent, const QString& securityId) :
    KEquityPriceUpdateDlgDecl(parent),
    m_fUpdateAll(false)
{
  QStringList headerList;
  headerList << i18n("ID") << i18nc("Equity name", "Name")
  << i18n("Price") << i18n("Date");

  lvEquityList->header()->setSortIndicator(0, Qt::AscendingOrder);
  lvEquityList->setColumnWidth(NAME_COL, 125);

  // This is a "get it up and running" hack.  Will replace this in the future.
  headerList << i18nc("Internal identifier", "Internal ID")
  << i18nc("Online quote source", "Source");
  lvEquityList->setColumnWidth(KMMID_COL, 0);

  lvEquityList->setHeaderLabels(headerList);

  lvEquityList->setSelectionMode(QAbstractItemView::MultiSelection);
  lvEquityList->setAllColumnsShowFocus(true);

  btnUpdateAll->setEnabled(false);

  MyMoneyFile* file = MyMoneyFile::instance();

  //
  // Add each price pair that we know about
  //

  // send in securityId == "XXX YYY" to get a single-shot update for XXX to YYY.
  // for consistency reasons, this accepts the same delimiters as WebPriceQuote::launch()
  QRegExp splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)", Qt::CaseInsensitive);
  MyMoneySecurityPair currencyIds;
  if (splitrx.indexIn(securityId) != -1) {
    currencyIds = MyMoneySecurityPair(splitrx.cap(1), splitrx.cap(2));
  }

  MyMoneyPriceList prices = file->priceList();
  for (MyMoneyPriceList::ConstIterator it_price = prices.constBegin(); it_price != prices.constEnd(); ++it_price) {
    const MyMoneySecurityPair& pair = it_price.key();
    if (file->security(pair.first).isCurrency() && (securityId.isEmpty() || (pair == currencyIds))) {
      const MyMoneyPriceEntries& entries = (*it_price);
      if (entries.count() > 0 && entries.begin().key() <= QDate::currentDate()) {
        addPricePair(pair);
        btnUpdateAll->setEnabled(true);
      }
    }
  }

  //
  // Add each investment
  //

  QList<MyMoneySecurity> securities = file->securityList();
  for (QList<MyMoneySecurity>::const_iterator it = securities.constBegin(); it != securities.constEnd(); ++it) {
    if (!(*it).isCurrency()
        && (securityId.isEmpty() || ((*it).id() == securityId))
        && !(*it).value("kmm-online-source").isEmpty()
       ) {
      addInvestment(*it);
      btnUpdateAll->setEnabled(true);
    }
  }

  // if list is empty, add the request price pair
  if (lvEquityList->invisibleRootItem()->childCount() == 0) {
    addPricePair(currencyIds, true);
  }

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(btnUpdateSelected, SIGNAL(clicked()), this, SLOT(slotUpdateSelectedClicked()));
  connect(btnUpdateAll, SIGNAL(clicked()), this, SLOT(slotUpdateAllClicked()));

  connect(m_fromDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChanged()));
  connect(m_toDate, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChanged()));

  connect(&m_webQuote, &WebPriceQuote::csvquote,
          this, &KEquityPriceUpdateDlg::slotReceivedCSVQuote);
  connect(&m_webQuote, SIGNAL(quote(QString,QString,QDate,double)),
          this, SLOT(slotReceivedQuote(QString,QString,QDate,double)));
  connect(&m_webQuote, SIGNAL(failed(QString,QString)),
          this, SLOT(slotQuoteFailed(QString,QString)));
  connect(&m_webQuote, SIGNAL(status(QString)),
          this, SLOT(logStatusMessage(QString)));
  connect(&m_webQuote, SIGNAL(error(QString)),
          this, SLOT(logErrorMessage(QString)));

  connect(lvEquityList, SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateSelection()));

  connect(btnConfigure, &QAbstractButton::clicked, this, &KEquityPriceUpdateDlg::slotConfigureClicked);

  if (!securityId.isEmpty()) {
    btnUpdateSelected->hide();
    btnUpdateAll->hide();
    // delete layout1;

    QTimer::singleShot(100, this, SLOT(slotUpdateAllClicked()));
  }

  // Hide OK button until we have received the first update
  buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

  slotUpdateSelection();

  // previous versions of this dialog allowed to store a "Don't ask again" switch.
  // Since we don't support it anymore, we just get rid of it
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Notification Messages");
  grp.deleteEntry("KEquityPriceUpdateDlg::slotQuoteFailed::Price Update Failed");
  grp.sync();
  grp = config->group("Equity Price Update");
  int policyValue = grp.readEntry("PriceUpdatingPolicy", QString().setNum(eUpdateMissingPrices)).toInt();
  if (policyValue >= eUpdatingPricePolicyEnd || policyValue < eUpdateAllPrices)
    m_updatingPricePolicy = eUpdateMissingPrices;
  else
    m_updatingPricePolicy = static_cast<updatingPricePolicyE>(policyValue);
}

KEquityPriceUpdateDlg::~KEquityPriceUpdateDlg()
{
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Equity Price Update");
  grp.writeEntry("PriceUpdatingPolicy", static_cast<int>(m_updatingPricePolicy));
  grp.sync();
}

void KEquityPriceUpdateDlg::addPricePair(const MyMoneySecurityPair& pair, bool dontCheckExistance)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  const QString symbol = QString::fromLatin1("%1 > %2").arg(pair.first, pair.second);
  const QString id = QString::fromLatin1("%1 %2").arg(pair.first, pair.second);
  // Check that the pair does not already exist
  if (lvEquityList->findItems(id, Qt::MatchExactly, KMMID_COL).empty()) {
    const MyMoneyPrice &pr = file->price(pair.first, pair.second);
    if (pr.source() != QLatin1String("KMyMoney")) {
      bool keep = true;
      if ((pair.first == file->baseCurrency().id())
          || (pair.second == file->baseCurrency().id())) {
        const QString& foreignCurrency = file->foreignCurrency(pair.first, pair.second);
        // check that the foreign currency is still in use
        QList<MyMoneyAccount>::const_iterator it_a;
        QList<MyMoneyAccount> list;
        file->accountList(list);
        for (it_a = list.constBegin(); !dontCheckExistance && it_a != list.constEnd(); ++it_a) {
          // if it's an account denominated in the foreign currency
          // keep it
          if (((*it_a).currencyId() == foreignCurrency)
              && !(*it_a).isClosed())
            break;
          // if it's an investment traded in the foreign currency
          // keep it
          if ((*it_a).isInvest() && !(*it_a).isClosed()) {
            MyMoneySecurity sec = file->security((*it_a).currencyId());
            if (sec.tradingCurrency() == foreignCurrency)
              break;
          }
        }
        // if it is in use, it_a is not equal to list.end()
        if (it_a == list.constEnd() && !dontCheckExistance)
          keep = false;
      }

      if (keep) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(WEBID_COL, symbol);
        item->setText(NAME_COL, i18n("%1 units in %2", pair.first, pair.second));
        if (pr.isValid()) {
          MyMoneySecurity fromCurrency = file->currency(pair.second);
          MyMoneySecurity toCurrency = file->currency(pair.first);
          item->setText(PRICE_COL, pr.rate(pair.second).formatMoney(fromCurrency.tradingSymbol(), toCurrency.pricePrecision()));
          item->setText(DATE_COL, pr.date().toString(Qt::ISODate));
        }
        item->setText(KMMID_COL, id);
        item->setText(SOURCE_COL, "Yahoo Currency");  // This string value should not be localized
        lvEquityList->invisibleRootItem()->addChild(item);
      }
    }
  }
}

void KEquityPriceUpdateDlg::addInvestment(const MyMoneySecurity& inv)
{
  const QString id = inv.id();
  // Check that the pair does not already exist
  if (lvEquityList->findItems(id, Qt::MatchExactly, KMMID_COL).empty()) {
    MyMoneyFile* file = MyMoneyFile::instance();
    // check that the security is still in use
    QList<MyMoneyAccount>::const_iterator it_a;
    QList<MyMoneyAccount> list;
    file->accountList(list);
    for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
      if ((*it_a).isInvest()
          && ((*it_a).currencyId() == inv.id())
          && !(*it_a).isClosed())
        break;
    }
    // if it is in use, it_a is not equal to list.end()
    if (it_a != list.constEnd()) {
      QString webID;
      WebPriceQuoteSource onlineSource(inv.value("kmm-online-source"));
      if (onlineSource.m_webIDBy == WebPriceQuoteSource::identifyBy::IdentificationNumber)
        webID = inv.value("kmm-security-id");   // insert ISIN number...
      else if (onlineSource.m_webIDBy == WebPriceQuoteSource::identifyBy::Name)
        webID = inv.name();                     // ...or name...
      else
        webID = inv.tradingSymbol();            // ...or symbol

      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setForeground(WEBID_COL, KColorScheme(QPalette::Normal).foreground(KColorScheme::NormalText));
      if (webID.isEmpty()) {
        webID = i18n("[No identifier]");
        item->setForeground(WEBID_COL, KColorScheme(QPalette::Normal).foreground(KColorScheme::NegativeText));
      }
      item->setText(WEBID_COL, webID);
      item->setText(NAME_COL, inv.name());
      MyMoneySecurity currency = file->currency(inv.tradingCurrency());
      const MyMoneyPrice &pr = file->price(id.toUtf8(), inv.tradingCurrency());
      if (pr.isValid()) {
        item->setText(PRICE_COL, pr.rate(currency.id()).formatMoney(currency.tradingSymbol(), inv.pricePrecision()));
        item->setText(DATE_COL, pr.date().toString(Qt::ISODate));
      }
      item->setText(KMMID_COL, id);
      if (inv.value("kmm-online-quote-system") == "Finance::Quote")
        item->setText(SOURCE_COL, QString("Finance::Quote %1").arg(inv.value("kmm-online-source")));
      else
        item->setText(SOURCE_COL, inv.value("kmm-online-source"));

      lvEquityList->invisibleRootItem()->addChild(item);

      // If this investment is denominated in a foreign currency, ensure that
      // the appropriate price pair is also on the list

      if (currency.id() != file->baseCurrency().id()) {
        addPricePair(MyMoneySecurityPair(currency.id(), file->baseCurrency().id()));
      }
    }
  }
}

void KEquityPriceUpdateDlg::logErrorMessage(const QString& message)
{
  logStatusMessage(QString("<font color=\"red\"><b>") + message + QString("</b></font>"));
}

void KEquityPriceUpdateDlg::logStatusMessage(const QString& message)
{
  lbStatus->append(message);
}

MyMoneyPrice KEquityPriceUpdateDlg::price(const QString& id) const
{
  MyMoneyPrice price;
  QTreeWidgetItem* item = 0;
  QList<QTreeWidgetItem*> foundItems = lvEquityList->findItems(id, Qt::MatchExactly, KMMID_COL);

  if (! foundItems.empty())
    item = foundItems.at(0);

  if (item) {
    MyMoneyMoney rate(item->text(PRICE_COL));
    if (!rate.isZero()) {
      QString id = item->text(KMMID_COL).toUtf8();

      // if the ID has a space, then this is TWO ID's, so it's a currency quote
      if (id.contains(" ")) {
        QStringList ids = id.split(' ', QString::SkipEmptyParts);
        QString fromid = ids[0].toUtf8();
        QString toid = ids[1].toUtf8();
        price = MyMoneyPrice(fromid, toid, QDate().fromString(item->text(DATE_COL), Qt::ISODate), rate, item->text(SOURCE_COL));
      } else
        // otherwise, it's a security quote
      {
        MyMoneySecurity security = MyMoneyFile::instance()->security(id);
        price = MyMoneyPrice(id, security.tradingCurrency(), QDate().fromString(item->text(DATE_COL), Qt::ISODate), rate, item->text(SOURCE_COL));
      }
    }
  }
  return price;
}

void KEquityPriceUpdateDlg::storePrices()
{
  // update the new prices into the equities

  MyMoneyFile* file = MyMoneyFile::instance();
  QString name;

  MyMoneyFileTransaction ft;
  try {
    for (int i = 0; i < lvEquityList->invisibleRootItem()->childCount(); ++i) {
      QTreeWidgetItem* item = lvEquityList->invisibleRootItem()->child(i);
      // turn on signals before we modify the last entry in the list
      file->blockSignals(i < lvEquityList->invisibleRootItem()->childCount() - 1);

      MyMoneyMoney rate(item->text(PRICE_COL));
      if (!rate.isZero()) {
        QString id = item->text(KMMID_COL);
        QString fromid;
        QString toid;

        // if the ID has a space, then this is TWO ID's, so it's a currency quote
        if (id.contains(QLatin1Char(' '))) {
          QStringList ids = id.split(QLatin1Char(' '), QString::SkipEmptyParts);
          fromid = ids.at(0);
          toid = ids.at(1);
          name = QString::fromLatin1("%1 --> %2").arg(fromid, toid);
        } else { // otherwise, it's a security quote
          MyMoneySecurity security = file->security(id);
          name = security.name();
          fromid = id;
          toid = security.tradingCurrency();
        }
        // TODO (Ace) Better handling of the case where there is already a price
        // for this date.  Currently, it just overrides the old value.  Really it
        // should check to see if the price is the same and prompt the user.
        file->addPrice(MyMoneyPrice(fromid, toid, QDate::fromString(item->text(DATE_COL), Qt::ISODate), rate, item->text(SOURCE_COL)));
      }
    }
    ft.commit();

  } catch (const MyMoneyException &) {
    qDebug("Unable to add price information for %s", qPrintable(name));
  }
}

void KEquityPriceUpdateDlg::slotConfigureClicked()
{
  EquityPriceUpdateConfDlg *dlg = new EquityPriceUpdateConfDlg(m_updatingPricePolicy);
  if (dlg->exec() == QDialog::Accepted)
    m_updatingPricePolicy = dlg->policy();
  delete dlg;
}

void KEquityPriceUpdateDlg::slotUpdateSelection()
{
  // Only enable the update button if there is a selection
  btnUpdateSelected->setEnabled(false);

  if (! lvEquityList->selectedItems().empty())
    btnUpdateSelected->setEnabled(true);
}

void KEquityPriceUpdateDlg::slotUpdateSelectedClicked()
{
  // disable sorting while the update is running to maintain the current order of items on which
  // the update process depends and which could be changed with sorting enabled due to the updated values
  lvEquityList->setSortingEnabled(false);
  QTreeWidgetItem* item = lvEquityList->invisibleRootItem()->child(0);
  int skipCnt = 1;
  while (item && !item->isSelected()) {
    item = lvEquityList->invisibleRootItem()->child(skipCnt);
    ++skipCnt;
  }
  m_webQuote.setDate(m_fromDate->date(), m_toDate->date());
  if (item) {
    prgOnlineProgress->setMaximum(1 + lvEquityList->invisibleRootItem()->childCount());
    prgOnlineProgress->setValue(skipCnt);
    m_webQuote.launch(item->text(WEBID_COL), item->text(KMMID_COL), item->text(SOURCE_COL));

  } else {

    logErrorMessage("No security selected.");
  }
}

void KEquityPriceUpdateDlg::slotUpdateAllClicked()
{
  // disable sorting while the update is running to maintain the current order of items on which
  // the update process depends and which could be changed with sorting enabled due to the updated values
  lvEquityList->setSortingEnabled(false);
  QTreeWidgetItem* item = lvEquityList->invisibleRootItem()->child(0);
  if (item) {
    prgOnlineProgress->setMaximum(1 + lvEquityList->invisibleRootItem()->childCount());
    prgOnlineProgress->setValue(1);
    m_fUpdateAll = true;
    m_webQuote.launch(item->text(WEBID_COL), item->text(KMMID_COL), item->text(SOURCE_COL));

  } else {
    logErrorMessage("Security list is empty.");
  }
}

void KEquityPriceUpdateDlg::slotDateChanged()
{
  m_fromDate->blockSignals(true);
  m_toDate->blockSignals(true);
  if (m_toDate->date() > QDate::currentDate())
    m_toDate->setDate(QDate::currentDate());
  if (m_toDate->date() < m_fromDate->date())
    m_fromDate->setDate(m_toDate->date());
  m_fromDate->blockSignals(false);
  m_toDate->blockSignals(false);
}

void KEquityPriceUpdateDlg::slotQuoteFailed(const QString& _kmmID, const QString& _webID)
{
  QList<QTreeWidgetItem*> foundItems = lvEquityList->findItems(_kmmID, Qt::MatchExactly, KMMID_COL);
  QTreeWidgetItem* item = 0;

  if (! foundItems.empty())
    item = foundItems.at(0);

  // Give the user some options
  int result;
  if (_kmmID.contains(" ")) {
    result = KMessageBox::warningContinueCancel(this, i18n("Failed to retrieve an exchange rate for %1 from %2. It will be skipped this time.", _webID, item->text(SOURCE_COL)), i18n("Price Update Failed"));
  } else {
    result = KMessageBox::questionYesNoCancel(this, QString("<qt>%1</qt>").arg(i18n("Failed to retrieve a quote for %1 from %2.  Press <b>No</b> to remove the online price source from this security permanently, <b>Yes</b> to continue updating this security during future price updates or <b>Cancel</b> to stop the current update operation.", _webID, item->text(SOURCE_COL))), i18n("Price Update Failed"), KStandardGuiItem::yes(), KStandardGuiItem::no());
  }

  if (result == KMessageBox::No) {
    // Disable price updates for this security

    MyMoneyFileTransaction ft;
    try {
      // Get this security (by ID)
      MyMoneySecurity security = MyMoneyFile::instance()->security(_kmmID.toUtf8());

      // Set the quote source to blank
      security.setValue("kmm-online-source", QString());
      security.setValue("kmm-online-quote-system", QString());

      // Re-commit the security
      MyMoneyFile::instance()->modifySecurity(security);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::error(this, QString("<qt>") + i18n("Cannot update security <b>%1</b>: %2", _webID, e.what()) + QString("</qt>"), i18n("Price Update Failed"));
    }
  }

  // As long as the user doesn't want to cancel, move on!
  if (result != KMessageBox::Cancel) {
    QTreeWidgetItem* next = 0;
    prgOnlineProgress->setValue(prgOnlineProgress->value() + 1);
    item->setSelected(false);

    // launch the NEXT one ... in case of m_fUpdateAll == false, we
    // need to parse the list to find the next selected one
    next = lvEquityList->invisibleRootItem()->child(lvEquityList->invisibleRootItem()->indexOfChild(item) + 1);
    if (!m_fUpdateAll) {
      while (next && !next->isSelected()) {
        prgOnlineProgress->setValue(prgOnlineProgress->value() + 1);
        next = lvEquityList->invisibleRootItem()->child(lvEquityList->invisibleRootItem()->indexOfChild(next) + 1);
      }
    }
    if (next) {
      m_webQuote.launch(next->text(WEBID_COL), next->text(KMMID_COL), next->text(SOURCE_COL));
    } else {
      finishUpdate();
    }
  } else {
    finishUpdate();
  }
}

void KEquityPriceUpdateDlg::slotReceivedCSVQuote(const QString& _kmmID, const QString& _webID, MyMoneyStatement& st)
{
  QList<QTreeWidgetItem*> foundItems = lvEquityList->findItems(_kmmID, Qt::MatchExactly, KMMID_COL);
  QTreeWidgetItem* item = 0;

  if (! foundItems.empty())
    item = foundItems.at(0);

  QTreeWidgetItem* next = 0;

  if (item) {
    MyMoneyFile *file = MyMoneyFile::instance();
    MyMoneySecurity fromCurrency, toCurrency;

    if (!_kmmID.contains(QLatin1Char(' '))) {
      try {
        toCurrency = MyMoneyFile::instance()->security(_kmmID);
        fromCurrency = MyMoneyFile::instance()->security(toCurrency.tradingCurrency());
      } catch (const MyMoneyException &) {
        fromCurrency = toCurrency = MyMoneySecurity();
      }

    } else {
      QRegExp splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)", Qt::CaseInsensitive);
      if (splitrx.indexIn(_kmmID) != -1) {
        try {
          fromCurrency = MyMoneyFile::instance()->security(splitrx.cap(2).toUtf8());
          toCurrency = MyMoneyFile::instance()->security(splitrx.cap(1).toUtf8());
        } catch (const MyMoneyException &) {
          fromCurrency = toCurrency = MyMoneySecurity();
        }
      }
    }

    if (m_updatingPricePolicy != eUpdateAllPrices) {
      QStringList qSources = WebPriceQuote::quoteSources();
      for (auto it = st.m_listPrices.begin(); it != st.m_listPrices.end();) {
        MyMoneyPrice storedPrice = file->price(toCurrency.id(), fromCurrency.id(), (*it).m_date, true);
        bool priceValid = storedPrice.isValid();
        if (!priceValid)
          ++it;
        else {
          switch(m_updatingPricePolicy) {
            case eUpdateMissingPrices:
              it = st.m_listPrices.erase(it);
              break;
            case eUpdateDownloadedPrices:
              if (!qSources.contains(storedPrice.source()))
                it = st.m_listPrices.erase(it);
              else
                ++it;
              break;
            case eUpdateSameSourcePrices:
              if (storedPrice.source().compare((*it).m_sourceName) != 0)
                it = st.m_listPrices.erase(it);
              else
                ++it;
              break;
            case eAsk:
            {
              int result = KMessageBox::questionYesNoCancel(this,
                                                            i18n("For <b>%1</b> on <b>%2</b> price <b>%3</b> already exists.<br>"
                                                                 "Do you want to replace it with <b>%4</b>?",
                                                                 storedPrice.from(), storedPrice.date().toString(Qt::ISODate),
                                                                 QString().setNum(storedPrice.rate(storedPrice.to()).toDouble(), 'g', 10),
                                                                 QString().setNum((*it).m_amount.toDouble(), 'g', 10)),
                                                            i18n("Price Already Exists"));
              switch(result) {
                case KStandardGuiItem::Yes:
                  ++it;
                  break;
                case KStandardGuiItem::No:
                  it = st.m_listPrices.erase(it);
                  break;
                default:
                case KStandardGuiItem::Cancel:
                  finishUpdate();
                  return;
                  break;
              }
              break;
            }
            default:
              ++it;
              break;
          }
        }
      }
    }

    if (!st.m_listPrices.isEmpty()) {
      kmymoney->slotStatementImport(st, true);

      MyMoneyStatement::Price priceClass;
      if (st.m_listPrices.first().m_date > st.m_listPrices.last().m_date)
        priceClass = st.m_listPrices.first();
      else
        priceClass = st.m_listPrices.last();

      QDate latestDate = QDate::fromString(item->text(DATE_COL),Qt::ISODate);
      if (latestDate <= priceClass.m_date && priceClass.m_amount.isPositive()) {
        item->setText(PRICE_COL, priceClass.m_amount.formatMoney(fromCurrency.tradingSymbol(), toCurrency.pricePrecision()));
        item->setText(DATE_COL, priceClass.m_date.toString(Qt::ISODate));
        item->setText(SOURCE_COL, priceClass.m_sourceName);
      }
      logStatusMessage(i18n("Price for %1 updated (id %2)", _webID, _kmmID));
      // make sure to make OK button available
    }
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

    prgOnlineProgress->setValue(prgOnlineProgress->value() + 1);
    item->setSelected(false);

    // launch the NEXT one ... in case of m_fUpdateAll == false, we
    // need to parse the list to find the next selected one
    next = lvEquityList->invisibleRootItem()->child(lvEquityList->invisibleRootItem()->indexOfChild(item) + 1);
    if (!m_fUpdateAll) {
      while (next && !next->isSelected()) {
        prgOnlineProgress->setValue(prgOnlineProgress->value() + 1);
        next = lvEquityList->invisibleRootItem()->child(lvEquityList->invisibleRootItem()->indexOfChild(next) + 1);
      }
    }
  } else {
    logErrorMessage(i18n("Received a price for %1 (id %2), but this symbol is not on the list. Aborting entire update.", _webID, _kmmID));
  }

  if (next) {
    m_webQuote.launch(next->text(WEBID_COL), next->text(KMMID_COL), next->text(SOURCE_COL));
  } else {
    finishUpdate();
  }
}

void KEquityPriceUpdateDlg::slotReceivedQuote(const QString& _kmmID, const QString& _webID, const QDate& _date, const double& _price)
{
  QList<QTreeWidgetItem*> foundItems = lvEquityList->findItems(_kmmID, Qt::MatchExactly, KMMID_COL);
  QTreeWidgetItem* item = 0;

  if (! foundItems.empty())
    item = foundItems.at(0);

  QTreeWidgetItem* next = 0;

  if (item) {
    if (_price > 0.0f && _date.isValid()) {
      QDate date = _date;
      if (date > QDate::currentDate())
        date = QDate::currentDate();

      MyMoneyMoney price = MyMoneyMoney::ONE;
      QString id = _kmmID.toUtf8();
      MyMoneySecurity fromCurrency, toCurrency;
      if (_kmmID.contains(" ") == 0) {
        MyMoneySecurity security = MyMoneyFile::instance()->security(id);
        QString factor = security.value("kmm-online-factor");
        if (!factor.isEmpty()) {
          price = price * MyMoneyMoney(factor);
        }
        try {
          toCurrency = MyMoneyFile::instance()->security(id);
          fromCurrency = MyMoneyFile::instance()->security(toCurrency.tradingCurrency());
        } catch (const MyMoneyException &) {
          fromCurrency = toCurrency = MyMoneySecurity();
        }

      } else {
        QRegExp splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)", Qt::CaseInsensitive);
        if (splitrx.indexIn(_kmmID) != -1) {
          try {
            fromCurrency = MyMoneyFile::instance()->security(splitrx.cap(2).toUtf8());
            toCurrency = MyMoneyFile::instance()->security(splitrx.cap(1).toUtf8());
          } catch (const MyMoneyException &) {
            fromCurrency = toCurrency = MyMoneySecurity();
          }
        }
      }
      price *= MyMoneyMoney(_price, MyMoneyMoney::precToDenom(toCurrency.pricePrecision()));

      item->setText(PRICE_COL, price.formatMoney(fromCurrency.tradingSymbol(), toCurrency.pricePrecision()));
      item->setText(DATE_COL, date.toString(Qt::ISODate));
      logStatusMessage(i18n("Price for %1 updated (id %2)", _webID, _kmmID));
      // make sure to make OK button available
      buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    } else {
      logErrorMessage(i18n("Received an invalid price for %1, unable to update.", _webID));
    }

    prgOnlineProgress->setValue(prgOnlineProgress->value() + 1);
    item->setSelected(false);

    // launch the NEXT one ... in case of m_fUpdateAll == false, we
    // need to parse the list to find the next selected one
    next = lvEquityList->invisibleRootItem()->child(lvEquityList->invisibleRootItem()->indexOfChild(item) + 1);
    if (!m_fUpdateAll) {
      while (next && !next->isSelected()) {
        prgOnlineProgress->setValue(prgOnlineProgress->value() + 1);
        next = lvEquityList->invisibleRootItem()->child(lvEquityList->invisibleRootItem()->indexOfChild(next) + 1);
      }
    }
  } else {
    logErrorMessage(i18n("Received a price for %1 (id %2), but this symbol is not on the list. Aborting entire update.", _webID, _kmmID));
  }

  if (next) {
    m_webQuote.launch(next->text(WEBID_COL), next->text(KMMID_COL), next->text(SOURCE_COL));
  } else {
    finishUpdate();
  }
}

void KEquityPriceUpdateDlg::finishUpdate()
{
  // we've run past the end, reset to the default value.
  m_fUpdateAll = false;
  // force progress bar to show 100%
  prgOnlineProgress->setValue(prgOnlineProgress->maximum());
  // re-enable the sorting that was disabled during the update process
  lvEquityList->setSortingEnabled(true);
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef WEBID_COL
#undef NAME_COL
#undef PRICE_COL
#undef DATE_COL
#undef KMMID_COL
#undef SOURCE_COL
