/***************************************************************************
                             transactioneditorcontainer.cpp
                             ----------
    begin                : Wed Jun 07 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
