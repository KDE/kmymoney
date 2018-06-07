/*
 * Copyright 2004-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KMYMONEYCOMPLETION_H
#define KMYMONEYCOMPLETION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QTreeWidgetItem;
class QTreeWidget;

class KMyMoneySelector;

/**
  * @author Thomas Baumgart
  */

class KMyMoneyCompletionPrivate;
class KMyMoneyCompletion : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyCompletion)

public:

  explicit  KMyMoneyCompletion(QWidget* parent = nullptr);
  virtual ~KMyMoneyCompletion();

  /**
    * Re-implemented for internal reasons.  API is unaffected.
    */
  virtual void hide();

  /**
    * This method sets the current account with id @p id as
    * the current selection.
    *
    * @param id id of account to be selected
    */
  void setSelected(const QString& id);

  KMyMoneySelector* selector() const;

public Q_SLOTS:
  void slotMakeCompletion(const QString& txt);

  void slotItemSelected(QTreeWidgetItem *item, int col);

protected:
  /**
    * Reimplemented from KMyMoneyAccountSelector to get events from the viewport (to hide
    * this widget on mouse-click, Escape-presses, etc.
    */
  bool eventFilter(QObject *, QEvent *) override;

  /**
    * Re-implemented for internal reasons.  API is unaffected.
    */
  void showEvent(QShowEvent*) override;

  /**
    * This method resizes the widget to show a maximum of @p count
    * or @a MAX_ITEMS items.
    *
    * @param count maximum number to be shown if < MAX_ITEMS
    */
  void adjustSize(const int count);

  /**
    * This method counts the number of items currently visible and
    * calls adjustSize(count).
    */
  void adjustSize();

  void connectSignals(QWidget *widget, QTreeWidget* lv);

  void show(bool presetSelected);

Q_SIGNALS:
  void itemSelected(const QString& id);

protected:
  KMyMoneyCompletionPrivate * const d_ptr;
  KMyMoneyCompletion(KMyMoneyCompletionPrivate &dd, QWidget* parent = nullptr);
  Q_DECLARE_PRIVATE(KMyMoneyCompletion)
};

#endif
