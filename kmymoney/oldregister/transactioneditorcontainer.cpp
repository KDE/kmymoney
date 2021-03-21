/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "transactioneditorcontainer.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

TransactionEditorContainer::TransactionEditorContainer(QWidget* parent) : QTableWidget(parent)
{
}

TransactionEditorContainer::~TransactionEditorContainer()
{
}

void TransactionEditorContainer::updateGeometries()
{
    QTableWidget::updateGeometries();
    emit geometriesUpdated();
}
