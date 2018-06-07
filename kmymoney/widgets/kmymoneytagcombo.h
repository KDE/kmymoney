/*
 * Copyright 2010-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2010-2016  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2010       Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KMYMONEYTAGCOMBO_H
#define KMYMONEYTAGCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneymvccombo.h"

class MyMoneyTag;

/**
  * This class implements a text based tag selector.
  * The widget has the functionality of a KMyMoneyPayeeCombo object.
  * Whenever a key is pressed, the set of loaded tags is searched for
  * tags names which match the currently entered text.
  *
  * @author Alessandro Russo
  */
class KMyMoneyTagComboPrivate;
class KMM_WIDGETS_EXPORT KMyMoneyTagCombo : public KMyMoneyMVCCombo
{
  Q_OBJECT
  Q_DISABLE_COPY(KMyMoneyTagCombo)

public:
  explicit KMyMoneyTagCombo(QWidget* parent = nullptr);
  ~KMyMoneyTagCombo() override;

  void loadTags(const QList<MyMoneyTag>& list);
  /** ids in usedIdList are escluded from the internal list
    * you should call loadTags before calling setUsedTagList because it doesn't readd
    * tag removed in previous call*/
  void setUsedTagList(QList<QString>& usedIdList, QList<QString>& usedTagNameList);

protected:
  /**
    * check if the current text is contained in the internal list, if not ask the user if want to create a new item.
    */
  virtual void checkCurrentText() override;

private:
  Q_DECLARE_PRIVATE(KMyMoneyTagCombo)
};

#endif
