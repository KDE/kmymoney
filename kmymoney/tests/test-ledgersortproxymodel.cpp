/*
    SPDX-FileCopyrightText: 2026 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QAbstractTableModel>
#include <QCoreApplication>
#include <QDate>
#include <QTest>

#include <utility>

#include "ledgersortorder.h"
#include "ledgersortproxymodel.h"
#include "ledgersortproxymodel_p.h"
#include "mymoneyenums.h"
#include "mymoneymoney.h"

namespace {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
using RoleList = QVector<int>;
#else
using RoleList = QList<int>;
#endif

struct RowData {
    QString id;
    QDate postDate;
    eMyMoney::Split::State reconcileState;
    MyMoneyMoney amount;
};

class TestLedgerModel final : public QAbstractTableModel
{
public:
    explicit TestLedgerModel(QObject* parent = nullptr)
        : QAbstractTableModel(parent)
    {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override final
    {
        return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override final
    {
        Q_UNUSED(parent)
        return 1;
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override final
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_rows.size())) {
            return {};
        }

        const auto& row = m_rows.at(index.row());
        switch (role) {
        case eMyMoney::Model::IdRole:
            return row.id;
        case eMyMoney::Model::TransactionPostDateRole:
            return row.postDate;
        case eMyMoney::Model::SplitReconcileFlagRole:
            return static_cast<int>(row.reconcileState);
        case eMyMoney::Model::SplitSharesRole:
            return QVariant::fromValue(row.amount);
        case eMyMoney::Model::BaseModelRole:
            return QVariant::fromValue(eMyMoney::Model::JournalEntryRole);
        default:
            break;
        }
        return {};
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override final
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_rows.size())) {
            return false;
        }

        auto& row = m_rows[index.row()];
        switch (role) {
        case eMyMoney::Model::SplitReconcileFlagRole:
            row.reconcileState = static_cast<eMyMoney::Split::State>(value.toInt());
            break;
        case eMyMoney::Model::SplitSharesRole:
            row.amount = value.value<MyMoneyMoney>();
            break;
        default:
            return false;
        }

        Q_EMIT dataChanged(index, index, RoleList{role});
        return true;
    }

    void setRows(QVector<RowData> rows)
    {
        beginResetModel();
        m_rows = std::move(rows);
        endResetModel();
    }

private:
    QVector<RowData> m_rows;
};

class TestLedgerSortProxyModel final : public LedgerSortProxyModel
{
public:
    explicit TestLedgerSortProxyModel(QObject* parent = nullptr)
        : LedgerSortProxyModel(new LedgerSortProxyModelPrivate(this), parent)
    {
    }
};
}

class LedgerSortProxyModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void modelResetTriggersResort();
    void sortRoleDataChangeTriggersResort();
};

static void processDeferredSort()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::MetaCall);
    QCoreApplication::processEvents();
}

void LedgerSortProxyModelTest::modelResetTriggersResort()
{
    TestLedgerModel model;
    model.setRows({
        {QStringLiteral("a"), QDate(2026, 5, 16), eMyMoney::Split::State::NotReconciled, MyMoneyMoney(-10)},
        {QStringLiteral("b"), QDate(2026, 5, 16), eMyMoney::Split::State::NotReconciled, MyMoneyMoney(100)},
        {QStringLiteral("c"), QDate(2026, 5, 16), eMyMoney::Split::State::Cleared, MyMoneyMoney(20)},
    });

    TestLedgerSortProxyModel proxy;
    proxy.setSourceModel(&model);
    proxy.setSortingEnabled(true);
    proxy.setLedgerSortOrder(LedgerSortOrder(QStringLiteral("1,-9,-4")));
    processDeferredSort();

    QCOMPARE(proxy.index(0, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("c"));
    QCOMPARE(proxy.index(1, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("b"));
    QCOMPARE(proxy.index(2, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("a"));

    model.setRows({
        {QStringLiteral("a"), QDate(2026, 5, 16), eMyMoney::Split::State::NotReconciled, MyMoneyMoney(-10)},
        {QStringLiteral("c"), QDate(2026, 5, 16), eMyMoney::Split::State::Cleared, MyMoneyMoney(20)},
        {QStringLiteral("b"), QDate(2026, 5, 16), eMyMoney::Split::State::NotReconciled, MyMoneyMoney(100)},
    });
    processDeferredSort();

    QCOMPARE(proxy.index(0, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("c"));
    QCOMPARE(proxy.index(1, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("b"));
    QCOMPARE(proxy.index(2, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("a"));
}

void LedgerSortProxyModelTest::sortRoleDataChangeTriggersResort()
{
    TestLedgerModel model;
    model.setRows({
        {QStringLiteral("a"), QDate(2026, 5, 16), eMyMoney::Split::State::NotReconciled, MyMoneyMoney(100)},
        {QStringLiteral("b"), QDate(2026, 5, 16), eMyMoney::Split::State::NotReconciled, MyMoneyMoney(50)},
    });

    TestLedgerSortProxyModel proxy;
    proxy.setSourceModel(&model);
    proxy.setSortingEnabled(true);
    proxy.setLedgerSortOrder(LedgerSortOrder(QStringLiteral("1,-9,-4")));
    processDeferredSort();

    QCOMPARE(proxy.index(0, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("a"));
    QCOMPARE(proxy.index(1, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("b"));

    QVERIFY(model.setData(model.index(1, 0), static_cast<int>(eMyMoney::Split::State::Cleared), eMyMoney::Model::SplitReconcileFlagRole));
    processDeferredSort();

    QCOMPARE(proxy.index(0, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("b"));
    QCOMPARE(proxy.index(1, 0).data(eMyMoney::Model::IdRole).toString(), QStringLiteral("a"));
}

QTEST_GUILESS_MAIN(LedgerSortProxyModelTest)

#include "test-ledgersortproxymodel.moc"
