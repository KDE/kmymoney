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

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "viewenums.h"

/**
  * This class is an abstract base class that all specific views
  * should be based on.
  */
class KMyMoneyViewBasePrivate;
class KMyMoneyViewBase : public QWidget
{
  Q_OBJECT

public:
    explicit KMyMoneyViewBase(QWidget* parent = nullptr);
    virtual ~KMyMoneyViewBase();

    virtual void setDefaultFocus() {}
    virtual void refresh() {}

Q_SIGNALS:
  void aboutToShow(const View view);

protected:
  const QScopedPointer<KMyMoneyViewBasePrivate> d_ptr;
  KMyMoneyViewBase(KMyMoneyViewBasePrivate &dd, QWidget *parent);

private:
  Q_DECLARE_PRIVATE(KMyMoneyViewBase)
};

#endif
