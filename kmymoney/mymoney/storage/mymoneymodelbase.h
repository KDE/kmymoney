/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef MYMONEYMODELBASE_H
#define MYMONEYMODELBASE_H

// ----------------------------------------------------------------------------
// Qt Includes

#include <QAbstractItemModel>
#include <QRegularExpression>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"

class KMM_MYMONEY_EXPORT MyMoneyModelBase : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit MyMoneyModelBase(QObject* parent, const QString& idLeadin, quint8 idSize);
    virtual ~MyMoneyModelBase();

    /**
     * This method returns a list with the first QModelIndex that mathches the @a name
     * in the Qt::DisplayRole role in the model.
     *
     * @sa
     */
    virtual QModelIndexList indexListByName(const QString& name, const QModelIndex& parent = QModelIndex()) const;

    QModelIndex lowerBound(const QString& id) const;

    virtual QModelIndex lowerBound(const QString& id, int first, int last) const = 0;

    QModelIndex upperBound(const QString& id) const;

    virtual QModelIndex upperBound(const QString& id, int first, int last) const = 0;

    /**
     * This is convenience method returning the value of
     * mapToBaseSource(idx).model()
     *
     * @sa mapToBaseSource
     */
    static const QAbstractItemModel* baseModel(const QModelIndex& idx);

    /**
     * This method returns the model index in the base model of a
     * QModelIndex @a idx. The method traverses any possible filter
     * model until it finds the base model. In case the index already
     * points to the base model or points to an unknown filter model type
     * it is returned unaltered.
     *
     * @note The following filter models (and any derivatives are supported:
     * QSortFilterProxyModel, QConcatenateTablesProxyModel
     */
    static QModelIndex mapToBaseSource(const QModelIndex& idx);

    /**
     * This method returns a QModelIndex for a stacked @a proxyModel based
     * on the model index @a idx pointing to the base model.
     */
    static QModelIndex mapFromBaseSource(QAbstractItemModel* proxyModel, const QModelIndex& idx);

    void setDirty(bool dirty = true);
    bool isDirty() const;
    QString peekNextId() const;

    /**
     * checks if the @a id follows the ids created by this model
     * @returns @c true in case of a valid id, @c false otherwise.
     */
    bool isValidId (const QString& id) const;

    /**
     * This is the default implementation that supports
     * the role eMyMoney::Model::LongDisplayRole. It simply
     * returns the models headerDate for Qt::DisplayRole if
     * called with @a role LongDisplayRole.
     * In all other cases it calls QAbstractItemModel::headerData()).
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    QString nextId();

    virtual void updateNextObjectId(const QString& id);

    virtual void doUpdateReferencedObjects() = 0;

    /**
     * Overridden for internal reasons. This version will
     * block emission of signals during the reset completely
     */
    void beginResetModel();

    /**
     * Overridden for internal reasons. This version will
     * reset the signal emission state to the state it was
     * prior to the call of @sa beginResetModel()
     */
    void endResetModel();

protected Q_SLOTS:
    void updateReferencedObjects();

Q_SIGNALS:
    void modelLoaded();

protected:
    quint64                       m_nextId;
    QString                       m_idLeadin;
    quint8                        m_idSize;
    bool                          m_dirty;
    bool                          m_blockedSignals;
    QRegularExpression            m_idMatchExp;
};

#endif // MYMONEYMODELBASE_H
