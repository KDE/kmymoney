/*
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef KMYMONEYVIEWBASE_H
#define KMYMONEYVIEWBASE_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "viewenums.h"

class MyMoneyObject;
class KPageWidgetItem;
class SelectedObjects;

/**
  * This class is an abstract base class that all specific views
  * should be based on.
  */
class KMyMoneyViewBasePrivate;
class KMM_WIDGETS_EXPORT KMyMoneyViewBase : public QWidget
{
  Q_OBJECT

public:
  virtual ~KMyMoneyViewBase();

  virtual void executeCustomAction(eView::Action) {}

protected:
  void changeEvent(QEvent* ev) override;

Q_SIGNALS:
  // these signals are send to application logic
  void requestSelectionChange (const SelectedObjects& selection);

  void viewStateChanged(bool enabled);

  /**
   * @deprecated use selectionChanged() instead
   */
  void selectByObject(const MyMoneyObject&, eView::Intent) Q_DECL_DEPRECATED;
  /**
   * @deprecated use selectionChanged() instead
   */
  void selectByVariant(const QVariantList&, eView::Intent) Q_DECL_DEPRECATED;

  void customActionRequested(View, eView::Action);

public slots:
  void viewChanged(KPageWidgetItem* current, KPageWidgetItem* before);

  virtual void updateActions(const SelectedObjects& selections) { Q_UNUSED(selections) }

  virtual void slotSelectByObject(const MyMoneyObject&, eView::Intent) {}
  virtual void slotSelectByVariant(const QVariantList&, eView::Intent) {}
  virtual void slotSettingsChanged() {}

protected:
  const QScopedPointer<KMyMoneyViewBasePrivate> d_ptr;

  // we do not allow to create objects of this class
  explicit KMyMoneyViewBase(QWidget* parent = nullptr);
  KMyMoneyViewBase(KMyMoneyViewBasePrivate &dd, QWidget *parent);

private:
  Q_DECLARE_PRIVATE(KMyMoneyViewBase)
};

#endif
