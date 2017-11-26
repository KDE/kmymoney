/***************************************************************************
                             kforecastview.h
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

#ifndef KFORECASTVIEW_H
#define KFORECASTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneymoney.h"
#include "mymoneyforecast.h"
#include "mymoneyprice.h"

#include "ui_kforecastviewdecl.h"

namespace reports { class KReportChartView; }

class FixedColumnTreeView;
class MyMoneyAccount;
class MyMoneySecurity;
class MyMoneyForecast;
class MyMoneyPrice;
/**
  * @author Alvaro Soliverez
  *
  * This class implements the forecast 'view'.
  */
class KForecastView : public QWidget, private Ui::KForecastViewDecl
{
  Q_OBJECT

public:
  enum EForecastViewType { eSummary = 0, eDetailed, eAdvanced, eBudget, eUndefined };

  explicit KForecastView(QWidget *parent = 0);
  virtual ~KForecastView();

  void setDefaultFocus();

  void showEvent(QShowEvent* event);

public slots:
  void slotLoadForecast();
  void slotManualForecast();

protected:
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

  QMap<QString, QString> m_nameIdx;


  /**
    * This method loads the forecast view.
    */
  void loadForecast(ForecastViewTab tab);

  /**
    * This method loads the detailed view
    */
  void loadListView();

  /**
   * This method loads the summary view
   */
  void loadSummaryView();

  /**
   * This method loads the advanced view
   */
  void loadAdvancedView();

  /**
   * This method loads the budget view
   */
  void loadBudgetView();

  /**
   * This method loads the budget view
   */
  void loadChartView();

  /**
   * This method loads the settings from user configuration
   */
  void loadForecastSettings();

protected slots:
  void slotTabChanged(int index);

  /**
   * Get the list of prices for an account
   * This is used later to create an instance of KMyMoneyAccountTreeForecastItem
   *
   */
  QList<MyMoneyPrice> getAccountPrices(const MyMoneyAccount& acc);

private slots:
  void itemExpanded(QTreeWidgetItem *item);
  void itemCollapsed(QTreeWidgetItem *item);

signals:
  /**
    * This signal is emitted whenever the view is about to be shown.
    */
  void aboutToShow();

private:
  void addAssetLiabilityRows(const MyMoneyForecast& forecast);
  void addIncomeExpenseRows(const MyMoneyForecast& forecast);
  void addTotalRow(QTreeWidget* forecastList, const MyMoneyForecast& forecast);
  bool includeAccount(MyMoneyForecast& forecast, const MyMoneyAccount& acc);
  void loadAccounts(MyMoneyForecast& forecast, const MyMoneyAccount& account, QTreeWidgetItem* parentItem, int forecastType);

  void adjustHeadersAndResizeToContents(QTreeWidget *widget);

  void updateSummary(QTreeWidgetItem *item);
  void updateDetailed(QTreeWidgetItem *item);
  void updateBudget(QTreeWidgetItem *item);

  /**
    * Sets the whole item to be shown with negative colors
    */
  void setNegative(QTreeWidgetItem *item, bool isNegative);
  void showAmount(QTreeWidgetItem *item, int column, const MyMoneyMoney &amount, const MyMoneySecurity &security);
  void adjustParentValue(QTreeWidgetItem *item, int column, const MyMoneyMoney& value);
  void setValue(QTreeWidgetItem *item, int column, const MyMoneyMoney &amount, const QDate &forecastDate);
  void setAmount(QTreeWidgetItem *item, int column, const MyMoneyMoney &amount);

  /** Initializes page and sets its load status to initialized
   */
  void init();

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
  reports::KReportChartView* m_forecastChart;
  QScopedPointer<FixedColumnTreeView> m_fixedColumnView;
};

#endif
