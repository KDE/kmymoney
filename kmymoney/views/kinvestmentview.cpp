/*
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QBitArray>
#include <QIcon>
#include <QMenu>
#include <QTimer>

#include <QPainter>
#include <QStyledItemDelegate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KExtraColumnsProxyModel>
#include <KMessageBox>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinvestmentview.h"

#include "accountsmodel.h"
#include "columnselector.h"
#include "equitiesmodel.h"
#include "icons.h"
#include "kcurrencycalculator.h"
#include "kequitypriceupdatedlg.h"
#include "kinvestmentview.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "kmymoneyviewbase_p.h"
#include "knewinvestmentwizard.h"
#include "menuenums.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneysecurity.h"
#include "securitiesmodel.h"
#include "storageenums.h"

using namespace Icons;

namespace eView { namespace Investment {
enum Tab { Equities = 0, Securities };
}}

class KInvestmentViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KInvestmentView)

public:
    explicit KInvestmentViewPrivate(KInvestmentView* qq)
        : KMyMoneyViewBasePrivate(qq)
        , ui(new Ui::KInvestmentView)
        , m_idInvAcc(QString())
        , m_needLoad(true)
        , m_accountsProxyModel(nullptr)
        , m_equitiesProxyModel(nullptr)
        , m_securitiesProxyModel(nullptr)
        , m_securityColumnSelector(nullptr)
        , m_equityColumnSelector(nullptr)
    {
    }

    ~KInvestmentViewPrivate()
    {
        delete ui;
    }

    void init()
    {
        Q_Q(KInvestmentView);
        m_needLoad = false;
        ui->setupUi(q);

        // Equities tab
        m_accountsProxyModel = new AccountNamesFilterProxyModel(q);
        m_accountsProxyModel->setObjectName("m_accountsProxyModel");
        m_accountsProxyModel->addAccountType(eMyMoney::Account::Type::Investment);
        m_accountsProxyModel->setHideEquityAccounts(false);
        m_accountsProxyModel->setHideZeroBalancedEquityAccounts(false);
        m_accountsProxyModel->setHideZeroBalancedAccounts(KMyMoneySettings::hideZeroBalanceAccounts());
        m_accountsProxyModel->setSourceModel(MyMoneyFile::instance()->accountsModel());
        m_accountsProxyModel->sort(AccountsModel::Column::AccountName);
        ui->m_accountComboBox->setModel(m_accountsProxyModel);
        ui->m_accountComboBox->expandAll();

        auto extraColumnModel = new EquitiesModel(q);
        extraColumnModel->setObjectName("extraColumnModel");
        extraColumnModel->setSourceModel(MyMoneyFile::instance()->accountsModel());

        m_equitiesProxyModel = new AccountsProxyModel(q);
        m_equitiesProxyModel->setObjectName("m_equitiesProxyModel");
        m_equitiesProxyModel->clear();
        m_equitiesProxyModel->addAccountType(eMyMoney::Account::Type::Stock);
        m_equitiesProxyModel->setHideEquityAccounts(false);
        m_equitiesProxyModel->setHideAllEntries(true);
        m_equitiesProxyModel->setClosedSelectable(true);
        m_equitiesProxyModel->setSourceModel(extraColumnModel);
        m_equitiesProxyModel->sort(AccountsModel::Column::AccountName);
        m_equitiesProxyModel->setSortRole(Qt::EditRole);
        m_equitiesProxyModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());

        ui->m_equitiesTree->setModel(m_equitiesProxyModel);

        QVector<int> equityColumns({extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Symbol),
                                    extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Value),
                                    extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Quantity),
                                    extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Price),
                                    extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::LastPriceUpdate)});

        m_equityColumnSelector = new ColumnSelector(ui->m_equitiesTree,
                                                    QStringLiteral("KInvestmentView_Equities"),
                                                    extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Symbol) - 1,
                                                    equityColumns);
        m_equityColumnSelector->setModel(m_equitiesProxyModel);

        m_equityColumnSelector->setAlwaysVisible(QVector<int>({AccountsModel::Column::AccountName}));

        QVector<int> columns;
        columns = m_equityColumnSelector->columns();

        int colIdx;
        for (const auto& col : qAsConst(equityColumns)) {
            colIdx = columns.indexOf(col);
            if (colIdx != -1)
                columns.remove(colIdx);
        }
        colIdx = columns.indexOf(AccountsModel::Column::AccountName);
        if (colIdx != -1)
            columns.remove(colIdx);

        m_equityColumnSelector->setAlwaysHidden(columns);
        m_equityColumnSelector->setSelectable(equityColumns);

        // Securities tab
        m_securitiesProxyModel = new QSortFilterProxyModel(q);
        ui->m_securitiesTree->setModel(m_securitiesProxyModel);
        m_securitiesProxyModel->setSourceModel(MyMoneyFile::instance()->securitiesModel());
        m_securitiesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

        m_securityColumnSelector = new ColumnSelector(ui->m_securitiesTree, QStringLiteral("KInvestmentView_Securities"));
        m_securityColumnSelector->setModel(MyMoneyFile::instance()->securitiesModel());
        m_securityColumnSelector->setAlwaysVisible(QVector<int>({0}));
        m_securityColumnSelector->setSelectable(m_securityColumnSelector->columns());

        q->connect(ui->m_searchSecurities, &QLineEdit::textChanged, m_securitiesProxyModel, &QSortFilterProxyModel::setFilterFixedString);
    }

    /**
     * Use a copy of the account @a id here because we pass
     * m_idInvAcc as argument at one point which gets cleared
     * but we need it later on.
     */
    void loadAccount(QString id)
    {
        Q_Q(KInvestmentView);
        auto baseModel = MyMoneyFile::instance()->accountsModel();
        auto baseIdx = baseModel->indexById(id);
        const auto currentSelectedId = ui->m_equitiesTree->currentIndex().data(eMyMoney::Model::IdRole).toString();
        QModelIndex idx;

        m_selections.clearSelections();
        Q_EMIT q->requestSelectionChange(m_selections);

        m_equitiesProxyModel->setHideAllEntries(true);
        m_idInvAcc.clear();
        if (baseIdx.isValid()) {
            if (baseIdx.data(eMyMoney::Model::AccountTypeRole).value<eMyMoney::Account::Type>() == eMyMoney::Account::Type::Investment) {
                m_equitiesProxyModel->setHideAllEntries(false);
                idx = baseModel->mapFromBaseSource(m_equitiesProxyModel, baseIdx);
                m_idInvAcc = id;
            }
        }

        if (idx.isValid()) {
            ui->m_equitiesTree->setRootIndex(idx);
        } else {
            m_equitiesProxyModel->setHideAllEntries(true);
        }

        if (m_equitiesProxyModel->rowCount(idx) > 0) {
            int row(0);
            if (!currentSelectedId.isEmpty()) {
                const auto indexList = m_equitiesProxyModel->match(idx, eMyMoney::Model::IdRole, currentSelectedId, 1, Qt::MatchRecursive);
                if (!indexList.isEmpty()) {
                    row = indexList.first().row();
                }
            }
            idx = m_equitiesProxyModel->index(row, 0, idx);
            ui->m_equitiesTree->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            ui->m_equitiesTree->setCurrentIndex(idx);
        }

        if (m_securitiesProxyModel->rowCount(QModelIndex()) > 0) {
            idx = m_securitiesProxyModel->index(0, 0);
            ui->m_securitiesTree->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            ui->m_securitiesTree->setCurrentIndex(idx);
        }

        q->updateActions(m_selections);
    }

    /**
     * This slot is used to programmatically preselect default account in investment view
     */
    void selectDefaultInvestmentAccount()
    {
        if (m_accountsProxyModel->rowCount() > 0) {
            const auto indexes = m_accountsProxyModel->match(m_accountsProxyModel->index(0, 0),
                                                             eMyMoney::Model::AccountTypeRole,
                                                             QVariant::fromValue<eMyMoney::Account::Type>(eMyMoney::Account::Type::Investment),
                                                             1,
                                                             Qt::MatchRecursive);
            if (!indexes.isEmpty()) {
                ui->m_accountComboBox->setSelected(indexes.first().data(eMyMoney::Model::IdRole).toString());
            }
        }
    }

    /**
     * This slots returns security currently selected in tree view
     */
    MyMoneySecurity currentSecurity()
    {
        MyMoneySecurity sec;
        const auto securityIdx = ui->m_securitiesTree->currentIndex();
        if (securityIdx.isValid()) {
            auto mdlItem = m_securitiesProxyModel->index(securityIdx.row(), SecuritiesModel::Security, securityIdx.parent());
            sec = MyMoneyFile::instance()->security(mdlItem.data(eMyMoney::Model::IdRole).toString());
        }
        return sec;
    }

    /**
     * This slots returns equity currently selected in tree view
     */
    MyMoneyAccount currentEquity()
    {
        QModelIndex idx = MyMoneyFile::baseModel()->mapToBaseSource(ui->m_equitiesTree->currentIndex());
        return MyMoneyFile::instance()->accountsModel()->itemByIndex(idx);
    }

    Ui::KInvestmentView* ui;
    QString m_idInvAcc;

    /**
     * This member holds the load state of page
     */
    bool m_needLoad;

    AccountNamesFilterProxyModel* m_accountsProxyModel;
    AccountsProxyModel* m_equitiesProxyModel;
    QSortFilterProxyModel* m_securitiesProxyModel;
    ColumnSelector* m_securityColumnSelector;
    ColumnSelector* m_equityColumnSelector;
    SelectedObjects m_equitySelections;
    SelectedObjects m_securitySelections;
    SelectedObjects m_externalSelections;
};

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
        if (!d->m_needLoad) {
            // make sure to remove any account reference
            d->ui->m_accountComboBox->setSelected(QString());
        }
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
        connect(
            d->ui->m_tab,
            &QTabWidget::currentChanged,
            this,
            [&](int index) {
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
            },
            Qt::QueuedConnection);

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
            const auto indexes = d->m_accountsProxyModel->match(d->m_accountsProxyModel->index(0, 0),
                                                                eMyMoney::Model::AccountTypeRole,
                                                                static_cast<int>(eMyMoney::Account::Type::Investment),
                                                                -1,
                                                                Qt::MatchExactly | Qt::MatchRecursive);
            if (indexes.count()) {
                d->ui->m_accountComboBox->setSelected(QString());
            }
            d->ui->m_accountComboBox->setSelected(accountId);
        }
    }

    const auto tab = static_cast<eView::Investment::Tab>(d->ui->m_tab->currentIndex());

    switch (tab) {
    case eView::Investment::Tab::Equities:
        d->m_selections = d->m_equitySelections;
        d->m_selections.setSelection(SelectedObjects::Security, d->m_securitySelections.selection(SelectedObjects::Security));
        break;
    case eView::Investment::Tab::Securities:
        d->m_selections = d->m_securitySelections;
        break;
    }
    Q_EMIT requestSelectionChange(d->m_selections);
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
    if (KMessageBox::questionTwoActions(this,
                                        i18n("<p>Do you really want to delete the investment <b>%1</b>?</p>", d->currentEquity().name()),
                                        i18n("Delete investment"),
                                        KMMYesNo::yes(),
                                        KMMYesNo::no(),
                                        "DeleteInvestment")
        == KMessageBox::PrimaryAction) {
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
    KNewInvestmentWizard::editSecurity(d->currentSecurity());
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
