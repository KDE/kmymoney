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

#include "kmymoneyviewbase.h"
#include "kmymoneyviewbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPageWidgetItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"

KMyMoneyViewBase::KMyMoneyViewBase(QWidget* parent)
  : QWidget(parent)
  , d_ptr(new KMyMoneyViewBasePrivate(this))
{
}

KMyMoneyViewBase::KMyMoneyViewBase(KMyMoneyViewBasePrivate &dd, QWidget *parent)
  : QWidget(parent)
  , d_ptr(&dd)
{
  // make sure we keep a copy of what we send out
  connect(this, &KMyMoneyViewBase::requestSelectionChange, this, [&](const SelectedObjects& selections) {
    Q_D(KMyMoneyViewBase);
    d->m_selections = selections;
  });
}

KMyMoneyViewBase::~KMyMoneyViewBase()
{
}

void KMyMoneyViewBase::viewChanged(KPageWidgetItem* current, KPageWidgetItem* before)
{
  Q_UNUSED(before)
  Q_D(KMyMoneyViewBase);

  // did I get selected?
  if (current->widget() == static_cast<QWidget*>(this)) {
    // tell everyone what is selected here
    emit requestSelectionChange(d->m_selections);
  }
}

void KMyMoneyViewBase::changeEvent(QEvent* ev)
{
  QWidget::changeEvent(ev);

  if(ev->type() == QEvent::EnabledChange) {
    emit viewStateChanged(isEnabled());
  }
}
