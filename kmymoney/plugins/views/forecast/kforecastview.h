/*
 * Copyright (C) 2007      Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright (C) 2017      Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright (C) 2020      Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KFORECASTVIEW_H
#define KFORECASTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

class KXMLGUIClient;

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

  void createActions(KXMLGUIClient* guiClient);
  void removeActions();

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
