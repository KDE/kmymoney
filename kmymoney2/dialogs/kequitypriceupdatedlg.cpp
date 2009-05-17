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

#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qfile.h>
#include <q3textstream.h>
#include <qtimer.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <ktextedit.h>
#include <k3listview.h>
#include <kdebug.h>
#include <kprogressdialog.h>
#include <kglobal.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kequitypriceupdatedlg.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyprice.h"
#include "../kmymoneyglobalsettings.h"

#define SYMBOL_COL      0
#define NAME_COL        1
#define PRICE_COL       2
#define DATE_COL        3
#define ID_COL          4
#define SOURCE_COL      5

KEquityPriceUpdateDlg::KEquityPriceUpdateDlg(QWidget *parent, const QString& securityId) :
  KEquityPriceUpdateDlgDecl(parent),
  m_fUpdateAll(false)
{
  lvEquityList->setRootIsDecorated(false);
  lvEquityList->setColumnText(0, i18n("Symbol"));
  lvEquityList->addColumn(i18n("Symbol"));
  lvEquityList->addColumn(i18n("Name"),125);
  lvEquityList->addColumn(i18n("Price"));
  lvEquityList->addColumn(i18n("Date"));

  // This is a "get it up and running" hack.  Will replace this in the future.
  lvEquityList->addColumn("ID");
  lvEquityList->addColumn("Source");
  lvEquityList->setColumnWidth(ID_COL, 0);

  lvEquityList->setMultiSelection(true);
  lvEquityList->setColumnWidthMode(SYMBOL_COL, Q3ListView::Maximum);
  lvEquityList->setColumnWidthMode(ID_COL, Q3ListView::Manual);
  lvEquityList->setAllColumnsShowFocus(true);

  btnUpdateAll->setEnabled(false);

  MyMoneyFile* file = MyMoneyFile::instance();

  //
  // Add each price pair that we know about
  //

  // send in securityId == "XXX YYY" to get a single-shot update for XXX to YYY.
  // for consistency reasons, this accepts the same delimiters as WebPriceQuote::launch()
  QRegExp splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)",false /*case sensitive*/);
  MyMoneySecurityPair currencyIds;
  if ( splitrx.search(securityId) != -1 )
    currencyIds = MyMoneySecurityPair(splitrx.cap(1).utf8(),splitrx.cap(2).utf8());

  MyMoneyPriceList prices = file->priceList();
  for(MyMoneyPriceList::ConstIterator it_price = prices.begin(); it_price != prices.end(); ++it_price)
  {
    const MyMoneySecurityPair& pair = it_price.key();
    if ( file->security( pair.first ).isCurrency() && ( securityId.isEmpty() || ( pair == currencyIds ) ) )
    {
      const MyMoneyPriceEntries& entries = (*it_price);
      if(entries.count() > 0 && entries.begin().key() <= QDate::currentDate()) {
        addPricePair(pair);
        btnUpdateAll->setEnabled(true);
      }
    }
  }

  //
  // Add each investment
  //

  Q3ValueList<MyMoneySecurity> securities = file->securityList();
  for(Q3ValueList<MyMoneySecurity>::ConstIterator it = securities.begin(); it != securities.end(); ++it)
  {
    if (  !(*it).isCurrency()
          && ( securityId.isEmpty() || ( (*it).id() == securityId ) )
          && !(*it).value("kmm-online-source").isEmpty()
       )
    {
      addInvestment(*it);
      btnUpdateAll->setEnabled(true);
    }
  }

  // if list is empty, add the request price pair
  if(lvEquityList->firstChild() == 0) {
    addPricePair(currencyIds, true);
  }

  connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(btnUpdateSelected, SIGNAL(clicked()), this, SLOT(slotUpdateSelectedClicked()));
  connect(btnUpdateAll, SIGNAL(clicked()), this, SLOT(slotUpdateAllClicked()));

  connect(&m_webQuote,SIGNAL(quote(const QString&, const QString&,const QDate&, const double&)),
    this,SLOT(slotReceivedQuote(const QString&, const QString&,const QDate&, const double&)));
  connect(&m_webQuote,SIGNAL(failed(const QString&, const QString&)),
    this,SLOT(slotQuoteFailed(const QString&, const QString&)));
  connect(&m_webQuote,SIGNAL(status(const QString&)),
    this,SLOT(logStatusMessage(const QString&)));
  connect(&m_webQuote,SIGNAL(error(const QString&)),
    this,SLOT(logErrorMessage(const QString&)));

  connect(lvEquityList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateSelection()));

  // Not implemented yet.
  btnConfigure->hide();
  //connect(btnConfigure, SIGNAL(clicked()), this, SLOT(slotConfigureClicked()));

  if ( !securityId.isEmpty() )
  {
    btnUpdateSelected->hide();
    btnUpdateAll->hide();
    // delete layout1;

    QTimer::singleShot(100,this,SLOT(slotUpdateAllClicked()));
  }

  // Hide OK button until we have received the first update
  btnOK->setEnabled(false);

  slotUpdateSelection();

  // previous versions of this dialog allowed to store a "Don't ask again" switch.
  // Since we don't support it anymore, we just get rid of it
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Notification Messages");
  grp.deleteEntry("KEquityPriceUpdateDlg::slotQuoteFailed::Price Update Failed");
}

