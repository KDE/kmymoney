/*
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEPRICEMODEL_H
#define ONLINEPRICEMODEL_H

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
#include "mymoneyobject.h"

class KMM_MYMONEY_EXPORT OnlinePriceEntry
{
public:
    explicit OnlinePriceEntry() = default;
    explicit OnlinePriceEntry(const QString& id, const OnlinePriceEntry& other)
    {
        *this = other;
        m_id = id;
    }

    void setSymbol(const QString& symbol);
    void setName(const QString& name);
    void setDate(const QDate& date);
    void setPrice(const QString& price);
    void setSource(const QString& source);
    void setDirty();

    QString id() const;
    QString symbol() const;
    QString name() const;
    QDate date() const;
    QString price() const;
    QString source() const;
    bool isDirty() const;

    KMMStringSet referencedObjects() const
    {
        return {};
    }

    bool hasReferenceTo(const QString& id) const
    {
        Q_UNUSED(id)
        return false;
    }

    // Inequality operator
    bool operator!=(const OnlinePriceEntry& right) const;

private:
    QString m_id;
    QString m_symbol;
    QString m_name;
    QDate m_date;
    QString m_price;
    QString m_source;
    bool m_dirty = false;
};

/**
 */
class KMM_MYMONEY_EXPORT OnlinePriceModel : public MyMoneyModel<OnlinePriceEntry>
{
    Q_OBJECT
    friend OnlinePriceEntry;

public:
    enum Column {
        Symbol = 0,
        Name,
        Price,
        Date,
        Id,
        Source,
        // insert new columns above this line
        MaxColumns,
    };

    explicit OnlinePriceModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    virtual ~OnlinePriceModel();

    static const int ID_SIZE = 18;

    int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
     * Add price pair to the model. For an unknown id, a new
     * entry will be created. An existing entry will be updated
     *
     * @param id internal KMyMoney ID
     * @param symbol the symbol of the security/currency
     * @param name the name of the price pair
     * @param price the price information
     * @param date the date of the price information
     * @param source the source of the price information
     */
    void addOnlinePrice(const QString& id, const QString& symbol, const QString& name, const QString& price, const QDate& date, const QString& source);

    bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) override;

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // ONLINEPRICEMODEL_H
