/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
