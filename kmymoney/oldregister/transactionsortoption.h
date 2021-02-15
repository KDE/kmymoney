/*
    SPDX-FileCopyrightText: 2009-2011 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
