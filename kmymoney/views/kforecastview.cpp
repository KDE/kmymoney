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

#include "kforecastview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTabWidget>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QIcon>
#include <QTimer>
#include <QScrollBar>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KTextEdit>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneyfile.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "kmymoneyglobalsettings.h"
#include "mymoneyforecast.h"
#include "mymoneybudget.h"
#include "pivottable.h"
#include "fixedcolumntreeview.h"
#include "kreportchartview.h"
#include "reportaccount.h"
#include "icons.h"

using namespace reports;
using namespace Icons;

KForecastView::KForecastView(QWidget *parent) :
    QWidget(parent),
    m_needLoad(true),
    m_totalItem(0),
    m_assetItem(0),
    m_liabilityItem(0),
    m_incomeItem(0),
    m_expenseItem(0),
    m_chartLayout(0),
    m_forecastChart(0)
{
}

KForecastView::~KForecastView()
{
}

void KForecastView::setDefaultFocus()
{
  QTimer::singleShot(0, m_forecastButton, SLOT(setFocus()));
}

void KForecastView::init()
{
  m_needLoad = false;
  setupUi(this);

  m_forecastChart = new KReportChartView(m_tabChart);

  for (int i = 0; i < MaxViewTabs; ++i)
    m_needReload[i] = false;

  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Last Use Settings");
  m_tab->setCurrentIndex(grp.readEntry("KForecastView_LastType", 0));

  m_forecastButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewForecast]));

  connect(m_tab, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadForecast()));

  connect(m_forecastButton, SIGNAL(clicked()), this, SLOT(slotManualForecast()));

  m_forecastList->setUniformRowHeights(true);
  m_forecastList->setAllColumnsShowFocus(true);
  m_summaryList->setAllColumnsShowFocus(true);
  m_budgetList->setAllColumnsShowFocus(true);
  m_advancedList->setAlternatingRowColors(true);

  connect(m_forecastList, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));
  connect(m_forecastList, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(itemCollapsed(QTreeWidgetItem*)));
  connect(m_summaryList, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));
  connect(m_summaryList, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(itemCollapsed(QTreeWidgetItem*)));
  connect(m_budgetList, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));
  connect(m_budgetList, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(itemCollapsed(QTreeWidgetItem*)));

  m_forecastChart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_chartLayout = m_tabChart->layout();
  m_chartLayout->setSpacing(6);
  m_chartLayout->addWidget(m_forecastChart);

  loadForecastSettings();
}

void KForecastView::slotTabChanged(int index)
{
  ForecastViewTab tab = static_cast<ForecastViewTab>(index);

  // remember this setting for startup
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("KForecastView_LastType", QVariant(tab).toString());

  loadForecast(tab);

}

