/*
    SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAYEEIDENTIFIERCONTAINERMODEL_H
#define PAYEEIDENTIFIERCONTAINERMODEL_H

#include "kmm_models_export.h"

#include <QAbstractListModel>
#include <QSharedPointer>

#include "mymoney/payeeidentifiermodel.h"
#include "mymoney/mymoneypayeeidentifiercontainer.h"
#include "payeeidentifier/payeeidentifier.h"

/**
 * @brief Model for MyMoneyPayeeIdentifierContainer
 *
 * Changes the user does have internal effect only.
 *
 * @see payeeIdentifierModel
 */
class MyMoneyPayeeIdentifierContainer;
class payeeIdentifier;
class KMM_MODELS_EXPORT payeeIdentifierContainerModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * @brief Roles for this model
     *
     * They are equal to payeeIdentifierModel::roles
     */
    enum roles {
        payeeIdentifierType = payeeIdentifierModel::payeeIdentifierType, /**< type of payeeIdentifier */
        payeeIdentifier = payeeIdentifierModel::payeeIdentifier, /**< actual payeeIdentifier */
    };

    explicit payeeIdentifierContainerModel(QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role) const final override;

    /**
     * This model only supports to edit payeeIdentifier role with a QVariant of type
     * payeeIdentifier.
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

    Qt::ItemFlags flags(const QModelIndex& index) const final override;

    int rowCount(const QModelIndex& parent) const final override;

    bool insertRows(int row, int count, const QModelIndex& parent) final override;
    bool removeRows(int row, int count, const QModelIndex& parent) final override;

    /**
     * @brief Set source of data
     *
     * This makes the model editable.
     */
    void setSource(MyMoneyPayeeIdentifierContainer data);

    /** @brief Get stored data */
    QList< ::payeeIdentifier > identifiers() const;

public Q_SLOTS:
    /**
     * @brief Removes all data from the model
     *
     * The model is not editable afterwards.
     */
    void closeSource();

private:
    /** @internal
     * The use of a shared pointer makes this future prof. Because using identifier() causes
     * some unnecessary work.
     */
    QSharedPointer<MyMoneyPayeeIdentifierContainer> m_data;
};

#endif // PAYEEIDENTIFIERCONTAINERMODEL_H
