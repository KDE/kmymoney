/*
    SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPLITMODEL_H
#define SPLITMODEL_H

#include "kmm_models_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractTableModel>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class SplitModelPrivate;
class KMM_MODELS_EXPORT SplitModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SplitModel(QObject* parent = nullptr);
    virtual ~SplitModel();
    void deepCopy(const SplitModel& right, bool revertSplitSign = false);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
     * Adds a single split @a t to the model
     */
    void addSplit(const QString& transactionSplitId);

    /**
     * Adds a single dummy split to the model which is used for
     * creation of new splits.
     */
    void addEmptySplitEntry();

    /**
     * Remove the single dummy split to the model which is used for
     * creation of new splits from the model.
     */
    void removeEmptySplitEntry();

    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

    /**
     * This method returns the string to be used for new split ids
     */
    static QString newSplitId();

    /**
     * This method compares the @a id against the one provided
     * by newSplitId() and returns true if it is identical.
     */
    static bool isNewSplitId(const QString& id);

    // void removeSplit(const LedgerTransaction& t);

private:
    Q_DISABLE_COPY(SplitModel)
    Q_DECLARE_PRIVATE(SplitModel)
    const QScopedPointer<SplitModelPrivate> d_ptr;

};
#endif // SPLITMODEL_H