void KForecastView::loadForecast(ForecastViewTab tab)
{
  if (m_needReload[tab]) {
    switch (tab) {
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

void KForecastView::showEvent(QShowEvent* event)
{
  if (m_needLoad) {
    init();
    loadForecastSettings();
  }
  emit aboutToShow();

  slotTabChanged(m_tab->currentIndex());

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KForecastView::slotLoadForecast()
{
  m_needReload[SummaryView] = true;
  m_needReload[ListView] = true;
  m_needReload[AdvancedView] = true;
  m_needReload[BudgetView] = true;
  m_needReload[ChartView] = true;

  if (isVisible()) {
    //refresh settings
    loadForecastSettings();
    slotTabChanged(m_tab->currentIndex());
  }
}

void KForecastView::slotManualForecast()
{
  m_needReload[SummaryView] = true;
  m_needReload[ListView] = true;
  m_needReload[AdvancedView] = true;
  m_needReload[BudgetView] = true;
  m_needReload[ChartView] = true;

  if (isVisible())
    slotTabChanged(m_tab->currentIndex());
}

void KForecastView::loadForecastSettings()
{
  //fill the settings controls
  m_forecastDays->setValue(KMyMoneyGlobalSettings::forecastDays());
  m_accountsCycle->setValue(KMyMoneyGlobalSettings::forecastAccountCycle());
  m_beginDay->setValue(KMyMoneyGlobalSettings::beginForecastDay());
  m_forecastCycles->setValue(KMyMoneyGlobalSettings::forecastCycles());
  m_historyMethod->setId(radioButton11, 0); // simple moving avg
  m_historyMethod->setId(radioButton12, 1); // weighted moving avg
  m_historyMethod->setId(radioButton13, 2); // linear regression
  m_historyMethod->button(KMyMoneyGlobalSettings::historyMethod())->setChecked(true);
  switch (KMyMoneyGlobalSettings::forecastMethod()) {
    case 0:
      m_forecastMethod->setText(i18nc("Scheduled method", "Scheduled"));
      m_forecastCycles->setDisabled(true);
      m_historyMethodGroupBox->setDisabled(true);
      break;
    case 1:
      m_forecastMethod->setText(i18nc("History-based method", "History"));
      m_forecastCycles->setEnabled(true);
      m_historyMethodGroupBox->setEnabled(true);
      break;
    default:
      m_forecastMethod->setText(i18nc("Unknown forecast method", "Unknown"));
      break;
  }
}

void KForecastView::loadListView()
{
  MyMoneyForecast forecast = KMyMoneyGlobalSettings::forecast();
  MyMoneyFile* file = MyMoneyFile::instance();

  //get the settings from current page
  forecast.setForecastDays(m_forecastDays->value());
  forecast.setAccountsCycle(m_accountsCycle->value());
  forecast.setBeginForecastDay(m_beginDay->value());
  forecast.setForecastCycles(m_forecastCycles->value());
  forecast.setHistoryMethod(m_historyMethod->checkedId());
  forecast.doForecast();

  m_forecastList->clear();
  m_forecastList->setColumnCount(0);
  m_forecastList->setIconSize(QSize(22, 22));
  m_forecastList->setSortingEnabled(true);
  m_forecastList->sortByColumn(0, Qt::AscendingOrder);

  //add columns
  QStringList headerLabels;
  headerLabels << i18n("Account");

  //add cycle interval columns
  headerLabels << i18nc("Today's forecast", "Current");

  for (int i = 1; i <= forecast.forecastDays(); ++i) {
    QDate forecastDate = QDate::currentDate().addDays(i);
    headerLabels << QLocale().toString(forecastDate, QLocale::LongFormat);
  }

  //add variation columns
  headerLabels << i18n("Total variation");

  //set the columns
  m_forecastList->setHeaderLabels(headerLabels);

  //add default rows
  addTotalRow(m_forecastList, forecast);
  addAssetLiabilityRows(forecast);

  //load asset and liability forecast accounts
  loadAccounts(forecast, file->asset(), m_assetItem, eDetailed);
  loadAccounts(forecast, file->liability(), m_liabilityItem, eDetailed);

  adjustHeadersAndResizeToContents(m_forecastList);

  // add the fixed column only if the horizontal scroll bar is visible
  m_fixedColumnView.reset(m_forecastList->horizontalScrollBar()->isVisible() ? new FixedColumnTreeView(m_forecastList) : 0);
}

void KForecastView::loadSummaryView()
{
  MyMoneyForecast forecast = KMyMoneyGlobalSettings::forecast();
  QList<MyMoneyAccount> accList;
  int dropMinimum;
  int dropZero;

  MyMoneyFile* file = MyMoneyFile::instance();

  //get the settings from current page
  forecast.setForecastDays(m_forecastDays->value());
  forecast.setAccountsCycle(m_accountsCycle->value());
  forecast.setBeginForecastDay(m_beginDay->value());
  forecast.setForecastCycles(m_forecastCycles->value());
  forecast.setHistoryMethod(m_historyMethod->checkedId());
  forecast.doForecast();

  //add columns
  QStringList headerLabels;
  headerLabels << i18n("Account");
  headerLabels << i18nc("Today's forecast", "Current");

  //if beginning of forecast is today, set the begin day to next cycle to avoid repeating the first cycle
  int daysToBeginDay;
  if (QDate::currentDate() < forecast.beginForecastDate()) {
    daysToBeginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
  } else {
    daysToBeginDay = forecast.accountsCycle();
  }
  for (int i = 0; ((i*forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int intervalDays = ((i * forecast.accountsCycle()) + daysToBeginDay);
    headerLabels << i18np("1 day", "%1 days", intervalDays);
  }

  //add variation columns
  headerLabels << i18n("Total variation");

  m_summaryList->clear();
  //set the columns
  m_summaryList->setHeaderLabels(headerLabels);

  m_summaryList->setIconSize(QSize(22, 22));
  m_summaryList->setSortingEnabled(true);
  m_summaryList->sortByColumn(0, Qt::AscendingOrder);

  //add default rows
  addTotalRow(m_summaryList, forecast);
  addAssetLiabilityRows(forecast);

  loadAccounts(forecast, file->asset(), m_assetItem, eSummary);
  loadAccounts(forecast, file->liability(), m_liabilityItem, eSummary);

  adjustHeadersAndResizeToContents(m_summaryList);

  //Add comments to the advice list
  m_adviceText->clear();

  //Get all accounts of the right type to calculate forecast
  m_nameIdx.clear();
  accList = forecast.accountList();
  QList<MyMoneyAccount>::const_iterator accList_t = accList.constBegin();
  for (; accList_t != accList.constEnd(); ++accList_t) {
    MyMoneyAccount acc = *accList_t;
    if (m_nameIdx[acc.id()] != acc.id()) { //Check if the account is there
      m_nameIdx[acc.id()] = acc.id();
    }
  }

  QMap<QString, QString>::ConstIterator it_nc;
  for (it_nc = m_nameIdx.constBegin(); it_nc != m_nameIdx.constEnd(); ++it_nc) {

    const MyMoneyAccount& acc = file->account(*it_nc);
    MyMoneySecurity currency;

    //change currency to deep currency if account is an investment
    if (acc.isInvest()) {
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

    if (dropMinimum != -1
        && !minBalance.isZero()
        && (dropMinimum < dropZero
            || dropZero == -1)) {
      switch (dropMinimum) {
        case -1:
          break;
        case 0:
          msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative).name());
          msg += i18n("The balance of %1 is below the minimum balance %2 today.", acc.name(), MyMoneyUtils::formatMoney(minBalance, acc, currency));
          msg += QString("</font>");
          break;
        default:
          msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative).name());
          msg += i18np("The balance of %2 will drop below the minimum balance %3 in %1 day.",
                       "The balance of %2 will drop below the minimum balance %3 in %1 days.",
                       dropMinimum - 1, acc.name(), MyMoneyUtils::formatMoney(minBalance, acc, currency));
          msg += QString("</font>");
      }

      if (!msg.isEmpty()) {
        m_adviceText->append(msg);
      }
    }

    // a drop below zero is always shown
    msg.clear();
    switch (dropZero) {
      case -1:
        break;
      case 0:
        if (acc.accountGroup() == eMyMoney::Account::Asset) {
          msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative).name());
          msg += i18n("The balance of %1 is below %2 today.", acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), acc, currency));
          msg += QString("</font>");
          break;
        }
        if (acc.accountGroup() == eMyMoney::Account::Liability) {
          msg = i18n("The balance of %1 is above %2 today.", acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), acc, currency));
          break;
        }
        break;
      default:
        if (acc.accountGroup() == eMyMoney::Account::Asset) {
          msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative).name());
          msg += i18np("The balance of %2 will drop below %3 in %1 day.",
                       "The balance of %2 will drop below %3 in %1 days.",
                       dropZero, acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), acc, currency));
          msg += QString("</font>");
          break;
        }
        if (acc.accountGroup() == eMyMoney::Account::Liability) {
          msg = i18np("The balance of %2 will raise above %3 in %1 day.",
                      "The balance of %2 will raise above %3 in %1 days.",
                      dropZero, acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), acc, currency));
          break;
        }
    }
    if (!msg.isEmpty()) {
      m_adviceText->append(msg);
    }

    //advice about trends
    msg.clear();
    MyMoneyMoney accCycleVariation = forecast.accountCycleVariation(acc);
    if (accCycleVariation < MyMoneyMoney()) {
      msg = QString("<font color=\"%1\">").arg(KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative).name());
      msg += i18n("The account %1 is decreasing %2 per cycle.", acc.name(), MyMoneyUtils::formatMoney(accCycleVariation, acc, currency));
      msg += QString("</font>");
    }

    if (!msg.isEmpty()) {
      m_adviceText->append(msg);
    }
  }
  m_adviceText->show();
}

