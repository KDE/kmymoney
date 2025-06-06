/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "institutionsmodel.h"
#include "accountsmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QColor>
#include <QDebug>
#include <QFont>
#include <QIcon>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "icons.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"

struct InstitutionsModel::Private
{
    Q_DECLARE_PUBLIC(InstitutionsModel)

    Private(InstitutionsModel* qq, QObject* parent)
        : q_ptr(qq)
        , parentObject(parent)
        , accountsModel(nullptr)
    {
    }

    void loadAccounts(const QModelIndex& idx, const QList<QString>& idList)
    {
        Q_Q(InstitutionsModel);
        const int accounts = idList.count();
        q->insertRows(0, accounts, idx);
        // we create institution subentries here with the id of the account. These will never
        // be used as institutions and also not found by indexById for a different m_leadIn
        // The are only used in data() to proxy data from the accountsModel.
        for (int row = 0; row < accounts; ++row) {
            const auto subIdx = q->index(row, 0, idx);
            MyMoneyInstitution account(idList.at(row), MyMoneyInstitution());
            static_cast<TreeItem<MyMoneyInstitution>*>(subIdx.internalPointer())->dataRef() = account;
        }
    }

    MyMoneyMoney institutionValue(const QModelIndex& idx)
    {
        Q_Q(InstitutionsModel);
        const auto rows = q->rowCount(idx);
        MyMoneyMoney value;
        for (int row = 0; row < rows; ++row) {
            const auto subIdx(q->index(row, 0, idx));
            value += subIdx.data(eMyMoney::Model::AccountValueRole).value<MyMoneyMoney>();
        }
        return value;
    }

    InstitutionsModel*  q_ptr;
    QObject*            parentObject;
    AccountsModel*      accountsModel;
    QColor              positiveScheme;
    QColor              negativeScheme;

};

InstitutionsModel::InstitutionsModel(AccountsModel* accountsModel, QObject* parent, QUndoStack* undoStack)
    : MyMoneyModel<MyMoneyInstitution>(parent, QStringLiteral("I"), InstitutionsModel::ID_SIZE, undoStack)
    , d(new Private(this, parent))
{
    d->accountsModel = accountsModel;
    setObjectName(QLatin1String("InstitutionsModel"));
}

InstitutionsModel::~InstitutionsModel()
{
}

int InstitutionsModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return AccountsModel::Column::MaxColumns;
}

QVariant InstitutionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (Q_UNLIKELY(!d->accountsModel))
        return QVariant();
    return d->accountsModel->headerData(section, orientation, role);
}

QVariant InstitutionsModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return QVariant();
    if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
        return QVariant();

    const MyMoneyInstitution& institution = static_cast<TreeItem<MyMoneyInstitution>*>(idx.internalPointer())->constDataRef();

    // check for a sub-entry which is actually a proxy to the corresponding account
    if (idx.parent().isValid()) {
        const auto accountIdx = d->accountsModel->indexById(institution.id());
        const auto subIdx = d->accountsModel->index(accountIdx.row(), idx.column(), accountIdx.parent());
        return d->accountsModel->data(subIdx, role);
    }

    switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(idx.column()) {
        case AccountsModel::Column::AccountName:
            // make sure to never return any displayable text for the dummy entry
            return institution.name();

        case AccountsModel::Column::TotalPostedValue:
        {
            const auto baseCurrency = MyMoneyFile::instance()->baseCurrency();
            return d->institutionValue(idx).formatMoney(baseCurrency.tradingSymbol(), MyMoneyMoney::denomToPrec(baseCurrency.smallestAccountFraction()));
        }

        case AccountsModel::Column::BankCode:
            return institution.bankcode();

        case AccountsModel::Column::Bic:
            return institution.value("bic");

        default:
            return QString();
        }
        break;

    case Qt::FontRole:
    {
        QFont font;
        // display top level account groups in bold
        if (!idx.parent().isValid()) {
            font.setBold(true);
        }
        return font;
    }

    case Qt::TextAlignmentRole:
        switch (idx.column()) {
        case AccountsModel::Column::Vat:
        case AccountsModel::Column::Balance:
        case AccountsModel::Column::PostedValue:
        case AccountsModel::Column::TotalPostedValue:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        default:
            break;
        }
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case Qt::ForegroundRole:
        switch(idx.column()) {
        case AccountsModel::Column::TotalPostedValue:
            return d->institutionValue(idx).isNegative() ? d->negativeScheme : d->positiveScheme;
        }
        break;

    case Qt::DecorationRole:
        if (idx.column() == AccountsModel::Column::AccountName) {
            if (!institution.value(QStringLiteral("icon")).isEmpty()) {
                return Icons::loadIconFromApplicationCache(institution.value(QStringLiteral("icon")));
            }
            return Icons::loadIconFromApplicationCache(QLatin1String("enum:Bank"));
        }
        break;

    case eMyMoney::Model::Roles::IdRole:
        return institution.id();

    case eMyMoney::Model::InstitutionNameRole:
        return institution.name();

    case eMyMoney::Model::InstitutionBankCodeRole:
        return institution.bankcode();

    case eMyMoney::Model::AccountDisplayOrderRole:
        // make sure the no bank assigned accounts show up at the top
        return (institution.id().isEmpty()) ? 0 : 1;
    }
    return QVariant();
}

