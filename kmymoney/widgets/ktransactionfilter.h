/*
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit KTransactionFilter(QWidget *parent = nullptr, bool withEquityAccounts = false, bool withInvestments = false, bool withDataTab = true);
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