void KForecastView::loadAdvancedView()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyAccount> accList;
  MyMoneySecurity baseCurrency = file->baseCurrency();
  MyMoneyForecast forecast = KMyMoneyGlobalSettings::forecast();
  int daysToBeginDay;

  //get the settings from current page
  forecast.setForecastDays(m_forecastDays->value());
  forecast.setAccountsCycle(m_accountsCycle->value());
  forecast.setBeginForecastDay(m_beginDay->value());
  forecast.setForecastCycles(m_forecastCycles->value());
  forecast.setHistoryMethod(m_historyMethod->checkedId());
  forecast.doForecast();

  //Get all accounts of the right type to calculate forecast
  m_nameIdx.clear();
  accList = forecast.accountList();
  QList<MyMoneyAccount>::const_iterator accList_t = accList.constBegin();
  for (; accList_t != accList.constEnd(); ++accList_t) {
    MyMoneyAccount acc = *accList_t;
    if (m_nameIdx[acc.id()] != acc.id()) { //Check if the account is there
      m_nameIdx[acc.id()] = acc.id();
    }
  }
  //clear the list, including columns
  m_advancedList->clear();
  m_advancedList->setColumnCount(0);
  m_advancedList->setIconSize(QSize(22, 22));

  QStringList headerLabels;

  //add first column of both lists
  headerLabels << i18n("Account");

  //if beginning of forecast is today, set the begin day to next cycle to avoid repeating the first cycle
  if (QDate::currentDate() < forecast.beginForecastDate()) {
    daysToBeginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
  } else {
    daysToBeginDay = forecast.accountsCycle();
  }

  //add columns
  for (int i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
    headerLabels << i18n("Min Bal %1", i);
    headerLabels << i18n("Min Date %1", i);
  }
  for (int i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
    headerLabels << i18n("Max Bal %1", i);
    headerLabels << i18n("Max Date %1", i);
  }
  headerLabels << i18nc("Average balance", "Average");

  m_advancedList->setHeaderLabels(headerLabels);

  QTreeWidgetItem *advancedItem = 0;

  QMap<QString, QString>::ConstIterator it_nc;
  for (it_nc = m_nameIdx.constBegin(); it_nc != m_nameIdx.constEnd(); ++it_nc) {
    const MyMoneyAccount& acc = file->account(*it_nc);
    QString amount;
    MyMoneyMoney amountMM;
    MyMoneySecurity currency;

    //change currency to deep currency if account is an investment
    if (acc.isInvest()) {
      MyMoneySecurity underSecurity = file->security(acc.currencyId());
      currency = file->security(underSecurity.tradingCurrency());
    } else {
      currency = file->security(acc.currencyId());
    }


    advancedItem = new QTreeWidgetItem(m_advancedList, advancedItem, false);
    advancedItem->setText(0, acc.name());
    advancedItem->setIcon(0, acc.accountPixmap());
    int it_c = 1; // iterator for the columns of the listview

    //get minimum balance list
    QList<QDate> minBalanceList = forecast.accountMinimumBalanceDateList(acc);
    QList<QDate>::Iterator t_min;
    for (t_min = minBalanceList.begin(); t_min != minBalanceList.end() ; ++t_min) {
      QDate minDate = *t_min;
      amountMM = forecast.forecastBalance(acc, minDate);

      amount = MyMoneyUtils::formatMoney(amountMM, acc, currency);
      advancedItem->setText(it_c, amount);
      advancedItem->setTextAlignment(it_c, Qt::AlignRight | Qt::AlignVCenter);
      if (amountMM.isNegative()) {
        advancedItem->setForeground(it_c, KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative));
      }
      it_c++;

      QString dateString = QLocale().toString(minDate, QLocale::ShortFormat);
      advancedItem->setText(it_c, dateString);
      advancedItem->setTextAlignment(it_c, Qt::AlignRight | Qt::AlignVCenter);
      if (amountMM.isNegative()) {
        advancedItem->setForeground(it_c, KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative));
      }
      it_c++;
    }

    //get maximum balance list
    QList<QDate> maxBalanceList = forecast.accountMaximumBalanceDateList(acc);
    QList<QDate>::Iterator t_max;
    for (t_max = maxBalanceList.begin(); t_max != maxBalanceList.end() ; ++t_max) {
      QDate maxDate = *t_max;
      amountMM = forecast.forecastBalance(acc, maxDate);

      amount = MyMoneyUtils::formatMoney(amountMM, acc, currency);
      advancedItem->setText(it_c, amount);
      advancedItem->setTextAlignment(it_c, Qt::AlignRight | Qt::AlignVCenter);
      if (amountMM.isNegative()) {
        advancedItem->setForeground(it_c, KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative));
      }
      it_c++;

      QString dateString = QLocale().toString(maxDate, QLocale::ShortFormat);
      advancedItem->setText(it_c, dateString);
      advancedItem->setTextAlignment(it_c, Qt::AlignRight | Qt::AlignVCenter);
      if (amountMM.isNegative()) {
        advancedItem->setForeground(it_c, KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative));
      }
      it_c++;
    }
    //get average balance
    amountMM = forecast.accountAverageBalance(acc);
    amount = MyMoneyUtils::formatMoney(amountMM, acc, currency);
    advancedItem->setText(it_c, amount);
    advancedItem->setTextAlignment(it_c, Qt::AlignRight | Qt::AlignVCenter);
    if (amountMM.isNegative()) {
      advancedItem->setForeground(it_c, KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative));
    }
    it_c++;
  }

  // make sure all data is shown
  adjustHeadersAndResizeToContents(m_advancedList);

  m_advancedList->show();
}

