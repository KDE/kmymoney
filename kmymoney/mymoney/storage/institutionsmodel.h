/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INSTITUTIONSMODEL_H
#define INSTITUTIONSMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "accountsmodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneyinstitution.h"

class AccountsModel;
class QColor;
class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT InstitutionsModel : public MyMoneyModel<MyMoneyInstitution>
{
    Q_OBJECT

public:
    explicit InstitutionsModel(AccountsModel* accountsModel, QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    virtual ~InstitutionsModel();

    static const int ID_SIZE = 6;

    int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const final override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;

    bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) final override;

    void load(const QMap<QString, MyMoneyInstitution>& list);
    void addAccount(const QString& institutionId, const QString& accountId);
    void removeAccount(const QString& institutionId, const QString& accountId);

    void setColorScheme(AccountsModel::ColorScheme scheme, const QColor& color);

public Q_SLOTS:
    /**
     * Add the accounts pointed to by @a indexes to the group of
     * accounts not assigned to any institution. The indexes should
     * point into the AccountsModel. The addition is performed on
     * the id returned by the @c IdRole role. The dirty flag
     * is not modified.
     */
    void slotLoadAccountsWithoutInstitutions(const QModelIndexList& indexes);

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // INSTITUTIONSMODEL_H

