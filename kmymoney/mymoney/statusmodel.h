/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATUSMODEL_H
#define STATUSMODEL_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneymodel.h>
#include <mymoneyenums.h>

class StatusEntry {
public:
    explicit StatusEntry() {}
    explicit StatusEntry(const QString& id, const StatusEntry& other)
        : m_id(id)
        , m_shortName(other.m_shortName)
        , m_longName(other.m_longName)
        , m_state(other.m_state)
    {}
    explicit StatusEntry(const QString& id, eMyMoney::Split::State state, const QString& shortName, const QString& longName)
        : m_id(id)
        , m_shortName(shortName)
        , m_longName(longName)
        , m_state(state)
    {}

    inline const QString& id() const {
        return m_id;
    }
    inline const QString& shortName() const {
        return m_shortName;
    }
    inline const QString& longName() const {
        return m_longName;
    }
    inline eMyMoney::Split::State state() const {
        return m_state;
    }
    inline QSet<QString> referencedObjects() const {
        return {};
    }
    bool hasReferenceTo(const QString& id) const {
        Q_UNUSED(id) return false;
    }

private:
    QString                 m_id;
    QString                 m_shortName;
    QString                 m_longName;
    eMyMoney::Split::State  m_state;
};

class KMM_MYMONEY_EXPORT StatusModel : public MyMoneyModel<StatusEntry>
{
    Q_OBJECT

public:
    explicit StatusModel(QObject* parent = nullptr);
    virtual ~StatusModel();

    int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;

private:

};
#endif // STATUSMODEL_H