void KForecastView::loadBudgetView()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyForecast forecast = KMyMoneyGlobalSettings::forecast();

  //get the settings from current page and calculate this year based on last year
  QDate historyEndDate = QDate(QDate::currentDate().year() - 1, 12, 31);
  QDate historyStartDate = historyEndDate.addDays(-m_accountsCycle->value() * m_forecastCycles->value());
  QDate forecastStartDate = QDate(QDate::currentDate().year(), 1, 1);
  QDate forecastEndDate = QDate::currentDate().addDays(m_forecastDays->value());
  forecast.setHistoryMethod(m_historyMethod->checkedId());

  MyMoneyBudget budget;
  forecast.createBudget(budget, historyStartDate, historyEndDate, forecastStartDate, forecastEndDate, false);

  m_budgetList->clear();
  m_budgetList->setIconSize(QSize(22, 22));
  m_budgetList->setSortingEnabled(true);
  m_budgetList->sortByColumn(0, Qt::AscendingOrder);

  //add columns
  QStringList headerLabels;
  headerLabels << i18n("Account");

  {
    QDate forecastStartDate = forecast.forecastStartDate();
    QDate forecastEndDate = forecast.forecastEndDate();

    //add cycle interval columns
    QDate f_date = forecastStartDate;
    for (; f_date <= forecastEndDate; f_date = f_date.addMonths(1)) {
      headerLabels << QDate::longMonthName(f_date.month());
    }
  }
  //add total column
  headerLabels << i18nc("Total balance", "Total");

  //set the columns
  m_budgetList->setHeaderLabels(headerLabels);

  //add default rows
  addTotalRow(m_budgetList, forecast);
  addIncomeExpenseRows(forecast);

  //load income and expense budget accounts
  loadAccounts(forecast, file->income(), m_incomeItem, eBudget);
  loadAccounts(forecast, file->expense(), m_expenseItem, eBudget);

  adjustHeadersAndResizeToContents(m_budgetList);
}

