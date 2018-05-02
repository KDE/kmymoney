/***************************************************************************
                          kmymoneyviewbase.h
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

/**
  * This class is an abstract base class that all specific views
  * should be based on.
  */
class KMyMoneyViewBasePrivate;
class KMM_WIDGETS_EXPORT KMyMoneyViewBase : public QWidget
{
  Q_OBJECT

public:
  explicit KMyMoneyViewBase(QWidget* parent = nullptr);
  virtual ~KMyMoneyViewBase();

  virtual void executeCustomAction(eView::Action) {}

Q_SIGNALS:
  void selectByObject(const MyMoneyObject&, eView::Intent);
  void selectByVariant(const QVariantList&, eView::Intent);
  void customActionRequested(View, eView::Action);

public slots:
  virtual void slotSelectByObject(const MyMoneyObject&, eView::Intent) {}
  virtual void slotSelectByVariant(const QVariantList&, eView::Intent) {}

protected:
  const QScopedPointer<KMyMoneyViewBasePrivate> d_ptr;
  KMyMoneyViewBase(KMyMoneyViewBasePrivate &dd, QWidget *parent);

private:
  Q_DECLARE_PRIVATE(KMyMoneyViewBase)
};

#endif
