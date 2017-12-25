/***************************************************************************
                             kforecastview.h
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
  explicit KForecastView(QWidget *parent = 0);
  ~KForecastView() override;

  void setDefaultFocus() override;
  void refresh() override;

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
