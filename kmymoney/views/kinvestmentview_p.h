/*
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINVESTMENTVIEW_P_H
#define KINVESTMENTVIEW_P_H

#include "kinvestmentview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KSharedConfig>
#include <KExtraColumnsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinvestmentview.h"
#include "kmymoneyviewbase_p.h"

#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyaccount.h"
#include "kmymoneysettings.h"
#include "kmymoneyaccountcombo.h"
#include "accountsmodel.h"
#include "equitiesmodel.h"
#include "securitiesmodel.h"
#include "icons.h"
#include "mymoneyenums.h"
#include "columnselector.h"

using namespace Icons;

namespace eView {
namespace Investment {
enum Tab { Equities = 0, Securities };
}
}

class KInvestmentViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KInvestmentView)

public:
    explicit KInvestmentViewPrivate(KInvestmentView *qq)
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
        m_equitiesProxyModel->setSourceModel(extraColumnModel);
        m_equitiesProxyModel->sort(AccountsModel::Column::AccountName);
        m_equitiesProxyModel->setSortRole(Qt::EditRole);

        ui->m_equitiesTree->setModel(m_equitiesProxyModel);

        QVector<int> equityColumns( {
            extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Symbol),
            extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Value),
            extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Quantity),
            extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Price),
        });

        m_equityColumnSelector = new ColumnSelector(ui->m_equitiesTree,
                QStringLiteral("KInvestmentView_Equities"),
                extraColumnModel->proxyColumnForExtraColumn(EquitiesModel::Column::Symbol)-1,
                equityColumns);
        m_equityColumnSelector->setModel(m_equitiesProxyModel);

        m_equityColumnSelector->setAlwaysVisible(QVector<int>({ AccountsModel::Column::AccountName }));

        QVector<int> columns;
        columns = m_equityColumnSelector->columns();

        int colIdx;
        for (auto col : equityColumns) {
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

    void loadAccount(const QString& id)
    {
        Q_Q(KInvestmentView);
        auto baseModel = MyMoneyFile::instance()->accountsModel();
        auto baseIdx = baseModel->indexById(id);
        QModelIndex idx;

        m_selections.clearSelections();
        emit q->requestSelectionChange(m_selections);

        m_equitiesProxyModel->setHideAllEntries(true);
        m_idInvAcc.clear();
        if (baseIdx.isValid()) {
            if (baseIdx.data(eMyMoney::Model::AccountTypeRole).value<eMyMoney::Account::Type>() == eMyMoney::Account::Type::Investment) {
                m_equitiesProxyModel->setHideAllEntries(false);
                idx = baseModel->mapFromBaseSource(m_equitiesProxyModel, baseIdx);
                m_idInvAcc = id;
            } else {
                idx = QModelIndex();
            }
        }
        ui->m_equitiesTree->setRootIndex(idx);

        if (m_equitiesProxyModel->rowCount(idx) > 0) {
            idx = m_equitiesProxyModel->index(0, 0, idx);
            ui->m_equitiesTree->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            ui->m_equitiesTree->setCurrentIndex(idx);
        }

        if (m_securitiesProxyModel->rowCount(QModelIndex()) > 0) {
            idx = m_securitiesProxyModel->index(0, 0);
            ui->m_securitiesTree->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
            ui->m_securitiesTree->setCurrentIndex(idx);
        }
    }

    /**
      * This slot is used to programatically preselect default account in investment view
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

    Ui::KInvestmentView *ui;
    QString             m_idInvAcc;

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
};

#endif