KEquityPriceUpdateDlg::~KEquityPriceUpdateDlg()
{

}

void KEquityPriceUpdateDlg::addPricePair(const MyMoneySecurityPair& pair, bool dontCheckExistance)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QString symbol = QString("%1 > %2").arg(pair.first,pair.second);
  QString id = QString("%1 %2").arg(pair.first,pair.second);
  if ( ! lvEquityList->findItem(id,ID_COL,Q3ListView::ExactMatch) )
  {
    MyMoneyPrice pr = file->price(pair.first,pair.second);
    if(pr.source() != "KMyMoney") {
      bool keep = true;
      if((pair.first == file->baseCurrency().id())
      || (pair.second == file->baseCurrency().id())) {
        const QString& foreignCurrency = file->foreignCurrency(pair.first, pair.second);
        // check that the foreign currency is still in use
        Q3ValueList<MyMoneyAccount>::const_iterator it_a;
        Q3ValueList<MyMoneyAccount> list;
        file->accountList(list);
        for(it_a = list.begin(); !dontCheckExistance && it_a != list.end(); ++it_a) {
          // if it's an account denominated in the foreign currency
          // keep it
          if(((*it_a).currencyId() == foreignCurrency)
          && !(*it_a).isClosed())
            break;
          // if it's an investment traded in the foreign currency
          // keep it
          if((*it_a).isInvest() && !(*it_a).isClosed()) {
            MyMoneySecurity sec = file->security((*it_a).currencyId());
            if(sec.tradingCurrency() == foreignCurrency)
              break;
          }
        }
        // if it is in use, it_a is not equal to list.end()
        if(it_a == list.end() && !dontCheckExistance)
          keep = false;
      }

      if(keep) {
        K3ListViewItem* item = new K3ListViewItem(lvEquityList,
          symbol,
          i18n("%1 units in %2").arg(pair.first,pair.second));
        if(pr.isValid()) {
          item->setText(PRICE_COL, pr.rate(pair.second).formatMoney(file->currency(pair.second).tradingSymbol(), KMyMoneyGlobalSettings::pricePrecision()));
          item->setText(DATE_COL, pr.date().toString(Qt::ISODate));
        }
        item->setText(ID_COL,id);
        item->setText(SOURCE_COL, "Yahoo Currency");  // This string value should not be localized
      }
    }
  }
}

