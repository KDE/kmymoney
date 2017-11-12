/***************************************************************************
                          kmymoneygeneralcombo.h  -  description
                             -------------------
    begin                : Mon Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>
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

#ifndef KMYMONEYGENERALCOMBO_H
#define KMYMONEYGENERALCOMBO_H

#include "kmm_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMM_WIDGETS_EXPORT KMyMoneyGeneralCombo : public KComboBox
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyGeneralCombo)
  Q_PROPERTY(int currentItem READ currentItem WRITE setCurrentItem STORED false)

public:
  explicit KMyMoneyGeneralCombo(QWidget* parent = nullptr);
  virtual ~KMyMoneyGeneralCombo();

  void insertItem(const QString& txt, int id, int idx = -1);

  void setCurrentItem(int id);
  int currentItem() const;

  void removeItem(int id);

public slots:
  void clear();

signals:
  void itemSelected(int id);

protected:
  // prevent the caller to use the standard KComboBox insertItem function with a default idx
  void insertItem(const QString&);

protected slots:
  void slotChangeItem(int idx);

};

#endif
