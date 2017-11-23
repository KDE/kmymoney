/***************************************************************************
                             kcurrencypage.cpp
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kcurrencypage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>
#include <QLabel>
#include <QList>
#include <QTreeView>
#include <QTreeWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "icons/icons.h"
#include "knewuserwizard.h"
#include "knewuserwizard_p.h"
#include "kaccountpage.h"
#include "kaccountpage_p.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "ui_currency.h"
#include "ui_kaccountpage.h"
#include "wizardpage.h"

using namespace Icons;

namespace NewUserWizard
{
  class CurrencyPagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(CurrencyPagePrivate)

  public:
    CurrencyPagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent)
    {
    }
  };

  CurrencyPage::CurrencyPage(Wizard* wizard) :
    Currency(wizard),
    WizardPage<Wizard>(*new CurrencyPagePrivate(wizard), stepCount++, this, wizard)
  {
    QTreeWidgetItem *first = 0;

    QList<MyMoneySecurity> list = MyMoneyFile::instance()->availableCurrencyList();
    QList<MyMoneySecurity>::const_iterator it;

    QString localCurrency(QLocale().currencySymbol(QLocale::CurrencyIsoCode));
    QString baseCurrency = MyMoneyFile::instance()->baseCurrency().id();


    ui->m_currencyList->clear();
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
      QTreeWidgetItem* p = insertCurrency(*it);
      if ((*it).id() == baseCurrency) {
        first = p;
        QIcon icon = QIcon::fromTheme(g_Icons[Icon::ViewBankAccount]);
        p->setIcon(0, icon);
      } else {
        p->setIcon(0, QIcon());
      }
      if (!first && (*it).id() == localCurrency)
        first = p;
    }

    QTreeWidgetItemIterator itemsIt = QTreeWidgetItemIterator(ui->m_currencyList, QTreeWidgetItemIterator::All);

    if (first == 0)
      first = *itemsIt;
    if (first != 0) {
      ui->m_currencyList->setCurrentItem(first);
      ui->m_currencyList->setItemSelected(first, true);
      ui->m_currencyList->scrollToItem(first, QTreeView::PositionAtTop);
    }
  }

  CurrencyPage::~CurrencyPage()
  {
  }

  void CurrencyPage::enterPage()
  {
    ui->m_currencyList->setFocus();
  }

  KMyMoneyWizardPage* CurrencyPage::nextPage() const
  {
    Q_D(const CurrencyPage);
    QString selCur = selectedCurrency();
    QList<MyMoneySecurity> currencies = MyMoneyFile::instance()->availableCurrencyList();
    foreach (auto currency, currencies) {
      if (selCur == currency.id()) {
        d->m_wizard->d_func()->m_baseCurrency = currency;
        break;
      }
    }
    d->m_wizard->d_func()->m_accountPage->d_func()->ui->m_accountCurrencyLabel->setText(d->m_wizard->d_func()->m_baseCurrency.tradingSymbol());
    return d->m_wizard->d_func()->m_accountPage;
  }

}
