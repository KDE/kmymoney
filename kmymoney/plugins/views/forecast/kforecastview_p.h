/***************************************************************************
                          kforecastview.cpp
                             -------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
                           (C) 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KFORECASTVIEW_P_H
#define KFORECASTVIEW_P_H

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

#include "ui_kforecastview.h"
#include "forecastviewsettings.h"
#include "kmymoneyviewbase_p.h"
#include "mymoneymoney.h"
#include "mymoneyforecast.h"
#include "mymoneyprice.h"
#include "mymoneyutils.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneysecurity.h"
#include "kmymoneysettings.h"
#include "mymoneybudget.h"
#include "fixedcolumntreeview.h"
#include "icons.h"
#include "mymoneyenums.h"
#include "kmymoneyutils.h"
#include "kmymoneyplugin.h"
#include "plugins/views/reports/reportsviewenums.h"

using namespace Icons;

typedef enum {
  SummaryView = 0,
  ListView,
  AdvancedView,
  BudgetView,
  ChartView,
  // insert new values above this line
  MaxViewTabs
} ForecastViewTab;

enum ForecastViewRoles {
  ForecastRole = Qt::UserRole,     /**< The forecast is held in this role.*/
  AccountRole = Qt::UserRole + 1,  /**< The MyMoneyAccount is stored in this role in column 0.*/
  AmountRole = Qt::UserRole + 2,   /**< The amount.*/
  ValueRole = Qt::UserRole + 3,    /**< The value.*/
};

enum EForecastViewType { eSummary = 0, eDetailed, eAdvanced, eBudget, eUndefined };

class KForecastViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KForecastView)

