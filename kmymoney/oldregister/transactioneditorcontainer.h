/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRANSACTIONEDITORCONTAINER_H
#define TRANSACTIONEDITORCONTAINER_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTableWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace KMyMoneyRegister {
class Transaction;
}

class KMM_OLDREGISTER_EXPORT TransactionEditorContainer : public QTableWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(TransactionEditorContainer)

public:
    explicit TransactionEditorContainer(QWidget* parent);
    virtual ~TransactionEditorContainer();

    virtual void arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t) = 0;
    virtual void removeEditWidgets(QMap<QString, QWidget*>& editWidgets) = 0;
    virtual void tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const = 0;
    // FIXME remove tabbar
    // virtual int action(QMap<QString, QWidget*>& editWidgets) const = 0;
    // virtual void setProtectedAction(QMap<QString, QWidget*>& editWidgets, ProtectedAction action) = 0;

Q_SIGNALS:
    void geometriesUpdated();

protected Q_SLOTS:
    void updateGeometries() final override;
};

#endif
