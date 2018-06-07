/*
 * Copyright 2002-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KTRANSACTIONFILTER_H
#define KTRANSACTIONFILTER_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QTreeWidgetItem;
class MyMoneyTransactionFilter;
class KMyMoneyAccountSelector;
class MyMoneyReport;
class DateRangeDlg;

/**
  * @author Thomas Baumgart
  * @author Łukasz Wojniłowicz
  */

class KTransactionFilterPrivate;
class KMM_WIDGETS_EXPORT KTransactionFilter : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KTransactionFilter)

public:
  /**
   @param withEquityAccounts set to false to hide equity accounts in account page
  */
  explicit KTransactionFilter(QWidget *parent = nullptr, bool withEquityAccounts = false, bool withDataTab = true);
  ~KTransactionFilter();

  MyMoneyTransactionFilter setupFilter();
  void resetFilter(MyMoneyReport& rep);
  KMyMoneyAccountSelector* categoriesView();
  DateRangeDlg* dateRange();

  /**
    * This slot opens the detailed help page in khelpcenter. The
    * anchor for the information is taken from m_helpAnchor.
    */
  void slotShowHelp();
  void slotReset();

private Q_SLOTS:
  void slotUpdateSelections();

  void slotAmountSelected();
  void slotAmountRangeSelected();

  void slotSelectAllPayees();
  void slotDeselectAllPayees();

  void slotSelectAllTags();
  void slotDeselectAllTags();

  void slotNrSelected();
  void slotNrRangeSelected();

Q_SIGNALS:

  /**
    * This signal is sent out when a selection has been made. It is
    * used to control the state of the Search button.
    * The Search button is only active when a selection has been made
    * (i.e. notEmpty == true)
    */
  void selectionNotEmpty(bool);

private:
  Q_DECLARE_PRIVATE(KTransactionFilter)
  KTransactionFilterPrivate * const d_ptr;
};

#endif