public:
  explicit KForecastViewPrivate(KForecastView *qq)
    : KMyMoneyViewBasePrivate(qq)
    , ui(new Ui::KForecastView)
    , m_needLoad(true)
    , m_totalItem(nullptr)
    , m_assetItem(nullptr)
    , m_liabilityItem(nullptr)
    , m_incomeItem(nullptr)
    , m_expenseItem(nullptr)
    , m_chartLayout(nullptr)
    , m_forecastChart(nullptr)
  {
  }

  ~KForecastViewPrivate()
  {
    delete ui;
  }

  void init()
  {
    Q_Q(KForecastView);
    m_needLoad = false;
    ui->setupUi(q);

    for (int i = 0; i < MaxViewTabs; ++i)
      m_needReload[i] = false;

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group("Last Use Settings");
    ui->m_tab->setCurrentIndex(grp.readEntry("KForecastView_LastType", 0));

    ui->m_forecastButton->setIcon(Icons::get(Icon::Forecast));

    q->connect(ui->m_tab, &QTabWidget::currentChanged, q, &KForecastView::slotTabChanged);

    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KForecastView::refresh);

    q->connect(ui->m_forecastButton, &QAbstractButton::clicked, q, &KForecastView::slotManualForecast);

    ui->m_forecastList->setUniformRowHeights(true);
    ui->m_forecastList->setAllColumnsShowFocus(true);
    ui->m_summaryList->setAllColumnsShowFocus(true);
    ui->m_budgetList->setAllColumnsShowFocus(true);
    ui->m_advancedList->setAlternatingRowColors(true);

    q->connect(ui->m_forecastList, &QTreeWidget::itemExpanded, q, &KForecastView::itemExpanded);
    q->connect(ui->m_forecastList, &QTreeWidget::itemCollapsed, q, &KForecastView::itemCollapsed);
    q->connect(ui->m_summaryList, &QTreeWidget::itemExpanded, q, &KForecastView::itemExpanded);
    q->connect(ui->m_summaryList, &QTreeWidget::itemCollapsed, q, &KForecastView::itemCollapsed);
    q->connect(ui->m_budgetList, &QTreeWidget::itemExpanded, q, &KForecastView::itemExpanded);
    q->connect(ui->m_budgetList, &QTreeWidget::itemCollapsed, q, &KForecastView::itemCollapsed);

    m_chartLayout = ui->m_tabChart->layout();
    m_chartLayout->setSpacing(6);

    loadForecastSettings();
  }

  void loadForecast(ForecastViewTab tab)
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

  void loadListView()
  {
    MyMoneyForecast forecast = KMyMoneyUtils::forecast();
    const auto file = MyMoneyFile::instance();

    //get the settings from current page
    forecast.setForecastDays(ui->m_forecastDays->value());
    forecast.setAccountsCycle(ui->m_accountsCycle->value());
    forecast.setBeginForecastDay(ui->m_beginDay->value());
    forecast.setForecastCycles(ui->m_forecastCycles->value());
    forecast.setHistoryMethod(ui->m_historyMethod->checkedId());
    forecast.doForecast();

    ui->m_forecastList->clear();
    ui->m_forecastList->setColumnCount(0);
    ui->m_forecastList->setIconSize(QSize(22, 22));
    ui->m_forecastList->setSortingEnabled(true);
    ui->m_forecastList->sortByColumn(0, Qt::AscendingOrder);

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
    ui->m_forecastList->setHeaderLabels(headerLabels);

    //add default rows
    addTotalRow(ui->m_forecastList, forecast);
    addAssetLiabilityRows(forecast);

    //load asset and liability forecast accounts
    loadAccounts(forecast, file->asset(), m_assetItem, eDetailed);
    loadAccounts(forecast, file->liability(), m_liabilityItem, eDetailed);

    adjustHeadersAndResizeToContents(ui->m_forecastList);

    // add the fixed column only if the horizontal scroll bar is visible
    m_fixedColumnView.reset(ui->m_forecastList->horizontalScrollBar()->isVisible() ? new FixedColumnTreeView(ui->m_forecastList) : 0);
  }

  void loadAccounts(MyMoneyForecast& forecast, const MyMoneyAccount& account, QTreeWidgetItem* parentItem, int forecastType)
  {
    QMap<QString, QString> nameIdx;
    const auto file = MyMoneyFile::instance();
    QTreeWidgetItem *forecastItem = 0;

    //Get all accounts of the right type to calculate forecast
    const auto accList = account.accountList();

    if (accList.isEmpty())
      return;

    foreach (const auto sAccount, accList) {
      auto subAccount = file->account(sAccount);
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
        case EForecastViewType::eBudget:
          updateBudget(forecastItem);
          break;
        default:
          break;
      }

      loadAccounts(forecast, subAccount, forecastItem, forecastType);
    }
  }

  void loadSummaryView()
  {
    MyMoneyForecast forecast = KMyMoneyUtils::forecast();
    QList<MyMoneyAccount> accList;

    const auto file = MyMoneyFile::instance();

    //get the settings from current page
    forecast.setForecastDays(ui->m_forecastDays->value());
    forecast.setAccountsCycle(ui->m_accountsCycle->value());
    forecast.setBeginForecastDay(ui->m_beginDay->value());
    forecast.setForecastCycles(ui->m_forecastCycles->value());
    forecast.setHistoryMethod(ui->m_historyMethod->checkedId());
    forecast.doForecast();

    //add columns
    QStringList headerLabels;
    headerLabels << i18n("Account");
    headerLabels << i18nc("Today's forecast", "Current");

    //if beginning of forecast is today, set the begin day to next cycle to avoid repeating the first cycle
    qint64 daysToBeginDay;
    if (QDate::currentDate() < forecast.beginForecastDate()) {
      daysToBeginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
    } else {
      daysToBeginDay = forecast.accountsCycle();
    }
    for (auto i = 0; ((i*forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
      auto intervalDays = ((i * forecast.accountsCycle()) + daysToBeginDay);
      headerLabels << i18np("1 day", "%1 days", intervalDays);
    }

    //add variation columns
    headerLabels << i18n("Total variation");

    ui->m_summaryList->clear();
    //set the columns
    ui->m_summaryList->setHeaderLabels(headerLabels);

    ui->m_summaryList->setIconSize(QSize(22, 22));
    ui->m_summaryList->setSortingEnabled(true);
    ui->m_summaryList->sortByColumn(0, Qt::AscendingOrder);

    //add default rows
    addTotalRow(ui->m_summaryList, forecast);
    addAssetLiabilityRows(forecast);

    loadAccounts(forecast, file->asset(), m_assetItem, eSummary);
    loadAccounts(forecast, file->liability(), m_liabilityItem, eSummary);

    adjustHeadersAndResizeToContents(ui->m_summaryList);

    //Add comments to the advice list
    ui->m_adviceText->clear();

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
      auto dropMinimum = forecast.daysToMinimumBalance(acc);

      //Check if the account is going to be below zero in the future
      auto dropZero = forecast.daysToZeroBalance(acc);

      // spit out possible warnings
      QString msg;

      // if a minimum balance has been specified, an appropriate warning will
      // only be shown, if the drop below 0 is on a different day or not present

      if (dropMinimum != -1
          && !minBalance.isZero()
          && (dropMinimum < dropZero
              || dropZero == -1)) {
        switch (dropMinimum) {
          case 0:
            msg = QString("<font color=\"%1\">").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name());
            msg += i18n("The balance of %1 is below the minimum balance %2 today.", acc.name(), MyMoneyUtils::formatMoney(minBalance, acc, currency));
            msg += QString("</font>");
            break;
          default:
            msg = QString("<font color=\"%1\">").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name());
            msg += i18np("The balance of %2 will drop below the minimum balance %3 in %1 day.",
                         "The balance of %2 will drop below the minimum balance %3 in %1 days.",
                         dropMinimum - 1, acc.name(), MyMoneyUtils::formatMoney(minBalance, acc, currency));
            msg += QString("</font>");
        }

        if (!msg.isEmpty()) {
          ui->m_adviceText->append(msg);
        }
      }

      // a drop below zero is always shown
      msg.clear();
      switch (dropZero) {
        case -1:
          break;
        case 0:
          if (acc.accountGroup() == eMyMoney::Account::Type::Asset) {
            msg = QString("<font color=\"%1\">").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name());
            msg += i18n("The balance of %1 is below %2 today.", acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), acc, currency));
            msg += QString("</font>");
            break;
          }
          if (acc.accountGroup() == eMyMoney::Account::Type::Liability) {
            msg = i18n("The balance of %1 is above %2 today.", acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), acc, currency));
            break;
          }
          break;
        default:
          if (acc.accountGroup() == eMyMoney::Account::Type::Asset) {
            msg = QString("<font color=\"%1\">").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name());
            msg += i18np("The balance of %2 will drop below %3 in %1 day.",
                         "The balance of %2 will drop below %3 in %1 days.",
                         dropZero, acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), acc, currency));
            msg += QString("</font>");
            break;
          }
          if (acc.accountGroup() == eMyMoney::Account::Type::Liability) {
            msg = i18np("The balance of %2 will raise above %3 in %1 day.",
                        "The balance of %2 will raise above %3 in %1 days.",
                        dropZero, acc.name(), MyMoneyUtils::formatMoney(MyMoneyMoney(), acc, currency));
            break;
          }
      }
      if (!msg.isEmpty()) {
        ui->m_adviceText->append(msg);
      }

      //advice about trends
      msg.clear();
      MyMoneyMoney accCycleVariation = forecast.accountCycleVariation(acc);
      if (accCycleVariation < MyMoneyMoney()) {
        msg = QString("<font color=\"%1\">").arg(KMyMoneySettings::schemeColor(SchemeColor::Negative).name());
        msg += i18n("The account %1 is decreasing %2 per cycle.", acc.name(), MyMoneyUtils::formatMoney(accCycleVariation, acc, currency));
        msg += QString("</font>");
      }

      if (!msg.isEmpty()) {
        ui->m_adviceText->append(msg);
      }
    }
    ui->m_adviceText->show();
  }

  void loadAdvancedView()
  {
    const auto file = MyMoneyFile::instance();
    QList<MyMoneyAccount> accList;
    MyMoneySecurity baseCurrency = file->baseCurrency();
    MyMoneyForecast forecast = KMyMoneyUtils::forecast();
    qint64 daysToBeginDay;

    //get the settings from current page
    forecast.setForecastDays(ui->m_forecastDays->value());
    forecast.setAccountsCycle(ui->m_accountsCycle->value());
    forecast.setBeginForecastDay(ui->m_beginDay->value());
    forecast.setForecastCycles(ui->m_forecastCycles->value());
    forecast.setHistoryMethod(ui->m_historyMethod->checkedId());
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
    ui->m_advancedList->clear();
    ui->m_advancedList->setColumnCount(0);
    ui->m_advancedList->setIconSize(QSize(22, 22));

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
    for (auto i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
      headerLabels << i18n("Min Bal %1", i);
      headerLabels << i18n("Min Date %1", i);
    }
    for (auto i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
      headerLabels << i18n("Max Bal %1", i);
      headerLabels << i18n("Max Date %1", i);
    }
    headerLabels << i18nc("Average balance", "Average");

    ui->m_advancedList->setHeaderLabels(headerLabels);

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


      advancedItem = new QTreeWidgetItem(ui->m_advancedList, advancedItem, false);
      advancedItem->setText(0, acc.name());
      advancedItem->setIcon(0, acc.accountPixmap());
      auto it_c = 1; // iterator for the columns of the listview

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
          advancedItem->setForeground(it_c, KMyMoneySettings::schemeColor(SchemeColor::Negative));
        }
        it_c++;

        QString dateString = QLocale().toString(minDate, QLocale::ShortFormat);
        advancedItem->setText(it_c, dateString);
        advancedItem->setTextAlignment(it_c, Qt::AlignRight | Qt::AlignVCenter);
        if (amountMM.isNegative()) {
          advancedItem->setForeground(it_c, KMyMoneySettings::schemeColor(SchemeColor::Negative));
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
          advancedItem->setForeground(it_c, KMyMoneySettings::schemeColor(SchemeColor::Negative));
        }
        it_c++;

        QString dateString = QLocale().toString(maxDate, QLocale::ShortFormat);
        advancedItem->setText(it_c, dateString);
        advancedItem->setTextAlignment(it_c, Qt::AlignRight | Qt::AlignVCenter);
        if (amountMM.isNegative()) {
          advancedItem->setForeground(it_c, KMyMoneySettings::schemeColor(SchemeColor::Negative));
        }
        it_c++;
      }
      //get average balance
      amountMM = forecast.accountAverageBalance(acc);
      amount = MyMoneyUtils::formatMoney(amountMM, acc, currency);
      advancedItem->setText(it_c, amount);
      advancedItem->setTextAlignment(it_c, Qt::AlignRight | Qt::AlignVCenter);
      if (amountMM.isNegative()) {
        advancedItem->setForeground(it_c, KMyMoneySettings::schemeColor(SchemeColor::Negative));
      }
      it_c++;
    }

    // make sure all data is shown
    adjustHeadersAndResizeToContents(ui->m_advancedList);

    ui->m_advancedList->show();
  }

  void loadBudgetView()
  {
    const auto file = MyMoneyFile::instance();
    MyMoneyForecast forecast = KMyMoneyUtils::forecast();

    //get the settings from current page and calculate this year based on last year
    QDate historyEndDate = QDate(QDate::currentDate().year() - 1, 12, 31);
    QDate historyStartDate = historyEndDate.addDays(-ui->m_accountsCycle->value() * ui->m_forecastCycles->value());
    QDate forecastStartDate = QDate(QDate::currentDate().year(), 1, 1);
    QDate forecastEndDate = QDate::currentDate().addDays(ui->m_forecastDays->value());
    forecast.setHistoryMethod(ui->m_historyMethod->checkedId());

    MyMoneyBudget budget;
    forecast.createBudget(budget, historyStartDate, historyEndDate, forecastStartDate, forecastEndDate, false);

    ui->m_budgetList->clear();
    ui->m_budgetList->setIconSize(QSize(22, 22));
    ui->m_budgetList->setSortingEnabled(true);
    ui->m_budgetList->sortByColumn(0, Qt::AscendingOrder);

    //add columns
    QStringList headerLabels;
    headerLabels << i18n("Account");

    {
      forecastStartDate = forecast.forecastStartDate();
      forecastEndDate = forecast.forecastEndDate();

      //add cycle interval columns
      QDate f_date = forecastStartDate;
      for (; f_date <= forecastEndDate; f_date = f_date.addMonths(1)) {
        headerLabels << QDate::longMonthName(f_date.month());
      }
    }
    //add total column
    headerLabels << i18nc("Total balance", "Total");

    //set the columns
    ui->m_budgetList->setHeaderLabels(headerLabels);

    //add default rows
    addTotalRow(ui->m_budgetList, forecast);
    addIncomeExpenseRows(forecast);

    //load income and expense budget accounts
    loadAccounts(forecast, file->income(), m_incomeItem, EForecastViewType::eBudget);
    loadAccounts(forecast, file->expense(), m_expenseItem, EForecastViewType::eBudget);

    adjustHeadersAndResizeToContents(ui->m_budgetList);
  }

  void loadChartView()
  {
    if (m_forecastChart)
      delete m_forecastChart;

    if (const auto reportsPlugin = pPlugins.data.value("reportsview", nullptr)) {
      const QString args =
          QString::number(ui->m_comboDetail->currentIndex()) + ';' +
          QString::number(ui->m_forecastDays->value()) + ';' +
          QString::number(ui->m_tab->width()) + ';' +
          QString::number(ui->m_tab->height());
      const auto variantReport = reportsPlugin->requestData(args, eWidgetPlugin::WidgetType::NetWorthForecastWithArgs);
      if (!variantReport.isNull())
        m_forecastChart = variantReport.value<QWidget *>();
      else
        m_forecastChart = new QLabel(i18n("No data provided by reports plugin for this chart."));
    } else {
        m_forecastChart = new QLabel(i18n("Enable reports plugin to see this chart."));
    }
    m_chartLayout->addWidget(m_forecastChart);
  }

  void loadForecastSettings()
  {
    //fill the settings controls
    ui->m_forecastDays->setValue(KMyMoneySettings::forecastDays());
    ui->m_accountsCycle->setValue(KMyMoneySettings::forecastAccountCycle());
    ui->m_beginDay->setValue(KMyMoneySettings::beginForecastDay());
    ui->m_forecastCycles->setValue(KMyMoneySettings::forecastCycles());
    ui->m_historyMethod->setId(ui->radioButton11, 0); // simple moving avg
    ui->m_historyMethod->setId(ui->radioButton12, 1); // weighted moving avg
    ui->m_historyMethod->setId(ui->radioButton13, 2); // linear regression
    ui->m_historyMethod->button(KMyMoneySettings::historyMethod())->setChecked(true);
    switch (KMyMoneySettings::forecastMethod()) {
      case 0:
        ui->m_forecastMethod->setText(i18nc("Scheduled method", "Scheduled"));
        ui->m_forecastCycles->setDisabled(true);
        ui->m_historyMethodGroupBox->setDisabled(true);
        break;
      case 1:
        ui->m_forecastMethod->setText(i18nc("History-based method", "History"));
        ui->m_forecastCycles->setEnabled(true);
        ui->m_historyMethodGroupBox->setEnabled(true);
        break;
      default:
        ui->m_forecastMethod->setText(i18nc("Unknown forecast method", "Unknown"));
        break;
    }
  }

  void addAssetLiabilityRows(const MyMoneyForecast& forecast)
  {
    const auto file = MyMoneyFile::instance();

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

  void addIncomeExpenseRows(const MyMoneyForecast& forecast)
  {
    const auto file = MyMoneyFile::instance();

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

  void addTotalRow(QTreeWidget* forecastList, const MyMoneyForecast& forecast)
  {
    const auto file = MyMoneyFile::instance();

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

  bool includeAccount(MyMoneyForecast& forecast, const MyMoneyAccount& acc)
  {
    const auto file = MyMoneyFile::instance();

    if (forecast.isForecastAccount(acc))
      return true;

    foreach (const auto sAccount, acc.accountList()) {
      auto account = file->account(sAccount);
      if (includeAccount(forecast, account))
        return true;
    }
    return false;
  }

  void adjustHeadersAndResizeToContents(QTreeWidget *widget)
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

  void setNegative(QTreeWidgetItem *item, bool isNegative)
  {
    if (isNegative) {
      for (int i = 0; i < item->columnCount(); ++i) {
        item->setForeground(i, KMyMoneySettings::schemeColor(SchemeColor::Negative));
      }
    }
  }

  void showAmount(QTreeWidgetItem* item, int column, const MyMoneyMoney& amount, const MyMoneySecurity& security)
  {
    item->setText(column, MyMoneyUtils::formatMoney(amount, security));
    item->setTextAlignment(column, Qt::AlignRight | Qt::AlignVCenter);
    item->setFont(column, item->font(0));
    if (amount.isNegative()) {
      item->setForeground(column, KMyMoneySettings::schemeColor(SchemeColor::Negative));
    }
  }

  void adjustParentValue(QTreeWidgetItem *item, int column, const MyMoneyMoney& value)
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

  void setValue(QTreeWidgetItem* item, int column, const MyMoneyMoney& amount, const QDate& forecastDate)
  {
    MyMoneyAccount account = item->data(0, AccountRole).value<MyMoneyAccount>();
    //calculate the balance in base currency for the total row
    if (account.currencyId() != MyMoneyFile::instance()->baseCurrency().id()) {
      const auto file = MyMoneyFile::instance();
      const auto curPrice = file->price(account.tradingCurrencyId(), file->baseCurrency().id(), forecastDate);
      const auto curRate = curPrice.rate(file->baseCurrency().id());
      auto baseAmountMM = amount * curRate;
      auto value = baseAmountMM.convert(file->baseCurrency().smallestAccountFraction());
      item->setData(column, ValueRole, QVariant::fromValue(value));
      adjustParentValue(item->parent(), column, value);
    } else {
      item->setData(column, ValueRole, QVariant::fromValue(item->data(column, ValueRole).value<MyMoneyMoney>() + amount));
      adjustParentValue(item->parent(), column, amount);
    }
  }

  void setAmount(QTreeWidgetItem* item, int column, const MyMoneyMoney& amount)
  {
    item->setData(column, AmountRole, QVariant::fromValue(amount));
    item->setTextAlignment(column, Qt::AlignRight | Qt::AlignVCenter);
  }

  void updateSummary(QTreeWidgetItem *item)
  {
    MyMoneyMoney amountMM;
    auto it_c = 1; // iterator for the columns of the listview
    const auto file = MyMoneyFile::instance();
    qint64 daysToBeginDay;

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
    for (summaryDate = QDate::currentDate().addDays(daysToBeginDay); summaryDate <= forecast.forecastEndDate(); summaryDate = summaryDate.addDays(forecast.accountsCycle()), ++it_c) {
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

  void updateDetailed(QTreeWidgetItem *item)
  {
    MyMoneyMoney vAmountMM;
    const auto file = MyMoneyFile::instance();

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

  void updateBudget(QTreeWidgetItem *item)
  {
    MyMoneySecurity currency;
    MyMoneyMoney tAmountMM;

    MyMoneyForecast forecast = item->data(0, ForecastRole).value<MyMoneyForecast>();

    const auto file = MyMoneyFile::instance();
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
      if (account.accountType() == eMyMoney::Account::Type::Expense)
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

  /**
   * Get the list of prices for an account
   * This is used later to create an instance of KMyMoneyAccountTreeForecastItem
   *
   */
//  QList<MyMoneyPrice> getAccountPrices(const MyMoneyAccount& acc)
//  {
//    const auto file = MyMoneyFile::instance();
//    QList<MyMoneyPrice> prices;
//    MyMoneySecurity security = file->baseCurrency();
//    try {
//      if (acc.isInvest()) {
//        security = file->security(acc.currencyId());
//        if (security.tradingCurrency() != file->baseCurrency().id()) {
//          MyMoneySecurity sec = file->security(security.tradingCurrency());
//          prices += file->price(sec.id(), file->baseCurrency().id());
//        }
//      } else if (acc.currencyId() != file->baseCurrency().id()) {
//        if (acc.currencyId() != file->baseCurrency().id()) {
//          security = file->security(acc.currencyId());
//          prices += file->price(acc.currencyId(), file->baseCurrency().id());
//        }
//      }

//    } catch (const MyMoneyException &e) {
//      qDebug() << Q_FUNC_INFO << " caught exception while adding " << acc.name() << "[" << acc.id() << "]: " << e.what();
//    }
//    return prices;
//  }

  Ui::KForecastView *ui;

  bool m_needReload[MaxViewTabs];

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;

  QTreeWidgetItem* m_totalItem;
  QTreeWidgetItem* m_assetItem;
  QTreeWidgetItem* m_liabilityItem;
  QTreeWidgetItem* m_incomeItem;
  QTreeWidgetItem* m_expenseItem;

  QLayout* m_chartLayout;
  QWidget *m_forecastChart;
  QScopedPointer<FixedColumnTreeView> m_fixedColumnView;
  QMap<QString, QString> m_nameIdx;
};

#endif
