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
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneyprice.h>
#include <mymoneyforecast.h>
#include <mymoneyutils.h>

#include "ui_kforecastviewdecl.h"
#include "kreportchartview.h"

using namespace reports;

/**
  * @author Alvaro Soliverez
  *
  * This class implements the forecast 'view'.
  */
class KForecastView : public QWidget, public Ui::KForecastViewDecl
{
  Q_OBJECT

public:
  enum EForecastViewType { eSummary = 0, eDetailed, eAdvanced, eBudget, eUndefined };

  KForecastView(QWidget *parent = 0);
  virtual ~KForecastView();

  void showEvent(QShowEvent* event);

public slots:
  void slotLoadForecast(void);
  void slotManualForecast(void);

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
  void loadListView(void);

  /**
   * This method loads the summary view
   */
  void loadSummaryView(void);

  /**
   * This method loads the advanced view
   */
  void loadAdvancedView(void);

  /**
   * This method loads the budget view
   */
  void loadBudgetView(void);

  /**
   * This method loads the budget view
   */
  void loadChartView(void);

  /**
   * This method loads the settings from user configuration
   */
  void loadForecastSettings(void);

protected slots:
  void slotTabChanged(QWidget*);

  /**
   * Get the list of prices for an account
   * This is used later to create an instance of KMyMoneyAccountTreeForecastItem
   *
   */
  QList<MyMoneyPrice> getAccountPrices(const MyMoneyAccount& acc);

private slots:
  void itemExpanded(QTreeWidgetItem *item);
  void itemCollapsed(QTreeWidgetItem *item);

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

  bool m_needReload[MaxViewTabs];

  QTreeWidgetItem* m_totalItem;
  QTreeWidgetItem* m_assetItem;
  QTreeWidgetItem* m_liabilityItem;
  QTreeWidgetItem* m_incomeItem;
  QTreeWidgetItem* m_expenseItem;

  QLayout* m_chartLayout;
  KReportChartView* m_forecastChart;

};

#endif
