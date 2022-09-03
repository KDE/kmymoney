/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRICEMODEL_H
#define PRICEMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QSharedDataPointer>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneyobject.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"

class QUndoStack;

class KMM_MYMONEY_EXPORT PriceEntry : public MyMoneyPrice
{
public:
    explicit PriceEntry() : MyMoneyPrice() {}
    explicit PriceEntry(const QString& id, const PriceEntry& other)
    {
        *this = other;
        m_id = id;
    }
    explicit PriceEntry(const MyMoneyPrice& price);

    /**
     * This method returns the id of the price entry which is build via
     * PriceModel::createId() and consists of the date and the two
     * securities ids. The items are separated by a dash character each.
     */
    const QString& id() const {
        return m_id;
    }

    /**
     * This method returns the price pair which consists of the
     * two security ids.
     */
    QPair<QString,QString> pricePair() const;

    QSet<QString> referencedObjects() const {
        return {};
    }
    bool hasReferenceTo(const QString& id) const {
        Q_UNUSED(id) return false;
    }

private:
    QString       m_id;
};

/**
  */
class KMM_MYMONEY_EXPORT PriceModel : public MyMoneyModel<PriceEntry>
{
    Q_OBJECT
    friend PriceEntry;

public:
    enum Column {
        Commodity = 0,    // FromSecurity
        StockName,
        Currency,         // ToSecurity
        Date,
        Price,
        Source,
        // insert new columns above this line
        MaxColumns,
    };

    explicit PriceModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    virtual ~PriceModel();

    static const int ID_SIZE = 18;


    int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

    /**
     * Special implementation using a binary search algorithm instead
     * of the linear one provided by the template function
     */
    MyMoneyPrice price(const QString& from, const QString& to, const QDate& date, bool exactDate) const;

    void addPrice(const MyMoneyPrice& price);
    void removePrice(const MyMoneyPrice& price);
    MyMoneyPriceList priceList() const;

    bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) override;

    void load(const QMap<MyMoneySecurityPair, MyMoneyPriceEntries>& list);

private:
    QModelIndex firstIndexById(const QString& id) const;
    QModelIndex firstIndexByKey(const QString& key) const;

    static QString createId(const QString& from, const QString& to, const QDate& date);

public Q_SLOTS:

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // PRICEMODEL_H