QList<MyMoneyPrice> KForecastView::getAccountPrices(const MyMoneyAccount& acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyPrice> prices;
  MyMoneySecurity security = file->baseCurrency();
  try {
    if (acc.isInvest()) {
      security = file->security(acc.currencyId());
      if (security.tradingCurrency() != file->baseCurrency().id()) {
        MyMoneySecurity sec = file->security(security.tradingCurrency());
        prices += file->price(sec.id(), file->baseCurrency().id());
      }
    } else if (acc.currencyId() != file->baseCurrency().id()) {
      if (acc.currencyId() != file->baseCurrency().id()) {
        security = file->security(acc.currencyId());
        prices += file->price(acc.currencyId(), file->baseCurrency().id());
      }
    }

  } catch (const MyMoneyException &e) {
    qDebug() << Q_FUNC_INFO << " caught exception while adding " << acc.name() << "[" << acc.id() << "]: " << e.what();
  }
  return prices;
}

void KForecastView::addAssetLiabilityRows(const MyMoneyForecast& forecast)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  m_assetItem = new QTreeWidgetItem(m_totalItem);
  m_assetItem->setText(0, file->asset().name());
  m_assetItem->setIcon(0, file->asset().accountPixmap());
  m_assetItem->setData(0, ForecastRole, QVariant::fromValue(forecast));
  m_assetItem->setData(0, AccountRole, QVariant::fromValue(file->asset()));
  m_assetItem->setExpanded(true);

  m_liabilityItem = new QTreeWidgetItem(m_totalItem);
  m_liabilityItem->setText(0, file->liability().name());
  m_liabilityItem->setIcon(0, file->liability().accountPixmap());
  m_liabilityItem->setData(0, ForecastRole, QVariant::fromValue(forecast));
  m_liabilityItem->setData(0, AccountRole, QVariant::fromValue(file->liability()));
  m_liabilityItem->setExpanded(true);
}

void KForecastView::addIncomeExpenseRows(const MyMoneyForecast& forecast)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  m_incomeItem = new QTreeWidgetItem(m_totalItem);
  m_incomeItem->setText(0, file->income().name());
  m_incomeItem->setIcon(0, file->income().accountPixmap());
  m_incomeItem->setData(0, ForecastRole, QVariant::fromValue(forecast));
  m_incomeItem->setData(0, AccountRole, QVariant::fromValue(file->income()));
  m_incomeItem->setExpanded(true);

  m_expenseItem = new QTreeWidgetItem(m_totalItem);
  m_expenseItem->setText(0, file->expense().name());
  m_expenseItem->setIcon(0, file->expense().accountPixmap());
  m_expenseItem->setData(0, ForecastRole, QVariant::fromValue(forecast));
  m_expenseItem->setData(0, AccountRole, QVariant::fromValue(file->expense()));
  m_expenseItem->setExpanded(true);
}

