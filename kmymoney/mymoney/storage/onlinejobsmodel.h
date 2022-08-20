/*
    SPDX-FileCopyrightText: 2019-2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEJOBSMODEL_H
#define ONLINEJOBSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "onlinejob.h"

class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT OnlineJobsModel : public MyMoneyModel<onlineJob>
{
    Q_OBJECT

public:
    enum Columns {
        PostDate,
        AccountName,
        Action,
        Destination,
        Value,
        Purpose,
        // insert new columns above this line
        MaxColumns,
    };

    explicit OnlineJobsModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    virtual ~OnlineJobsModel();

    static const int ID_SIZE = 6;

    int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

public Q_SLOTS:

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // ONLINEJOBSMODEL_H

