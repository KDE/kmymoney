/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCHEDULESJOURNALMODEL_H
#define SCHEDULESJOURNALMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "mymoneyenums.h"

#include "kmm_mymoney_export.h"


class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT SchedulesJournalModel : public JournalModel
{
    Q_OBJECT

public:
    class Column {
    public:
        enum {
            Name,
        } Columns;
    };

    explicit SchedulesJournalModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    ~SchedulesJournalModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void setPreviewPeriod(int days);
    void setShowPlannedDate(bool showPlannedDate = true);

public Q_SLOTS:
    void updateData();

private Q_SLOTS:
    /**
     * override the JournalModel::load() method here so that it cannot
     * be called, as it is useless in the context of this class
     */
    void load(const QMap<QString, MyMoneyTransaction>& list);
    void doLoad();

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // SCHEDULESJOURNALMODEL_H

