/*
 * Copyright 2002-2004  Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2003-2019  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2004-2005  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2009-2010  Alvaro Soliverez <asoliverez@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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



#include "kinvestmentview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QBitArray>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "kequitypriceupdatedlg.h"
#include "kcurrencycalculator.h"
#include "knewinvestmentwizard.h"
#include "kmymoneyutils.h"
#include "menuenums.h"
#include "storageenums.h"

using namespace Icons;

KInvestmentView::KInvestmentView(QWidget *parent) :
  KMyMoneyViewBase(*new KInvestmentViewPrivate(this), parent)
{
  connect(pActions[eMenu::Action::NewInvestment],       &QAction::triggered, this, &KInvestmentView::slotNewInvestment);
  connect(pActions[eMenu::Action::EditInvestment],      &QAction::triggered, this, &KInvestmentView::slotEditInvestment);
  connect(pActions[eMenu::Action::DeleteInvestment],    &QAction::triggered, this, &KInvestmentView::slotDeleteInvestment);
  connect(pActions[eMenu::Action::UpdatePriceOnline],   &QAction::triggered, this, &KInvestmentView::slotUpdatePriceOnline);
  connect(pActions[eMenu::Action::UpdatePriceManually], &QAction::triggered, this, &KInvestmentView::slotUpdatePriceManually);
}

KInvestmentView::~KInvestmentView()
{
}

void KInvestmentView::setDefaultFocus()
{
  Q_D(KInvestmentView);
  auto tab = static_cast<eView::Investment::Tab>(d->ui->m_tab->currentIndex());

  switch (tab) {
  case eView::Investment::Tab::Equities:
    QMetaObject::invokeMethod(d->ui->m_equitiesTree, "setFocus");
    break;
  case eView::Investment::Tab::Securities:
    QMetaObject::invokeMethod(d->ui->m_securitiesTree, "setFocus");
    break;
  }
}

void KInvestmentView::executeCustomAction(eView::Action action)
{
  switch(action) {
    case eView::Action::Refresh:
      refresh();
      break;

    case eView::Action::SetDefaultFocus:
      setDefaultFocus();
      break;

    default:
      break;
  }
}

void KInvestmentView::refresh()
{
  Q_D(KInvestmentView);
  d->m_needReload[eView::Investment::Tab::Equities] = d->m_needReload[eView::Investment::Tab::Securities] = true;
  if (isVisible())
    slotLoadTab(d->ui->m_tab->currentIndex());
}

void KInvestmentView::showEvent(QShowEvent* event)
{
  Q_D(KInvestmentView);
  if (d->m_needLoad)
    d->init();

  emit customActionRequested(View::Investments, eView::Action::AboutToShow);

  d->m_needReload[eView::Investment::Tab::Equities] = true;  // ensure tree view will be reloaded after selecting account in ledger view
  if (d->m_needReload[eView::Investment::Tab::Equities] == true ||
      d->m_needReload[eView::Investment::Tab::Securities] == true)
    refresh();

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KInvestmentView::updateActions(const MyMoneyObject& obj)
{
  Q_D(KInvestmentView);
  if (typeid(obj) != typeid(MyMoneyAccount) &&
      (obj.id().isEmpty() && d->m_currentEquity.id().isEmpty())) // do not disable actions that were already disabled)))
    return;

  const auto& acc = static_cast<const MyMoneyAccount&>(obj);

  const auto file = MyMoneyFile::instance();
  auto b = acc.accountType() == eMyMoney::Account::Type::Investment ? true : false;
  if (isVisible() && !d->m_idInvAcc.isEmpty())
    b = true;

  pActions[eMenu::Action::NewInvestment]->setEnabled(b);

  b = acc.isInvest() ? true : false;
  pActions[eMenu::Action::EditInvestment]->setEnabled(b);
  pActions[eMenu::Action::DeleteInvestment]->setEnabled(b && !file->isReferenced(acc));
  pActions[eMenu::Action::UpdatePriceManually]->setEnabled(b);
  pActions[eMenu::Action::UpdatePriceOnline]->setEnabled(b && !file->security(acc.currencyId()).value("kmm-online-source").isEmpty());

  switch (acc.accountType()) {
    case eMyMoney::Account::Type::Investment:
    case eMyMoney::Account::Type::Stock:
      d->m_currentEquity = acc;
      break;
    default:
      d->m_currentEquity = MyMoneyAccount();
      break;
  }
}

void KInvestmentView::slotLoadTab(int index)
{
  Q_D(KInvestmentView);
  auto tab = static_cast<eView::Investment::Tab>(index);
  if (d->m_needReload[tab]) {
    switch (tab) {
      case eView::Investment::Tab::Equities:
        d->loadInvestmentTab();
        break;
      case eView::Investment::Tab::Securities:
        d->loadSecuritiesTab();
        break;
    }
    d->m_needReload[tab] = false;
  }
}

void KInvestmentView::slotEquitySelected(const QModelIndex &current, const QModelIndex &previous)
{
  Q_D(KInvestmentView);
  Q_UNUSED(current);
  Q_UNUSED(previous);

  const auto equ = d->currentEquity();
  updateActions(equ);

  emit selectByObject(equ, eView::Intent::None);
}

void KInvestmentView::slotSecuritySelected(const QModelIndex &current, const QModelIndex &previous)
{
  Q_D(KInvestmentView);
  Q_UNUSED(current);
  Q_UNUSED(previous);
  const auto sec = d->currentSecurity();
  if (!sec.id().isEmpty()) {
    QBitArray skip((int)eStorage::Reference::Count);
    skip.fill(false);
    skip.setBit((int)eStorage::Reference::Price);
    d->ui->m_editSecurityButton->setEnabled(true);
    d->ui->m_deleteSecurityButton->setEnabled(!MyMoneyFile::instance()->isReferenced(sec, skip));
  } else {
    d->ui->m_editSecurityButton->setEnabled(false);
    d->ui->m_deleteSecurityButton->setEnabled(false);
  }
}

void KInvestmentView::slotSelectAccount(const QString &id)
{
  Q_D(KInvestmentView);
  if (!id.isEmpty()) {
    d->m_idInvAcc = id;
    if (isVisible()) {
      d->ui->m_accountComboBox->setSelected(id);
      slotSecuritySelected(QModelIndex(), QModelIndex());
      slotEquitySelected(QModelIndex(), QModelIndex());
    }
  }
}

void KInvestmentView::slotSelectAccount(const MyMoneyObject &obj)
{
  if (typeid(obj) != typeid(MyMoneyAccount))
     return;
  const auto acc = dynamic_cast<const MyMoneyAccount &>(obj);

  if (acc.accountType() == eMyMoney::Account::Type::Investment)
    slotSelectAccount(acc.id());
}

void KInvestmentView::slotLoadAccount(const QString &id)
{
  Q_D(KInvestmentView);
  MyMoneyAccount acc;

  auto baseModel = MyMoneyFile::instance()->accountsModel();
  auto baseIdx = baseModel->indexById(id);
  QModelIndex idx;

  d->m_equitiesProxyModel->setHideAllEntries(true);
  if (baseIdx.isValid()) {
    acc = MyMoneyFile::instance()->accountsModel()->itemByIndex(baseIdx);
    if (acc.accountType() == eMyMoney::Account::Type::Investment) {
      d->m_equitiesProxyModel->setHideAllEntries(false);
      idx = baseModel->mapFromBaseSource(d->m_equitiesProxyModel, baseIdx);
      d->m_idInvAcc = id;
      if (isVisible())
        emit selectByObject(acc, eView::Intent::SynchronizeAccountInLedgersView);
    } else {
      idx = QModelIndex();
    }
  }
  d->ui->m_equitiesTree->setRootIndex(idx);

  updateActions(acc);
}

void KInvestmentView::slotInvestmentMenuRequested(const QPoint&)
{
  Q_D(KInvestmentView);
  MyMoneyAccount acc;
  auto treeItem = d->ui->m_equitiesTree->currentIndex();
  if (treeItem.isValid()) {
    auto idx = AccountsModel::mapToBaseSource(treeItem);
    acc = MyMoneyFile::instance()->accountsModel()->itemByIndex(idx);
  }
  slotShowInvestmentMenu(acc);
}

void KInvestmentView::slotShowInvestmentMenu(const MyMoneyAccount& acc)
{
  Q_UNUSED(acc);
  pMenus[eMenu::Menu::Investment]->exec(QCursor::pos());
}

void KInvestmentView::slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent)
{
  switch(intent) {
    case eView::Intent::UpdateActions:
      updateActions(obj);
      break;

    case eView::Intent::SynchronizeAccountInInvestmentView:
      if (KMyMoneySettings::syncLedgerInvestment())
        slotSelectAccount(obj);
      break;

    case eView::Intent::OpenContextMenu:
      slotShowInvestmentMenu(static_cast<const MyMoneyAccount&>(obj));
      break;

    default:
      break;
  }
}

void KInvestmentView::slotNewInvestment()
{
  Q_D(KInvestmentView);
  if (!isVisible())
    KNewInvestmentWizard::newInvestment(d->m_currentEquity);
  else
    KNewInvestmentWizard::newInvestment(MyMoneyFile::instance()->account(d->m_idInvAcc));
}

void KInvestmentView::slotEditInvestment()
{
  Q_D(KInvestmentView);
  KNewInvestmentWizard::editInvestment(d->m_currentEquity);
}

void KInvestmentView::slotDeleteInvestment()
{
  Q_D(KInvestmentView);
  if (KMessageBox::questionYesNo(this,
                                 i18n("<p>Do you really want to delete the investment <b>%1</b>?</p>", d->m_currentEquity.name()),
                                 i18n("Delete investment"),
                                 KStandardGuiItem::yes(), KStandardGuiItem::no(),
                                 "DeleteInvestment") == KMessageBox::Yes) {
    auto file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    try {
//      d->m_selectedAccount = MyMoneyAccount(); // CAUTION: deleting equity from investments view needs this, if ID of the equity to be deleted is the smallest from all
      file->removeAccount(d->m_currentEquity);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::information(this, i18n("Unable to delete investment: %1", QString::fromLatin1(e.what())));
    }
  } else {
    // we should not keep the 'no' setting because that can confuse people like
    // I have seen in some usability tests. So we just delete it right away.
    KSharedConfigPtr kconfig = KSharedConfig::openConfig();
    if (kconfig) {
      kconfig->group(QLatin1String("Notification Messages")).deleteEntry(QLatin1String("DeleteInvestment"));
    }
  }
}

void KInvestmentView::slotUpdatePriceOnline()
{
  Q_D(KInvestmentView);
  if (!d->m_currentEquity.id().isEmpty()) {
    QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(0, d->m_currentEquity.currencyId());
    if ((dlg->exec() == QDialog::Accepted) && (dlg != nullptr))
      dlg->storePrices();
    delete dlg;
  }
}

void KInvestmentView::slotUpdatePriceManually()
{
  Q_D(KInvestmentView);
  if (!d->m_currentEquity.id().isEmpty()) {
    try {
      auto security = MyMoneyFile::instance()->security(d->m_currentEquity.currencyId());
      auto currency = MyMoneyFile::instance()->security(security.tradingCurrency());
      const auto& price = MyMoneyFile::instance()->price(security.id(), currency.id());

      QPointer<KCurrencyCalculator> calc =
        new KCurrencyCalculator(security, currency, MyMoneyMoney::ONE,
                                price.rate(currency.id()), price.date(),
                                MyMoneyMoney::precToDenom(security.pricePrecision()));
      calc->setupPriceEditor();

      // The dialog takes care of adding the price if necessary
      calc->exec();
      delete calc;
    } catch (const MyMoneyException &e) {
      qDebug("Error in price update: %s", e.what());
    }
  }
}

void KInvestmentView::slotEditSecurity()
{
  Q_D(KInvestmentView);
  auto sec = d->currentSecurity();

  if (!sec.id().isEmpty()) {
    QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(sec, this);
    dlg->setObjectName("KNewInvestmentWizard");
    if (dlg->exec() == QDialog::Accepted)
      dlg->createObjects(QString());
    delete dlg;
  }
}

void KInvestmentView::slotDeleteSecurity()
{
  Q_D(KInvestmentView);
  auto sec = d->currentSecurity();
  if (!sec.id().isEmpty())
    KMyMoneyUtils::deleteSecurity(sec, this);
}
