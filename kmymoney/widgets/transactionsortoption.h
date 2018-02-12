/*  This file is part of the KDE project
    Copyright (C) 2009 Laurent Montel <montel@kde.org>
    (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef TRANSACTIONSORTOPTIONIMPL_H
#define TRANSACTIONSORTOPTIONIMPL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class TransactionSortOption; }
namespace eWidgets { enum class SortField; }

class QListWidget;
class QListWidgetItem;
class TransactionSortOption : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(TransactionSortOption)

public:
  explicit TransactionSortOption(QWidget *parent);
  ~TransactionSortOption();
  QString settings() const;

public Q_SLOTS:
  void setSettings(const QString& settings);

protected:
  QListWidgetItem * addEntry(QListWidget * p, QListWidgetItem * after, int idx);
  void setDirectionIcon(QListWidgetItem* item);

protected Q_SLOTS:
  void slotUpdateButtons();
  void slotAddItem();
  void slotRemoveItem();
  void slotUpItem();
  void slotDownItem();
  void slotToggleDirection();

Q_SIGNALS:
  void settingsChanged(const QString&);

private:
  static eWidgets::SortField textToSortOrder(const QString& text);
  static QString sortOrderToText(eWidgets::SortField idx);
  Ui::TransactionSortOption *ui;
};

#endif /* TRANSACTIONSORTOPTIONIMPL_H */
