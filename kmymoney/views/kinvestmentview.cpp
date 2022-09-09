/*
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kinvestmentview_p.h"

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
    connect(pActions[eMenu::Action::NewInvestment], &QAction::triggered, this, &KInvestmentView::slotNewInvestment);
    connect(pActions[eMenu::Action::EditInvestment], &QAction::triggered, this, &KInvestmentView::slotEditInvestment);
    connect(pActions[eMenu::Action::DeleteInvestment], &QAction::triggered, this, &KInvestmentView::slotDeleteInvestment);
    connect(pActions[eMenu::Action::UpdatePriceOnline], &QAction::triggered, this, &KInvestmentView::slotUpdatePriceOnline);
    connect(pActions[eMenu::Action::UpdatePriceManually], &QAction::triggered, this, &KInvestmentView::slotUpdatePriceManually);
    connect(pActions[eMenu::Action::EditSecurity], &QAction::triggered, this, &KInvestmentView::slotEditSecurity);
    connect(pActions[eMenu::Action::DeleteSecurity], &QAction::triggered, this, &KInvestmentView::slotDeleteSecurity);
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
        QMetaObject::invokeMethod(d->ui->m_equitiesTree, "setFocus", Qt::QueuedConnection);
        break;
    case eView::Investment::Tab::Securities:
        QMetaObject::invokeMethod(d->ui->m_securitiesTree, "setFocus", Qt::QueuedConnection);
        break;
    }
}

void KInvestmentView::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    Q_UNUSED(selections)
    Q_D(KInvestmentView);
    switch (action) {
    case eMenu::Action::FileNew:
        if (!d->m_needLoad) {
            d->ui->m_accountComboBox->expandAll();
            d->m_equitiesProxyModel->invalidate();
            d->m_securitiesProxyModel->invalidate();
            d->selectDefaultInvestmentAccount();
        }
        break;

    case eMenu::Action::FileClose:
        d->m_idInvAcc.clear();
        d->m_equitySelections.clearSelections();
        d->m_securitySelections.clearSelections();
        d->m_selections.clearSelections();
        break;

    default:
        break;
    }
}
void KInvestmentView::showEvent(QShowEvent* event)
{
    Q_D(KInvestmentView);
    if (d->m_needLoad) {
        d->init();

        connect(d->ui->m_equitiesTree, &QWidget::customContextMenuRequested, this, [&](const QPoint& pos) {
            Q_D(KInvestmentView);
            Q_EMIT requestCustomContextMenu(eMenu::Menu::Investment, d->ui->m_equitiesTree->viewport()->mapToGlobal(pos));
        });

        connect(d->ui->m_securitiesTree, &QWidget::customContextMenuRequested, this, [&](const QPoint& pos) {
            Q_D(KInvestmentView);
            Q_EMIT requestCustomContextMenu(eMenu::Menu::Security, d->ui->m_equitiesTree->viewport()->mapToGlobal(pos));
        });

        connect(d->ui->m_equitiesTree->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                [&](const QModelIndex& current, const QModelIndex& previous) {
                    Q_UNUSED(previous)
                    Q_D(KInvestmentView);
                    d->m_equitySelections.clearSelections(SelectedObjects::Account);
                    // when closing equities, current may still reference a row that
                    // is not valid any longer. For this reason, we set the row
                    // to the last row in the model
                    if (current.isValid()) {
                        const auto rows = current.model()->rowCount(current.parent());
                        auto idx = current;
                        if (idx.row() >= rows) {
                            idx = idx.model()->index(rows - 1, idx.column(), idx.parent());
                        }
                        if (idx.isValid()) {
                            d->m_equitySelections.setSelection(SelectedObjects::Account, idx.data(eMyMoney::Model::IdRole).toString());
                        }
                    } else {
                        // suppress display if no more equities are shown
                        d->m_equitiesProxyModel->setHideAllEntries(true);
                    }
                    if (d->ui->m_equitiesTree->isVisible()) {
                        d->m_selections = d->m_equitySelections;
                        Q_EMIT requestSelectionChange(d->m_selections);
                    }
                });

        connect(d->ui->m_securitiesTree->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                [&](const QModelIndex& current, const QModelIndex& previous) {
                    Q_UNUSED(previous)
                    Q_D(KInvestmentView);
                    d->m_securitySelections.setSelection(SelectedObjects::Security, current.data(eMyMoney::Model::IdRole).toString());
                    if (d->ui->m_securitiesTree->isVisible()) {
                        d->m_selections = d->m_securitySelections;
                        Q_EMIT requestSelectionChange(d->m_selections);
                    }
                });

        connect(d->ui->m_equitiesTree, &QTreeView::doubleClicked, this, &KInvestmentView::slotEditInvestment);

        // use a QueuedConnection here to suppress duplicate call (at least on Qt 5.12.7)
        connect(d->ui->m_tab, &QTabWidget::currentChanged, this, [&](int index) {
            Q_D(KInvestmentView);
            const auto tab = static_cast<eView::Investment::Tab>(index);

            switch (tab) {
            case eView::Investment::Tab::Equities:
                d->m_selections = d->m_equitySelections;
                break;
            case eView::Investment::Tab::Securities:
                d->m_selections = d->m_securitySelections;
                break;
            }
            Q_EMIT requestSelectionChange(d->m_selections);
        });

        connect(d->ui->m_accountComboBox, &KMyMoneyAccountCombo::accountSelected, this, [&](const QString& accountId) {
            Q_D(KInvestmentView);
            d->loadAccount(accountId);
        });

        d->selectDefaultInvestmentAccount();
    }

    // don't forget base class implementation
    QWidget::showEvent(event);

    // check if the last selected account was an investment account.
    // if so, then select it in this view as well. otherwise, we
    // leave the selection as is
    const auto accountId = d->m_externalSelections.firstSelection(SelectedObjects::Account);
    if (!accountId.isEmpty()) {
        const auto account = MyMoneyFile::instance()->account(accountId);
        if (account.accountType() == eMyMoney::Account::Type::Investment) {
            d->ui->m_accountComboBox->setSelected(accountId);
        }
    }
}

void KInvestmentView::updateActions(const SelectedObjects& selections)
{
    Q_D(KInvestmentView);
    const auto equityId = selections.firstSelection(SelectedObjects::Account);
    const auto securityId = selections.firstSelection(SelectedObjects::Security);
    const auto file = MyMoneyFile::instance();

    pActions[eMenu::Action::NewInvestment]->setEnabled(false);
    pActions[eMenu::Action::EditInvestment]->setEnabled(false);
    pActions[eMenu::Action::DeleteInvestment]->setEnabled(false);
    pActions[eMenu::Action::UpdatePriceManually]->setEnabled(false);
    pActions[eMenu::Action::UpdatePriceOnline]->setEnabled(false);

    pActions[eMenu::Action::EditSecurity]->setEnabled(false);
    pActions[eMenu::Action::DeleteSecurity]->setEnabled(false);

    // check that the selected account (combobox) is an investment account
    auto idx = file->accountsModel()->indexById(d->m_idInvAcc);
    if (idx.data(eMyMoney::Model::AccountTypeRole).value<eMyMoney::Account::Type>() == eMyMoney::Account::Type::Investment) {
        pActions[eMenu::Action::NewInvestment]->setEnabled(true);
    }

    if (!equityId.isEmpty()) {
        idx = file->accountsModel()->indexById(equityId);
        if (idx.data(eMyMoney::Model::AccountIsInvestRole).toBool()) {
            pActions[eMenu::Action::EditInvestment]->setEnabled(true);
            pActions[eMenu::Action::UpdatePriceManually]->setEnabled(true);
            pActions[eMenu::Action::DeleteInvestment]->setDisabled(file->isReferenced(equityId));
            const auto secId = idx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
            const auto sec = file->securitiesModel()->itemById(secId);
            pActions[eMenu::Action::UpdatePriceOnline]->setDisabled(sec.value("kmm-online-source").isEmpty());
        }
    }
    if (!securityId.isEmpty()) {
        QBitArray skip((int)eStorage::Reference::Count);
        skip.fill(false);
        skip.setBit((int)eStorage::Reference::Price);
        pActions[eMenu::Action::EditSecurity]->setEnabled(true);
        pActions[eMenu::Action::DeleteSecurity]->setDisabled(file->isReferenced(securityId, skip));
    }

    d->m_externalSelections = selections;
}

void KInvestmentView::slotNewInvestment()
{
    Q_D(KInvestmentView);
    if (!isVisible())
        KNewInvestmentWizard::newInvestment(d->currentEquity());
    else
        KNewInvestmentWizard::newInvestment(MyMoneyFile::instance()->account(d->m_idInvAcc));
}

void KInvestmentView::slotEditInvestment()
{
    Q_D(KInvestmentView);
    KNewInvestmentWizard::editInvestment(d->currentEquity());
}

void KInvestmentView::slotDeleteInvestment()
{
    Q_D(KInvestmentView);
    if (KMessageBox::questionYesNo(this,
                                   i18n("<p>Do you really want to delete the investment <b>%1</b>?</p>", d->currentEquity().name()),
                                   i18n("Delete investment"),
                                   KStandardGuiItem::yes(),
                                   KStandardGuiItem::no(),
                                   "DeleteInvestment")
        == KMessageBox::Yes) {
        auto file = MyMoneyFile::instance();
        MyMoneyFileTransaction ft;
        try {
            file->removeAccount(d->currentEquity());
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
    if (!d->currentEquity().id().isEmpty()) {
        QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(0, d->currentEquity().currencyId());
        if ((dlg->exec() == QDialog::Accepted) && (dlg != nullptr))
            dlg->storePrices();
        delete dlg;
    }
}

void KInvestmentView::slotUpdatePriceManually()
{
    Q_D(KInvestmentView);
    if (!d->currentEquity().id().isEmpty()) {
        try {
            auto security = MyMoneyFile::instance()->security(d->currentEquity().currencyId());
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

void KInvestmentView::slotSettingsChanged()
{
    Q_D(KInvestmentView);
    if (d->m_needLoad) {
        return;
    }

    const bool showAllAccounts = KMyMoneySettings::showAllAccounts();
    if (d->m_equitiesProxyModel->hideClosedAccounts() == showAllAccounts) {
        d->m_equitiesProxyModel->setHideClosedAccounts(!showAllAccounts);
        d->loadAccount(d->m_idInvAcc);
    }
}
