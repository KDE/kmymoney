/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef CHECKPRINTING_H
#define CHECKPRINTING_H

#include <memory>

#include "kmymoneyplugin.h"
#include "selectedtransactions.h"

class KPluginInfo;
class QObject;
class CheckPrinting : public KMyMoneyPlugin::Plugin
{
    Q_OBJECT

public:
    explicit CheckPrinting(QObject *parent, const QVariantList &args);
    ~CheckPrinting() override;

public Q_SLOTS:
    void plug() override;
    void unplug() override;
    void configurationChanged() override;

private:
    void readCheckTemplate();

    bool canBePrinted(const KMyMoneyRegister::SelectedTransaction & selectedTransaction) const;
    void markAsPrinted(const KMyMoneyRegister::SelectedTransaction & selectedTransaction);

protected Q_SLOTS:
    void slotPrintCheck();
    void slotTransactionsSelected(const KMyMoneyRegister::SelectedTransactions& transactions);

private:
    struct Private;
    std::unique_ptr<Private> d;
};

#endif

