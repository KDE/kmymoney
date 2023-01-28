/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RECONCILIATIONMODEL_H
#define RECONCILIATIONMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneymodel.h"
#include "mymoneymoney.h"

class /* no export here on purpose */ ReconciliationEntry
{
public:
    explicit ReconciliationEntry()
    {
    }
    explicit ReconciliationEntry(const QString& id, const ReconciliationEntry& other)
        : m_id(id)
        , m_accountId(other.m_accountId)
        , m_amount(other.m_amount)
        , m_date(other.m_date)
        , m_filterHint(other.m_filterHint)
    {
    }

    ReconciliationEntry(const QString& id,
                        const QString& accountId,
                        const QDate& date,
                        const MyMoneyMoney& amount,
                        eMyMoney::Model::ReconciliationFilterHint filterHint)
        : m_id(id)
        , m_accountId(accountId)
        , m_amount(amount)
        , m_date(date)
        , m_filterHint(filterHint)
    {
    }

    inline const QString& id() const
    {
        return m_id;
    }
    inline const QString& accountId() const
    {
        return m_accountId;
    }
    inline const MyMoneyMoney& amount() const
    {
        return m_amount;
    }
    inline const QDate& date() const
    {
        return m_date;
    }
    inline bool hasReferenceTo(const QString&) const
    {
        return false;
    }
    inline eMyMoney::Model::ReconciliationFilterHint filterHint() const
    {
        return m_filterHint;
    }

    /**
     * @copydoc MyMoneyObject::referencedObjects
     */
    inline QSet<QString> referencedObjects() const
    {
        return {};
    }

private:
    QString m_id;
    QString m_accountId;
    MyMoneyMoney m_amount;
    QDate m_date;
    eMyMoney::Model::ReconciliationFilterHint m_filterHint;
};

class QUndoStack;
/**
 */
class KMM_MYMONEY_EXPORT ReconciliationModel : public MyMoneyModel<ReconciliationEntry>
{
    Q_OBJECT

public:
    explicit ReconciliationModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    ~ReconciliationModel();

    static const int ID_SIZE = 4;

    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void setOptions(bool showDateHeaders);

public Q_SLOTS:
    void updateData();

private Q_SLOTS:
    /**
     * override the MyMoneyModel::load() method here so that it cannot
     * be called, as it is useless in the context of this class
     */
    void load(const QMap<QString, ReconciliationEntry>& list);
    void doLoad();

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // RECONCILIATIONMODEL_H
