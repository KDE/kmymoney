/***************************************************************************
                             transactioneditorcontainer.h
                             ----------
    begin                : Wed Jun 07 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRANSACTIONEDITORCONTAINER_H
#define TRANSACTIONEDITORCONTAINER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QTableWidget>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

namespace KMyMoneyRegister
{
class Transaction;
}

typedef enum {
  ProtectNone = 0,
  ProtectTransfer,
  ProtectNonTransfer,
  ProtectAll
} ProtectedAction;

class TransactionEditorContainer : public QTableWidget
{
  Q_OBJECT

public:
  TransactionEditorContainer(QWidget* parent) : QTableWidget(parent) {}

  virtual void arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t) = 0;
  virtual void removeEditWidgets(QMap<QString, QWidget*>& editWidgets) = 0;
  virtual void tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const = 0;
  // FIXME remove tabbar
  // virtual int action(QMap<QString, QWidget*>& editWidgets) const = 0;
  // virtual void setProtectedAction(QMap<QString, QWidget*>& editWidgets, ProtectedAction action) = 0;

signals:
  void geometriesUpdated();

protected slots:
  void updateGeometries() {
    QTableWidget::updateGeometries();
    emit geometriesUpdated();
  }
};

#endif