void KForecastView::addTotalRow(QTreeWidget* forecastList, const MyMoneyForecast& forecast)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  m_totalItem = new QTreeWidgetItem(forecastList);
  QFont font;
  font.setBold(true);
  m_totalItem->setFont(0, font);
  m_totalItem->setText(0, i18nc("Total balance", "Total"));
  m_totalItem->setIcon(0, file->asset().accountPixmap());
  m_totalItem->setData(0, ForecastRole, QVariant::fromValue(forecast));
  m_totalItem->setData(0, AccountRole, QVariant::fromValue(file->asset()));
  m_totalItem->setExpanded(true);
}

bool KForecastView::includeAccount(MyMoneyForecast& forecast, const MyMoneyAccount& acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  if (forecast.isForecastAccount(acc))
    return true;

  QStringList accounts = acc.accountList();

  if (accounts.size() > 0) {
    QStringList::ConstIterator it_acc;
    for (it_acc = accounts.constBegin(); it_acc != accounts.constEnd(); ++it_acc) {
      MyMoneyAccount account = file->account(*it_acc);
      if (includeAccount(forecast, account))
        return true;
    }
  }
  return false;
}

void KForecastView::adjustHeadersAndResizeToContents(QTreeWidget *widget)
{
  QSize sizeHint(0, widget->sizeHintForRow(0));
  QTreeWidgetItem *header = widget->headerItem();
  for (int i = 0; i < header->columnCount(); ++i) {
    if (i > 0) {
      header->setData(i, Qt::TextAlignmentRole, Qt::AlignRight);
      // make sure that the row height stays the same even when the column that has icons is not visible
      if (m_totalItem) {
        m_totalItem->setSizeHint(i, sizeHint);
      }
    }
    widget->resizeColumnToContents(i);
  }
}

void KForecastView::loadAccounts(MyMoneyForecast& forecast, const MyMoneyAccount& account, QTreeWidgetItem* parentItem, int forecastType)
{
  QMap<QString, QString> nameIdx;
  QStringList accList;
  MyMoneyFile* file = MyMoneyFile::instance();
  QTreeWidgetItem *forecastItem = 0;

  //Get all accounts of the right type to calculate forecast
  accList = account.accountList();

  if (accList.size() == 0)
    return;

  QStringList::ConstIterator accList_t;
  for (accList_t = accList.constBegin(); accList_t != accList.constEnd(); ++accList_t) {
    MyMoneyAccount subAccount = file->account(*accList_t);
    //only add the account if it is a forecast account or the parent of a forecast account
    if (includeAccount(forecast, subAccount)) {
      nameIdx[subAccount.id()] = subAccount.id();
    }
  }

  QMap<QString, QString>::ConstIterator it_nc;
  for (it_nc = nameIdx.constBegin(); it_nc != nameIdx.constEnd(); ++it_nc) {

    const MyMoneyAccount subAccount = file->account(*it_nc);
    MyMoneySecurity currency;
    if (subAccount.isInvest()) {
      MyMoneySecurity underSecurity = file->security(subAccount.currencyId());
      currency = file->security(underSecurity.tradingCurrency());
    } else {
      currency = file->security(subAccount.currencyId());
    }

    forecastItem = new QTreeWidgetItem(parentItem);
    forecastItem->setText(0, subAccount.name());
    forecastItem->setIcon(0, subAccount.accountPixmap());
    forecastItem->setData(0, ForecastRole, QVariant::fromValue(forecast));
    forecastItem->setData(0, AccountRole, QVariant::fromValue(subAccount));
    forecastItem->setExpanded(true);

    switch (forecastType) {
      case eSummary:
        updateSummary(forecastItem);
        break;
      case eDetailed:
        updateDetailed(forecastItem);
        break;
      case eBudget:
        updateBudget(forecastItem);
        break;
      default:
        break;
    }

    loadAccounts(forecast, subAccount, forecastItem, forecastType);
  }
}

