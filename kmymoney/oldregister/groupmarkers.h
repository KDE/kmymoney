/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef GROUPMARKERS_H
#define GROUPMARKERS_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "groupmarker.h"

namespace eWidgets { enum class SortField;
                     namespace Transaction { enum class Column; }
                     namespace Register { enum class DetailColumn;} }
namespace eMyMoney { namespace Account { enum class Type; } }

namespace KMyMoneyRegister
{
  class RegisterItem;

  class TypeGroupMarkerPrivate;
  class TypeGroupMarker : public GroupMarker
  {
    Q_DISABLE_COPY(TypeGroupMarker)

  public:
    explicit TypeGroupMarker(Register* getParent, eWidgets::eRegister::CashFlowDirection dir, eMyMoney::Account::Type accType);
    ~TypeGroupMarker() override;

    eWidgets::eRegister::CashFlowDirection sortType() const override;

  private:
    Q_DECLARE_PRIVATE(TypeGroupMarker)
  };

  class PayeeGroupMarker : public GroupMarker
  {
    Q_DISABLE_COPY(PayeeGroupMarker)

  public:
    explicit PayeeGroupMarker(Register* getParent, const QString& name);
    ~PayeeGroupMarker() override;

    const QString& sortPayee() const override;
  };

  class CategoryGroupMarker : public GroupMarker
  {
    Q_DISABLE_COPY(CategoryGroupMarker)

  public:
    explicit CategoryGroupMarker(Register* getParent, const QString& category);
    ~CategoryGroupMarker() override;

    const QString& sortCategory() const override;
    const QString sortSecurity() const override;
    const char* className() override;
  };

  class ReconcileGroupMarkerPrivate;
  class ReconcileGroupMarker : public GroupMarker
  {
    Q_DISABLE_COPY(ReconcileGroupMarker)

  public:
    explicit ReconcileGroupMarker(Register* getParent, eMyMoney::Split::State state);
    ~ReconcileGroupMarker() override;

    eMyMoney::Split::State sortReconcileState() const override;

  private:
    Q_DECLARE_PRIVATE(ReconcileGroupMarker)
  };

} // namespace

#endif
