/*
 * Copyright 2009-2011  Laurent Montel <montel@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRANSACTIONSORTOPTIONIMPL_H
#define TRANSACTIONSORTOPTIONIMPL_H

#include "kmm_oldregister_export.h"

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
class KMM_OLDREGISTER_EXPORT TransactionSortOption : public QWidget
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
