/***************************************************************************
                          kforecastview.cpp
                             -------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
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

#include <qtabwidget.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <q3buttongroup.h>
#include <q3textedit.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>
#include <klocale.h>
#include <k3listview.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kforecastview.h"
#include "../kmymoneyglobalsettings.h"
#include "../kmymoney2.h"
#include "../kmymoneyutils.h"
#include "../mymoney/mymoneyforecast.h"
#include "../widgets/kmymoneyforecastlistviewitem.h"
#include "../widgets/kmymoneyaccounttreeforecast.h"
#include "../reports/pivottable.h"
#include "../reports/pivotgrid.h"

KForecastView::KForecastView(QWidget *parent, const char *name) :
  KForecastViewDecl(parent,name)
{
  for(int i=0; i < MaxViewTabs; ++i)
    m_needReload[i] = false;

  KSharedConfigPtr config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_tab->setCurrentPage(config->readNumEntry("KForecastView_LastType", 0));

  connect(m_tab, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotTabChanged(QWidget*)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadForecast()));

  connect(m_forecastButton, SIGNAL(clicked()), this, SLOT(slotManualForecast()));

  m_forecastList->setAllColumnsShowFocus(true);
  m_summaryList->setAllColumnsShowFocus(true);
  //m_adviceList->setAllColumnsShowFocus(true);
  m_advancedList->setAllColumnsShowFocus(true);

  m_forecastChart = new KReportChartView(m_tabChart, "forecastChart" );
  m_forecastChart->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  loadForecastSettings();

}

KForecastView::~KForecastView()
{
}

void KForecastView::slotTabChanged(QWidget* _tab)
{
  ForecastViewTab tab = static_cast<ForecastViewTab>(m_tab->indexOf(_tab));

  // remember this setting for startup
  KSharedConfigPtr config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KForecastView_LastType", tab);

  loadForecast(tab);

}

void KForecastView::loadForecast(ForecastViewTab tab)
{
  if(m_needReload[tab]) {
    switch(tab) {
      case ListView:
        loadListView();
        break;
      case SummaryView:
        loadSummaryView();
        break;
      case AdvancedView:
        loadAdvancedView();
        break;
      case BudgetView:
        loadBudgetView();
        break;
      case ChartView:
        loadChartView();
        break;
      default:
        break;
    }
    m_needReload[tab] = false;
  }
}

void KForecastView::show(void)
{
  // don't forget base class implementation
  KForecastViewDecl::show();
  slotTabChanged(m_tab->currentPage());
}

void KForecastView::slotLoadForecast(void)
{
  m_needReload[SummaryView] = true;
  m_needReload[ListView] = true;
  m_needReload[AdvancedView] = true;
  m_needReload[BudgetView] = true;
  m_needReload[ChartView] = true;

  //refresh settings
  loadForecastSettings();

  if(isVisible())
    slotTabChanged(m_tab->currentPage());
}

void KForecastView::slotManualForecast(void)
{
  m_needReload[SummaryView] = true;
  m_needReload[ListView] = true;
  m_needReload[AdvancedView] = true;
  m_needReload[BudgetView] = true;
  m_needReload[ChartView] = true;

  if(isVisible())
    slotTabChanged(m_tab->currentPage());
}

void KForecastView::loadForecastSettings(void)
{
  //fill the settings controls
  m_forecastDays->setValue(KMyMoneyGlobalSettings::forecastDays());
  m_accountsCycle->setValue(KMyMoneyGlobalSettings::forecastAccountCycle());
  m_beginDay->setValue(KMyMoneyGlobalSettings::beginForecastDay());
  m_forecastCycles->setValue(KMyMoneyGlobalSettings::forecastCycles());
  m_historyMethod->setButton(KMyMoneyGlobalSettings::historyMethod());
  switch(KMyMoneyGlobalSettings::forecastMethod()) {
    case 0:
      m_forecastMethod->setText(i18n("Scheduled"));
      m_forecastCycles->setDisabled(true);
      m_historyMethod->setDisabled(true);
      break;
    case 1:
      m_forecastMethod->setText(i18n("History"));
      m_forecastCycles->setEnabled(true);
      m_historyMethod->setEnabled(true);
      break;
    default:
      m_forecastMethod->setText(i18n("Unknown"));
      break;
  }
}

void KForecastView::loadListView(void)
{
  MyMoneyForecast forecast;
  MyMoneyFile* file = MyMoneyFile::instance();

  m_forecastList->setBaseCurrency(file->baseCurrency());

  //get the settings from current page
  forecast.setForecastDays(m_forecastDays->value());
  forecast.setAccountsCycle(m_accountsCycle->value());
  forecast.setBeginForecastDay(m_beginDay->value());
  forecast.setForecastCycles(m_forecastCycles->value());
  forecast.setHistoryMethod(m_historyMethod->selectedId());
  forecast.doForecast();

  //clear the list, including columns
  m_forecastList->clearColumns();

  //add columns
  m_forecastList->showAccount();
  m_forecastList->showDetailed(forecast);

  //add default rows
  addTotalRow(m_forecastList, forecast);
  addAssetLiabilityRows(forecast);

  //load asset and liability forecast accounts
  loadAccounts(forecast, file->asset(), m_assetItem, KMyMoneyAccountTreeForecastItem::eDetailed);
  loadAccounts(forecast, file->liability(), m_liabilityItem, KMyMoneyAccountTreeForecastItem::eDetailed);

  m_forecastList->show();
}

void KForecastView::loadSummaryView(void)
{
  MyMoneyForecast forecast;
  Q3ValueList<MyMoneyAccount> accList;
  int dropMinimum;
  int dropZero;

  MyMoneyFile* file = MyMoneyFile::instance();

  m_summaryList->setBaseCurrency(file->baseCurrency());

  //get the settings from current page
  forecast.setForecastDays(m_forecastDays->value());
  forecast.setAccountsCycle(m_accountsCycle->value());
  forecast.setBeginForecastDay(m_beginDay->value());
  forecast.setForecastCycles(m_forecastCycles->value());
  forecast.setHistoryMethod(m_historyMethod->selectedId());
  forecast.doForecast();

  //clear the list, including columns
  m_summaryList->clearColumns();

  //add columns
  m_summaryList->showAccount();
  m_summaryList->showSummary(forecast);

  //add default rows
  addTotalRow(m_summaryList, forecast);
  addAssetLiabilityRows(forecast);

  loadAccounts(forecast, file->asset(), m_assetItem, KMyMoneyAccountTreeForecastItem::eSummary);
  loadAccounts(forecast, file->liability(), m_liabilityItem, KMyMoneyAccountTreeForecastItem::eSummary);

  //Add comments to the advice list
  //Get all accounts of the right type to calculate forecast
  m_nameIdx.clear();
  accList = forecast.accountList();
  Q3ValueList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ++accList_t ) {
    MyMoneyAccount acc = *accList_t;
    if(m_nameIdx[acc.id()] != acc.id()) { //Check if the account is there
      m_nameIdx[acc.id()] = acc.id();
    }
  }

  QMap<QString, QString>::ConstIterator it_nc;
  for(it_nc = m_nameIdx.begin(); it_nc != m_nameIdx.end(); ++it_nc) {

    const MyMoneyAccount& acc = file->account(*it_nc);
    MyMoneySecurity currency;

    //change currency to deep currency if account is an investment
    if(acc.isInvest()) {
      MyMoneySecurity underSecurity = file->security(acc.currencyId());
      currency = file->security(underSecurity.tradingCurrency());
    } else {
      currency = file->security(acc.currencyId());
    }

    //Check if the account is going to be below zero or below the minimal balance in the forecast period
    QString minimumBalance = acc.value("minimumBalance");
    MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);

    //Check if the account is going to be below minimal balance
    dropMinimum = forecast.daysToMinimumBalance(acc);

    //Check if the account is going to be below zero in the future
    dropZero = forecast.daysToZeroBalance(acc);

    // spit out possible warnings
    QString msg;

    // if a minimum balance has been specified, an appropriate warning will
    // only be shown, if the drop below 0 is on a different day or not present

    if(dropMinimum != -1
       && !minBalance.isZero()
       && (dropMinimum < dropZero
       || dropZero == -1)) {
      switch(dropMinimum) {
        case -1:
          break;
        case 0:
          msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::listNegativeValueColor().name());
          msg += i18n("The balance of %2 is below the minimum balance %3 today.").arg(acc.name()).arg(minBalance.formatMoney(acc, currency));
          msg += QString("</font>");
          break;
        default:
          msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::listNegativeValueColor().name());
          msg += i18n("The balance of %1 will drop below the minimum balance %2 in %3 days.").arg(acc.name()).arg(minBalance.formatMoney(acc, currency)).arg(dropMinimum-1);
          msg += QString("</font>");
      }

      if(!msg.isEmpty()) {
        m_adviceText->append(msg);
      }
    }

    // a drop below zero is always shown
    msg = QString();
    switch(dropZero) {
      case -1:
        break;
      case 0:
        if(acc.accountGroup() == MyMoneyAccount::Asset) {
          msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::listNegativeValueColor().name());
          msg += i18n("The balance of %1 is below %2 today.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency));
          msg += QString("</font>");
          break;
        }
        if(acc.accountGroup() == MyMoneyAccount::Liability) {
          msg = i18n("The balance of %1 is above %2 today.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency));
          break;
        }
        break;
      default:
        if(acc.accountGroup() == MyMoneyAccount::Asset) {
          msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::listNegativeValueColor().name());
          msg += i18n("The balance of %1 will drop below %2 in %3 days.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency)).arg(dropZero);
          msg += QString("</font>");
          break;
        }
        if(acc.accountGroup() == MyMoneyAccount::Liability) {
          msg = i18n("The balance of %1 will raise above %2 in %3 days.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency)).arg(dropZero);
          break;
        }
    }
    if(!msg.isEmpty()) {
      m_adviceText->append(msg);
    }

    //advice about trends
    msg = QString();
    MyMoneyMoney accCycleVariation = forecast.accountCycleVariation(acc);
    if (accCycleVariation < MyMoneyMoney(0, 1)) {
      msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::listNegativeValueColor().name());
      msg += i18n("The account %1 is decreasing %2 per cycle.").arg(acc.name()).arg(accCycleVariation.formatMoney(acc, currency));
      msg += QString("</font>");
    }

    if(!msg.isEmpty()) {
      m_adviceText->append(msg);
    }
  }
  m_summaryList->show();
  m_adviceText->show();
}

void KForecastView::loadAdvancedView(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  Q3ValueList<MyMoneyAccount> accList;
  MyMoneySecurity baseCurrency = file->baseCurrency();
  MyMoneyForecast forecast;
  int daysToBeginDay;

  //get the settings from current page
  forecast.setForecastDays(m_forecastDays->value());
  forecast.setAccountsCycle(m_accountsCycle->value());
  forecast.setBeginForecastDay(m_beginDay->value());
  forecast.setForecastCycles(m_forecastCycles->value());
  forecast.setHistoryMethod(m_historyMethod->selectedId());
  forecast.doForecast();

  //Get all accounts of the right type to calculate forecast
  m_nameIdx.clear();
  accList = forecast.accountList();
  Q3ValueList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ++accList_t ) {
    MyMoneyAccount acc = *accList_t;
    if(m_nameIdx[acc.id()] != acc.id()) { //Check if the account is there
      m_nameIdx[acc.id()] = acc.id();
    }
  }
  //clear the list, including columns
  m_advancedList->clear();
  for(;m_advancedList->columns() > 0;) {
    m_advancedList->removeColumn(0);
  }

  //add first column of both lists
  int accountColumn = m_advancedList->addColumn(i18n("Account"), -1);

  //if beginning of forecast is today, set the begin day to next cycle to avoid repeating the first cycle
  if(QDate::currentDate() < forecast.beginForecastDate()) {
    daysToBeginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
  } else {
    daysToBeginDay = forecast.accountsCycle();
  }

  //add columns
  for(int i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int col = m_advancedList->addColumn(i18n("Min Bal %1").arg(i), -1);
    m_advancedList->setColumnAlignment(col, Qt::AlignRight);
    m_advancedList->addColumn(i18n("Min Date %1").arg(i), -1);
  }
  for(int i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int col = m_advancedList->addColumn(i18n("Max Bal %1").arg(i), -1);
    m_advancedList->setColumnAlignment(col, Qt::AlignRight);
    m_advancedList->addColumn(i18n("Max Date %1").arg(i), -1);
  }
  int col = m_advancedList->addColumn(i18n("Average"), -1);
  m_advancedList->setColumnAlignment(col, Qt::AlignRight);
  m_advancedList->setSorting(-1);
  KMyMoneyForecastListViewItem *advancedItem = 0;

  QMap<QString, QString>::ConstIterator it_nc;
  for(it_nc = m_nameIdx.begin(); it_nc != m_nameIdx.end(); ++it_nc) {
    const MyMoneyAccount& acc = file->account(*it_nc);
    QString amount;
    MyMoneyMoney amountMM;
    MyMoneySecurity currency;

    //change currency to deep currency if account is an investment
    if(acc.isInvest()) {
      MyMoneySecurity underSecurity = file->security(acc.currencyId());
      currency = file->security(underSecurity.tradingCurrency());
    } else {
      currency = file->security(acc.currencyId());
    }


    advancedItem = new KMyMoneyForecastListViewItem(m_advancedList, advancedItem, false);
    advancedItem->setText(accountColumn, acc.name());
    int it_c = 1; // iterator for the columns of the listview

    //get minimum balance list
    Q3ValueList<QDate> minBalanceList = forecast.accountMinimumBalanceDateList(acc);
    Q3ValueList<QDate>::Iterator t_min;
    for(t_min = minBalanceList.begin(); t_min != minBalanceList.end() ; ++t_min)
    {
      QDate minDate = *t_min;
      amountMM = forecast.forecastBalance(acc, minDate);

      amount = amountMM.formatMoney(acc, currency);
      advancedItem->setText(it_c, amount, amountMM.isNegative());
      it_c++;

      QString dateString = KGlobal::locale()->formatDate(minDate, true);
      advancedItem->setText(it_c, dateString, amountMM.isNegative());
      it_c++;
    }

    //get maximum balance list
    Q3ValueList<QDate> maxBalanceList = forecast.accountMaximumBalanceDateList(acc);
    Q3ValueList<QDate>::Iterator t_max;
    for(t_max = maxBalanceList.begin(); t_max != maxBalanceList.end() ; ++t_max)
    {
      QDate maxDate = *t_max;
      amountMM = forecast.forecastBalance(acc, maxDate);

      amount = amountMM.formatMoney(acc, currency);
      advancedItem->setText(it_c, amount, amountMM.isNegative());
      it_c++;

      QString dateString = KGlobal::locale()->formatDate(maxDate, true);
      advancedItem->setText(it_c, dateString, amountMM.isNegative());
      it_c++;
    }
    //get average balance
    amountMM = forecast.accountAverageBalance(acc);
    amount = amountMM.formatMoney(acc, currency);
    advancedItem->setText(it_c, amount, amountMM.isNegative());
    it_c++;
  }
  m_advancedList->show();
}

void KForecastView::loadBudgetView(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyForecast forecast;
//  QValueList<MyMoneyAccount> accList;

  m_budgetList->setBaseCurrency(file->baseCurrency());

  //get the settings from current page and calculate this year based on last year
  QDate historyEndDate = QDate(QDate::currentDate().year()-1, 12, 31);
  QDate historyStartDate = historyEndDate.addDays(-m_accountsCycle->value() * m_forecastCycles->value());
  QDate forecastStartDate = QDate(QDate::currentDate().year(), 1, 1);
  QDate forecastEndDate = QDate::currentDate().addDays(m_forecastDays->value());
  forecast.setHistoryMethod(m_historyMethod->selectedId());

  MyMoneyBudget budget;
  forecast.createBudget(budget, historyStartDate, historyEndDate, forecastStartDate, forecastEndDate, false);

  //clear the list, including columns
  m_budgetList->clearColumns();

  //add columns
  m_budgetList->showAccount();
  m_budgetList->showBudget(forecast);

  //add default rows
  addTotalRow(m_budgetList, forecast);
  addIncomeExpenseRows(forecast);

  //load income and expense budget accounts
  loadAccounts(forecast, file->income(), m_incomeItem, KMyMoneyAccountTreeForecastItem::eBudget);
  loadAccounts(forecast, file->expense(), m_expenseItem, KMyMoneyAccountTreeForecastItem::eBudget);

  m_budgetList->show();
}

Q3ValueList<MyMoneyPrice> KForecastView::getAccountPrices(const MyMoneyAccount& acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  Q3ValueList<MyMoneyPrice> prices;
  MyMoneySecurity security = file->baseCurrency();
    try {
      if(acc.isInvest()) {
        security = file->security(acc.currencyId());
        if(security.tradingCurrency() != file->baseCurrency().id()) {
          MyMoneySecurity sec = file->security(security.tradingCurrency());
          prices += file->price(sec.id(), file->baseCurrency().id());
        }
      } else if(acc.currencyId() != file->baseCurrency().id()) {
        if(acc.currencyId() != file->baseCurrency().id()) {
          security = file->security(acc.currencyId());
          prices += file->price(acc.currencyId(), file->baseCurrency().id());
        }
      }

    } catch(MyMoneyException *e) {
      kDebug(2) << __PRETTY_FUNCTION__ << " caught exception while adding " << acc.name() << "[" << acc.id() << "]: " << e->what();
      delete e;
    }
  return prices;
}

void KForecastView::addAssetLiabilityRows(const MyMoneyForecast& forecast)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  Q3ValueList<MyMoneyPrice> basePrices;
  m_assetItem = new KMyMoneyAccountTreeForecastItem( m_totalItem, file->asset(), forecast, basePrices, file->baseCurrency() );
  m_assetItem->setOpen(true);
  m_liabilityItem = new KMyMoneyAccountTreeForecastItem( m_totalItem, file->liability(), forecast, basePrices, file->baseCurrency());
  m_liabilityItem->setOpen(true);
}

void KForecastView::addIncomeExpenseRows(const MyMoneyForecast& forecast)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  Q3ValueList<MyMoneyPrice> basePrices;
  m_incomeItem = new KMyMoneyAccountTreeForecastItem( m_totalItem, file->income(), forecast, basePrices, file->baseCurrency() );
  m_incomeItem->setOpen(true);
  m_expenseItem = new KMyMoneyAccountTreeForecastItem( m_totalItem, file->expense(), forecast, basePrices, file->baseCurrency());
  m_expenseItem->setOpen(true);
}

void KForecastView::addTotalRow(KMyMoneyAccountTreeForecast* forecastList, const MyMoneyForecast& forecast)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  m_totalItem = new KMyMoneyAccountTreeForecastItem( forecastList, file->asset(), forecast, file->baseCurrency(), i18n("Total") );
  m_totalItem->setOpen(true);
}

bool KForecastView::includeAccount(MyMoneyForecast& forecast, const MyMoneyAccount& acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  if( forecast.isForecastAccount(acc) )
    return true;

  QStringList accounts = acc.accountList();

  if(accounts.size() > 0) {
    QStringList::ConstIterator it_acc;
    for(it_acc = accounts.begin(); it_acc != accounts.end(); ++it_acc) {
      MyMoneyAccount account = file->account(*it_acc);
      if( includeAccount(forecast, account) )
        return true;
    }
  }
  return false;
}

void KForecastView::loadAccounts(MyMoneyForecast& forecast, const MyMoneyAccount& account, KMyMoneyAccountTreeForecastItem* parentItem, int forecastType )
{
  QMap<QString, QString> nameIdx;
  QStringList accList;
  MyMoneyFile* file = MyMoneyFile::instance();
  KMyMoneyAccountTreeForecastItem *forecastItem = 0;

  //Get all accounts of the right type to calculate forecast
  accList = account.accountList();

  if(accList.size() == 0)
    return;

  QStringList::ConstIterator accList_t;
  for(accList_t = accList.begin(); accList_t != accList.end(); ++accList_t ) {
    MyMoneyAccount subAccount = file->account(*accList_t);
    //only add the account if it is a forecast account or the parent of a forecast account
    if(includeAccount(forecast, subAccount)) {
      nameIdx[subAccount.id()] = subAccount.id();
    }
  }

  QMap<QString, QString>::ConstIterator it_nc;
  for(it_nc = nameIdx.begin(); it_nc != nameIdx.end(); ++it_nc) {

    const MyMoneyAccount subAccount = file->account(*it_nc);
    MyMoneySecurity currency;
    if(subAccount.isInvest()) {
      MyMoneySecurity underSecurity = file->security(subAccount.currencyId());
      currency = file->security(underSecurity.tradingCurrency());
    } else {
      currency = file->security(subAccount.currencyId());
    }

    QString amount;
    QString vAmount;
    MyMoneyMoney vAmountMM;

    //get prices
    Q3ValueList<MyMoneyPrice> prices = getAccountPrices(subAccount);

    forecastItem = new KMyMoneyAccountTreeForecastItem( parentItem, subAccount, forecast, prices, currency, static_cast<KMyMoneyAccountTreeForecastItem::EForecastViewType>(forecastType) );
    forecastItem->setOpen(true);

    loadAccounts(forecast, subAccount, forecastItem, forecastType);
  }
}

void KForecastView::loadChartView(void)
{
  MyMoneyReport::EDetailLevel detailLevel[4] = { MyMoneyReport::eDetailAll, MyMoneyReport::eDetailTop, MyMoneyReport::eDetailGroup, MyMoneyReport::eDetailTotal };

  MyMoneyReport reportCfg = MyMoneyReport(
    MyMoneyReport::eAssetLiability,
    MyMoneyReport::eMonths,
    MyMoneyTransactionFilter::userDefined, // overridden by the setDateFilter() call below
    detailLevel[m_comboDetail->currentItem()],
    i18n("Networth Forecast"),
    i18n("Generated Report"));

  reportCfg.setChartByDefault(true);
  reportCfg.setChartGridLines(false);
  reportCfg.setChartType(MyMoneyReport::eChartLine);
  reportCfg.setIncludingSchedules( false );
  reportCfg.addAccountGroup(MyMoneyAccount::Asset);
  reportCfg.addAccountGroup(MyMoneyAccount::Liability);
  reportCfg.setColumnsAreDays( true );
  reportCfg.setConvertCurrency( true );
  reportCfg.setIncludingForecast( true );
  reportCfg.setDateFilter(QDate::currentDate(),QDate::currentDate().addDays(m_forecastDays->value()));

  reports::PivotTable table(reportCfg);

  table.drawChart(*m_forecastChart);

  // Adjust the size
  int nh;
  nh = (width()*0.5);
  m_forecastChart->resize(width()-60, nh);

  m_forecastChart->update();
}

#include "kforecastview.moc"