void KForecastView::updateSummary(QTreeWidgetItem *item)
{
  MyMoneyMoney amountMM;
  int it_c = 1; // iterator for the columns of the listview
  MyMoneyFile* file = MyMoneyFile::instance();
  int daysToBeginDay;

  MyMoneyForecast forecast = item->data(0, ForecastRole).value<MyMoneyForecast>();

  if (QDate::currentDate() < forecast.beginForecastDate()) {
    daysToBeginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
  } else {
    daysToBeginDay = forecast.accountsCycle();
  }

  MyMoneyAccount account = item->data(0, AccountRole).value<MyMoneyAccount>();
  MyMoneySecurity currency;
  if (account.isInvest()) {
    MyMoneySecurity underSecurity = file->security(account.currencyId());
    currency = file->security(underSecurity.tradingCurrency());
  } else {
    currency = file->security(account.currencyId());
  }


  //add current balance column
  QDate summaryDate = QDate::currentDate();
  amountMM = forecast.forecastBalance(account, summaryDate);

  //calculate the balance in base currency for the total row
  setAmount(item, it_c, amountMM);
  setValue(item, it_c, amountMM, summaryDate);
  showAmount(item, it_c, amountMM, currency);
  it_c++;

  //iterate through all other columns
  for (QDate summaryDate = QDate::currentDate().addDays(daysToBeginDay); summaryDate <= forecast.forecastEndDate(); summaryDate = summaryDate.addDays(forecast.accountsCycle()), ++it_c) {
    amountMM = forecast.forecastBalance(account, summaryDate);

    //calculate the balance in base currency for the total row
    setAmount(item, it_c, amountMM);
    setValue(item, it_c, amountMM, summaryDate);
    showAmount(item, it_c, amountMM, currency);
  }
  //calculate and add variation per cycle
  setNegative(item, forecast.accountTotalVariation(account).isNegative());
  setAmount(item, it_c, forecast.accountTotalVariation(account));
  setValue(item, it_c, forecast.accountTotalVariation(account), forecast.forecastEndDate());
  showAmount(item, it_c, forecast.accountTotalVariation(account), currency);
}

void KForecastView::updateDetailed(QTreeWidgetItem *item)
{
  QString amount;
  QString vAmount;
  MyMoneyMoney vAmountMM;
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyAccount account = item->data(0, AccountRole).value<MyMoneyAccount>();

  MyMoneySecurity currency;
  if (account.isInvest()) {
    MyMoneySecurity underSecurity = file->security(account.currencyId());
    currency = file->security(underSecurity.tradingCurrency());
  } else {
    currency = file->security(account.currencyId());
  }

  int it_c = 1; // iterator for the columns of the listview

  MyMoneyForecast forecast = item->data(0, ForecastRole).value<MyMoneyForecast>();

  for (QDate forecastDate = QDate::currentDate(); forecastDate <= forecast.forecastEndDate(); ++it_c, forecastDate = forecastDate.addDays(1)) {
    MyMoneyMoney amountMM = forecast.forecastBalance(account, forecastDate);

    //calculate the balance in base currency for the total row
    setAmount(item, it_c, amountMM);
    setValue(item, it_c, amountMM, forecastDate);
    showAmount(item, it_c, amountMM, currency);
  }

  //calculate and add variation per cycle
  vAmountMM = forecast.accountTotalVariation(account);
  setAmount(item, it_c, vAmountMM);
  setValue(item, it_c, vAmountMM, forecast.forecastEndDate());
  showAmount(item, it_c, vAmountMM, currency);
}

void KForecastView::updateBudget(QTreeWidgetItem *item)
{
  MyMoneySecurity currency;
  MyMoneyMoney tAmountMM;

  MyMoneyForecast forecast = item->data(0, ForecastRole).value<MyMoneyForecast>();

  MyMoneyFile* file = MyMoneyFile::instance();
  int it_c = 1; // iterator for the columns of the listview
  QDate forecastDate = forecast.forecastStartDate();

  MyMoneyAccount account = item->data(0, AccountRole).value<MyMoneyAccount>();

  if (account.isInvest()) {
    MyMoneySecurity underSecurity = file->security(account.currencyId());
    currency = file->security(underSecurity.tradingCurrency());
  } else {
    currency = file->security(account.currencyId());
  }

  //iterate columns
  for (; forecastDate <= forecast.forecastEndDate(); forecastDate = forecastDate.addMonths(1), ++it_c) {
    MyMoneyMoney amountMM;
    amountMM = forecast.forecastBalance(account, forecastDate);
    if (account.accountType() == eMyMoney::Account::Expense)
      amountMM = -amountMM;

    tAmountMM += amountMM;
    setAmount(item, it_c, amountMM);
    setValue(item, it_c, amountMM, forecastDate);
    showAmount(item, it_c, amountMM, currency);
  }

  //set total column
  setAmount(item, it_c, tAmountMM);
  setValue(item, it_c, tAmountMM, forecast.forecastEndDate());
  showAmount(item, it_c, tAmountMM, currency);
}

void KForecastView::setNegative(QTreeWidgetItem *item, bool isNegative)
{
  if (isNegative) {
    for (int i = 0; i < item->columnCount(); ++i) {
      item->setForeground(i, KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative));
    }
  }
}

void KForecastView::showAmount(QTreeWidgetItem* item, int column, const MyMoneyMoney& amount, const MyMoneySecurity& security)
{
  item->setText(column, MyMoneyUtils::formatMoney(amount, security));
  item->setTextAlignment(column, Qt::AlignRight | Qt::AlignVCenter);
  item->setFont(column, item->font(0));
  if (amount.isNegative()) {
    item->setForeground(column, KMyMoneyGlobalSettings::schemeColor(SchemeColor::Negative));
  }
}

