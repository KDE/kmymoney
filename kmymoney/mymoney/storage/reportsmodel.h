/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REPORTSMODEL_H
#define REPORTSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneymodel.h"

#include "mymoneyreport.h"
#include "reportgroup.h"

class QUndoStack;

/**
  */
class KMM_MYMONEY_EXPORT ReportsModel : public MyMoneyModel<MyMoneyReport>
{
    Q_OBJECT

public:
    enum Columns {
        ReportName,
        Comment,
        Favorite,
        Modified,
        Group,
        // insert new columns above this line
        MaxColumns,
    };

    explicit ReportsModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    virtual ~ReportsModel();

    static const int ID_SIZE = 6;

    int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    int processItems(Worker* worker) override;

    bool useGroups() const;
    void setUseGroups(bool state);

public Q_SLOTS:
    void load(const QMap<QString, MyMoneyReport>& reports) override;
    void load(const QList<ReportGroup>& reportGroups);

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // REPORTSMODEL_H

