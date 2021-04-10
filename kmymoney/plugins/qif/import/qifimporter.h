/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QIFIMPORTER_H
#define QIFIMPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"
#include "mymoneystatement.h"

class MyMoneyQifReader;

class QIFImporter : public KMyMoneyPlugin::Plugin
{
    Q_OBJECT

public:
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
    explicit QIFImporter(QObject *parent, const QVariantList &args);
#else
    explicit QIFImporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
#endif
    ~QIFImporter() override;

    QAction          *m_action;

private:
    MyMoneyQifReader *m_qifReader;

private Q_SLOTS:

    /**
      * Called when the user wishes to import tab delimited transactions
      * into the current account.  An account must be open for this to
      * work.  Calls KMyMoneyView::slotAccountImportAscii.
      *
      * @see MyMoneyAccount
      */
    void slotQifImport();

    bool slotGetStatements(const QList<MyMoneyStatement> &statements);

protected:
    void createActions();
};

#endif