bool InstitutionsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid()) {
        return false;
    }

    qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
    return QAbstractItemModel::setData(index, value, role);
}

void InstitutionsModel::setColorScheme(AccountsModel::ColorScheme scheme, const QColor& color)
{
    switch(scheme) {
    case AccountsModel::Positive:
        d->positiveScheme = color;
        break;
    case AccountsModel::Negative:
        d->negativeScheme = color;
        break;
    }
}


void InstitutionsModel::load(const QMap<QString, MyMoneyInstitution>& list)
{
    QElapsedTimer t;

    t.start();
    beginResetModel();
    // first get rid of any existing entries
    clearModelItems();

    int row = 0;
    // insert one more used for the no institution item
    insertRows(0, list.count()+1);
    MyMoneyInstitution noBank((QString()), MyMoneyInstitution());
    noBank.setName(i18n("Accounts with no institution assigned"));
    static_cast<TreeItem<MyMoneyInstitution>*>(index(0, 0).internalPointer())->dataRef() = noBank;
    ++row;

    // make sure to start from 0
    m_nextId = 0;

    for (const auto& item : list) {
        updateNextObjectId(item.id());
        const auto idx = index(row, 0);
        static_cast<TreeItem<MyMoneyInstitution>*>(idx.internalPointer())->dataRef() = item;
        d->loadAccounts(idx, item.accountList());
        ++row;
    }

    // and don't count loading as a modification
    setDirty(false);

    endResetModel();

    Q_EMIT modelLoaded();

    qDebug() << "Model for" << m_idLeadin << "loaded with" << rowCount() << "items in" << t.elapsed() << "ms";
}

void InstitutionsModel::slotLoadAccountsWithoutInstitutions(const QModelIndexList& indexes)
{
    bool dirty = m_dirty;
    for (const auto idx : indexes) {
        addAccount(QString(), idx.data(eMyMoney::Model::IdRole).toString());
    }
    m_dirty = dirty;
}

void InstitutionsModel::addAccount(const QString& institutionId, const QString& accountId)
{
    auto idx = indexById(institutionId);
    if (idx.isValid()) {
        const auto rows = rowCount(idx);
        insertRow(rows, idx);
        const auto subIdx = index(rows, 0, idx);
        MyMoneyInstitution account(accountId, MyMoneyInstitution());
        static_cast<TreeItem<MyMoneyInstitution>*>(subIdx.internalPointer())->dataRef() = account;
        Q_EMIT dataChanged(subIdx, subIdx);
    }
}

void InstitutionsModel::removeAccount(const QString& institutionId, const QString& accountId)
{
    auto idx = indexById(institutionId);
    if (idx.isValid()) {
        const auto rows = rowCount(idx);
        for (int row = 0; row < rows; ++row) {
            const auto subIdx = index(row, 0, idx);
            if (subIdx.data(eMyMoney::Model::IdRole).toString() == accountId) {
                removeRows(row, 1, idx);
                break;
            }
        }
    }
}