void KEquityPriceUpdateDlg::addInvestment(const MyMoneySecurity& inv)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QString symbol = inv.tradingSymbol();
  QString id = inv.id();
  if ( ! lvEquityList->findItem(id, ID_COL, Q3ListView::ExactMatch) )
  {
    // check that the security is still in use
    Q3ValueList<MyMoneyAccount>::const_iterator it_a;
    Q3ValueList<MyMoneyAccount> list;
    file->accountList(list);
    for(it_a = list.begin(); it_a != list.end(); ++it_a) {
      if((*it_a).isInvest()
      && ((*it_a).currencyId() == inv.id())
      && !(*it_a).isClosed())
        break;
    }
    // if it is in use, it_a is not equal to list.end()
    if(it_a != list.end()) {
      K3ListViewItem* item = new K3ListViewItem(lvEquityList, symbol, inv.name());
      MyMoneySecurity currency = file->currency(inv.tradingCurrency());
      MyMoneyPrice pr = file->price(id.utf8(), inv.tradingCurrency());
      if(pr.isValid()) {
        item->setText(PRICE_COL, pr.rate(currency.id()).formatMoney(currency.tradingSymbol(), KMyMoneyGlobalSettings::pricePrecision()));
        item->setText(DATE_COL, pr.date().toString(Qt::ISODate));
      }
      item->setText(ID_COL,id);
      if (inv.value("kmm-online-quote-system") == "Finance::Quote")
        item->setText(SOURCE_COL, QString("Finance::Quote %1").arg( inv.value("kmm-online-source")));
      else
        item->setText(SOURCE_COL, inv.value("kmm-online-source"));

      // If this investment is denominated in a foreign currency, ensure that
      // the appropriate price pair is also on the list

      if ( currency.id() != file->baseCurrency().id() )
      {
        addPricePair(MyMoneySecurityPair(currency.id(),file->baseCurrency().id()));
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
  Q3ListViewItem* item;

  if((item = lvEquityList->findItem(id, ID_COL, Q3ListView::ExactMatch)) != 0) {
    MyMoneyMoney rate(item->text(PRICE_COL));
    if ( !rate.isZero() )
    {
      QString id = item->text(ID_COL).utf8();

        // if the ID has a space, then this is TWO ID's, so it's a currency quote
      if ( QString(id).contains(" ") )
      {
        QStringList ids = QStringList::split(" ",QString(id));
        QString fromid = ids[0].utf8();
        QString toid = ids[1].utf8();
        price = MyMoneyPrice(fromid,toid,QDate().fromString(item->text(DATE_COL), Qt::ISODate),rate,item->text(SOURCE_COL));
      }
      else
      // otherwise, it's a security quote
      {
        MyMoneySecurity security = MyMoneyFile::instance()->security(id);
        price = MyMoneyPrice(id, security.tradingCurrency(), QDate().fromString(item->text(DATE_COL), Qt::ISODate), rate, item->text(SOURCE_COL));
      }
    }
  }
  return price;
}

void KEquityPriceUpdateDlg::storePrices(void)
{
  // update the new prices into the equities

  MyMoneyFile* file = MyMoneyFile::instance();
  Q3ValueList<MyMoneySecurity> equities = file->securityList();

  Q3ListViewItem* item = lvEquityList->firstChild();
  MyMoneyFileTransaction ft;
  QString name;
  try {
    while ( item )
    {
      // turn on signals before we modify the last entry in the list
      MyMoneyFile::instance()->blockSignals(item->nextSibling() != 0);

      MyMoneyMoney rate(item->text(PRICE_COL));
      if ( !rate.isZero() )
      {
        QString id = item->text(ID_COL).utf8();

        // if the ID has a space, then this is TWO ID's, so it's a currency quote
        if ( QString(id).contains(" ") )
        {
          QStringList ids = QStringList::split(" ",QString(id));
          QString fromid = ids[0].utf8();
          QString toid = ids[1].utf8();
          name = QString("%1 --> %2").arg(fromid).arg(toid);
          MyMoneyPrice price(fromid,toid,QDate().fromString(item->text(DATE_COL), Qt::ISODate),rate,item->text(SOURCE_COL));
          file->addPrice(price);
        }
        else
        // otherwise, it's a security quote
        {
          MyMoneySecurity security = MyMoneyFile::instance()->security(id);
          name = security.name();
          MyMoneyPrice price(id, security.tradingCurrency(), QDate().fromString(item->text(DATE_COL), Qt::ISODate), rate, item->text(SOURCE_COL));

          // TODO (Ace) Better handling of the case where there is already a price
          // for this date.  Currently, it just overrides the old value.  Really it
          // should check to see if the price is the same and prompt the user.
          MyMoneyFile::instance()->addPrice(price);
        }

      }
      item = item->nextSibling();
    }
    ft.commit();

  } catch(MyMoneyException *e) {
    qDebug("Unable to add price information for %s", name.data());
    delete e;
  }
}

void KEquityPriceUpdateDlg::slotUpdateSelection(void)
{
  btnUpdateSelected->setEnabled(false);

  Q3ListViewItem* item = lvEquityList->firstChild();
  while ( item && !item->isSelected())
    item = item->nextSibling();

  if(item)
    btnUpdateSelected->setEnabled(true);
}

void KEquityPriceUpdateDlg::slotUpdateSelectedClicked(void)
{
  Q3ListViewItem* item = lvEquityList->firstChild();
  int skipCnt = 1;
  while ( item && !item->isSelected())
  {
    skipCnt++;
    item = item->nextSibling();
  }

  if(item) {
    prgOnlineProgress->setMaximum(1+lvEquityList->childCount());
    prgOnlineProgress->setValue(skipCnt);
    m_webQuote.launch(item->text(SYMBOL_COL),item->text(ID_COL),item->text(SOURCE_COL));
  }
  else
    logErrorMessage("No security selected.");
}

void KEquityPriceUpdateDlg::slotUpdateAllClicked(void)
{
  Q3ListViewItem* item = lvEquityList->firstChild();
  if ( item )
  {
    prgOnlineProgress->setMaximum(1+lvEquityList->childCount());
    prgOnlineProgress->setValue(1);
    m_fUpdateAll = true;
    m_webQuote.launch(item->text(SYMBOL_COL),item->text(ID_COL),item->text(SOURCE_COL));
  }
  else
    logErrorMessage("Security list is empty.");
}

void KEquityPriceUpdateDlg::slotQuoteFailed(const QString& _id, const QString& _symbol)
{
  Q3ListViewItem* item = lvEquityList->findItem(_id,ID_COL,Q3ListView::ExactMatch);

  // Give the user some options
  int result;
  if(_id.contains(" ")) {
    result = KMessageBox::warningContinueCancel(this, i18n("Failed to retrieve an exchange rate for %1 from %2. It will be skipped this time.",_symbol, item->text(SOURCE_COL)), i18n("Price Update  Failed"));
  } else {
    result = KMessageBox::questionYesNoCancel(this, QString("<qt>%1</qt>").arg(i18n("Failed to retrieve a quote for %1 from %2.  Press <b>No</b> to remove the online price source from this security permanently, <b>Yes</b> to continue updating this security during future price updates or <b>Cancel</b> to stop the current update operation.",_symbol, item->text(SOURCE_COL))), i18n("Price Update Failed"), KStandardGuiItem::yes(), KStandardGuiItem::no());
  }

  if ( result == KMessageBox::No )
  {
    // Disable price updates for this security

    MyMoneyFileTransaction ft;
    try {
      // Get this security (by ID)
      MyMoneySecurity security = MyMoneyFile::instance()->security(_id.utf8());

      // Set the quote source to blank
      security.setValue("kmm-online-source",QString());
      security.setValue("kmm-online-quote-system",QString());

      // Re-commit the security
      MyMoneyFile::instance()->modifySecurity(security);
      ft.commit();
    } catch(MyMoneyException* e) {
      KMessageBox::error(this, QString("<qt>")+i18n("Cannot update security <b>%1</b>: %2",_symbol, e->what())+QString("</qt>"), i18n("Price Update Failed"));
      delete e;
    }
  }

  // As long as the user doesn't want to cancel, move on!
  if ( result != KMessageBox::Cancel )
  {
    Q3ListViewItem* next = NULL;
    prgOnlineProgress->setValue(prgOnlineProgress->value()+1);
    item->listView()->setSelected(item, false);

    // launch the NEXT one ... in case of m_fUpdateAll == false, we
    // need to parse the list to find the next selected one
    next = item->nextSibling();
    if ( !m_fUpdateAll )
    {
      while(next && !next->isSelected())
      {
        prgOnlineProgress->setValue(prgOnlineProgress->value()+1);
        next = next->nextSibling();
      }
    }
    if (next)
    {
      m_webQuote.launch(next->text(SYMBOL_COL),next->text(ID_COL),next->text(SOURCE_COL));
    }
    else
    {
      finishUpdate();
    }
  }
  else
  {
    finishUpdate();
  }
}

void KEquityPriceUpdateDlg::slotReceivedQuote(const QString& _id, const QString& _symbol,const QDate& _date, const double& _price)
{
  Q3ListViewItem* item = lvEquityList->findItem(_id,ID_COL,Q3ListView::ExactMatch);
  Q3ListViewItem* next = NULL;

  if ( item )
  {
    if ( _price > 0.0f && _date.isValid() )
    {
      QDate date = _date;
      if ( date > QDate::currentDate() )
        date = QDate::currentDate();

      double price = _price;
      QString id = _id.utf8();
      MyMoneySecurity sec;
      if ( _id.contains(" ") == 0) {
        MyMoneySecurity security = MyMoneyFile::instance()->security(id);
        QString factor = security.value("kmm-online-factor");
        if(!factor.isEmpty()) {
          price *= MyMoneyMoney(factor).toDouble();
        }
        try {
          sec = MyMoneyFile::instance()->security(id);
          sec = MyMoneyFile::instance()->security(sec.tradingCurrency());
        } catch(MyMoneyException *e) {
          sec = MyMoneySecurity();
          delete e;
        }

      } else {
        QRegExp splitrx("([0-9a-z\\.]+)[^a-z0-9]+([0-9a-z\\.]+)",false /*case sensitive*/);
        if ( splitrx.search(_id) != -1 ) {
          try {
            sec = MyMoneyFile::instance()->security(splitrx.cap(2).utf8());
          } catch(MyMoneyException *e) {
            sec = MyMoneySecurity();
            delete e;
          }
        }
      }
      item->setText(PRICE_COL, KGlobal::locale()->formatMoney(price, sec.tradingSymbol(), KMyMoneyGlobalSettings::pricePrecision()));
      item->setText(DATE_COL, date.toString(Qt::ISODate));
      logStatusMessage(i18n("Price for %1 updated (id %2)",_symbol,_id));
      // make sure to make OK button available
      btnOK->setEnabled(true);
    }
    else
    {
      logErrorMessage(i18n("Received an invalid price for %1, unable to update.",_symbol));
    }

    prgOnlineProgress->setValue(prgOnlineProgress->value()+1);
    item->listView()->setSelected(item, false);

    // launch the NEXT one ... in case of m_fUpdateAll == false, we
    // need to parse the list to find the next selected one
    next = item->nextSibling();
    if ( !m_fUpdateAll )
    {
      while(next && !next->isSelected())
      {
        prgOnlineProgress->setValue(prgOnlineProgress->value()+1);
        next = next->nextSibling();
      }
    }
  }
  else
  {
    logErrorMessage(i18n("Received a price for %1 (id %2), but this symbol is not on the list!  Aborting entire update.",_symbol,_id));
  }

  if (next)
  {
    m_webQuote.launch(next->text(SYMBOL_COL),next->text(ID_COL),next->text(SOURCE_COL));
  }
  else
  {
    finishUpdate();
  }
}

void KEquityPriceUpdateDlg::finishUpdate(void)
{
  // we've run past the end, reset to the default value.
  m_fUpdateAll = false;
  // force progress bar to show 100%
  prgOnlineProgress->setValue(prgOnlineProgress->maximum());
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef SYMBOL_COL
#undef NAME_COL
#undef PRICE_COL
#undef DATE_COL
#undef ID_COL
#undef SOURCE_COL

#include "kequitypriceupdatedlg.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
