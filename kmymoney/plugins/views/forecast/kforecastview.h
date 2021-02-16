/*
    SPDX-FileCopyrightText: 2007 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KFORECASTVIEW_H
#define KFORECASTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"

class QTreeWidgetItem;

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
class KForecastViewPrivate;
class KForecastView : public KMyMoneyViewBase
{
  Q_OBJECT

public:
  explicit KForecastView(QWidget *parent = nullptr);
  ~KForecastView() override;

  void executeCustomAction(eView::Action action) override;
  void refresh();

protected:
  void showEvent(QShowEvent* event) override;

private:
  Q_DECLARE_PRIVATE(KForecastView)

private Q_SLOTS:
  void slotTabChanged(int index);
  void slotManualForecast();
  void itemExpanded(QTreeWidgetItem *item);
  void itemCollapsed(QTreeWidgetItem *item);
};

#endif