void KForecastView::adjustParentValue(QTreeWidgetItem *item, int column, const MyMoneyMoney& value)
{
  if (!item)
    return;

  item->setData(column, ValueRole, QVariant::fromValue(item->data(column, ValueRole).value<MyMoneyMoney>() + value));
  item->setData(column, ValueRole, QVariant::fromValue(item->data(column, ValueRole).value<MyMoneyMoney>().convert(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction())));

  // if the entry has no children,
  // or it is the top entry
  // or it is currently not open
  // we need to display the value of it
  if (item->childCount() == 0 || !item->parent() || (!item->isExpanded() && item->childCount() > 0) || (item->parent() && !item->parent()->parent())) {
    if (item->childCount() > 0)
      item->setText(column, " ");
    MyMoneyMoney amount = item->data(column, ValueRole).value<MyMoneyMoney>();
    showAmount(item, column, amount, MyMoneyFile::instance()->baseCurrency());
  }

  // now make sure, the upstream accounts also get notified about the value change
  adjustParentValue(item->parent(), column, value);
}

void KForecastView::setValue(QTreeWidgetItem* item, int column, const MyMoneyMoney& amount, const QDate& forecastDate)
{
  MyMoneyAccount account = item->data(0, AccountRole).value<MyMoneyAccount>();
  //calculate the balance in base currency for the total row
  if (account.currencyId() != MyMoneyFile::instance()->baseCurrency().id()) {
    ReportAccount repAcc = ReportAccount(account.id());
    MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(forecastDate);
    MyMoneyMoney baseAmountMM = amount * curPrice;
    MyMoneyMoney value = baseAmountMM.convert(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction());
    item->setData(column, ValueRole, QVariant::fromValue(value));
    adjustParentValue(item->parent(), column, value);
  } else {
    item->setData(column, ValueRole, QVariant::fromValue(item->data(column, ValueRole).value<MyMoneyMoney>() + amount));
    adjustParentValue(item->parent(), column, amount);
  }
}

void KForecastView::setAmount(QTreeWidgetItem* item, int column, const MyMoneyMoney& amount)
{
  item->setData(column, AmountRole, QVariant::fromValue(amount));
  item->setTextAlignment(column, Qt::AlignRight | Qt::AlignVCenter);
}

void KForecastView::itemExpanded(QTreeWidgetItem *item)
{
  if (!item->parent() || !item->parent()->parent())
    return;
  for (int i = 1; i < item->columnCount(); ++i) {
    showAmount(item, i, item->data(i, AmountRole).value<MyMoneyMoney>(), MyMoneyFile::instance()->security(item->data(0, AccountRole).value<MyMoneyAccount>().currencyId()));
  }
}

void KForecastView::itemCollapsed(QTreeWidgetItem *item)
{
  for (int i = 1; i < item->columnCount(); ++i) {
    showAmount(item, i, item->data(i, ValueRole).value<MyMoneyMoney>(), MyMoneyFile::instance()->baseCurrency());
  }
}

void KForecastView::loadChartView()
{
  MyMoneyReport::EDetailLevel detailLevel[4] = { MyMoneyReport::eDetailAll, MyMoneyReport::eDetailTop, MyMoneyReport::eDetailGroup, MyMoneyReport::eDetailTotal };

  MyMoneyReport reportCfg = MyMoneyReport(
                              MyMoneyReport::eAssetLiability,
                              MyMoneyReport::eMonths,
                              MyMoneyTransactionFilter::userDefined, // overridden by the setDateFilter() call below
                              detailLevel[m_comboDetail->currentIndex()],
                              i18n("Net Worth Forecast"),
                              i18n("Generated Report"));

  reportCfg.setChartByDefault(true);
  reportCfg.setChartCHGridLines(false);
  reportCfg.setChartSVGridLines(false);
  reportCfg.setChartType(MyMoneyReport::eChartLine);
  reportCfg.setIncludingSchedules(false);
  // FIXME: this causes a crash
  //reportCfg.setColumnsAreDays( true );
  reportCfg.setChartDataLabels(false);
  reportCfg.setConvertCurrency(true);
  reportCfg.setIncludingForecast(true);
  reportCfg.setDateFilter(QDate::currentDate(), QDate::currentDate().addDays(m_forecastDays->value()));
  reports::PivotTable table(reportCfg);

  table.drawChart(*m_forecastChart);

  // Adjust the size
  m_forecastChart->resize(m_tab->width() - 10, m_tab->height());
  //m_forecastChart->show();
  m_forecastChart->update();

}
