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

#ifndef FANCYDATEGROUPMARKERS_H
#define FANCYDATEGROUPMARKERS_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "fancydategroupmarker.h"

namespace eWidgets { enum class SortField;
                     namespace Transaction { enum class Column; }
                     namespace Register { enum class DetailColumn;} }
namespace eMyMoney { namespace Account { enum class Type; } }

namespace KMyMoneyRegister
{
  class Register;

  class StatementGroupMarkerPrivate;
  class StatementGroupMarker : public FancyDateGroupMarker
  {
    Q_DISABLE_COPY(StatementGroupMarker)

  public:
    explicit StatementGroupMarker(Register* getParent, eWidgets::eRegister::CashFlowDirection dir, const QDate& date, const QString& txt);
    ~StatementGroupMarker() override;

    eWidgets::eRegister::CashFlowDirection sortType() const override;
    int sortSamePostDate() const override;

  private:
    Q_DECLARE_PRIVATE(StatementGroupMarker)
  };


  class SimpleDateGroupMarker : public FancyDateGroupMarker
  {
    Q_DISABLE_COPY(SimpleDateGroupMarker)

  public:
    explicit SimpleDateGroupMarker(Register* getParent, const QDate& date, const QString& txt);
    ~SimpleDateGroupMarker() override;

    void paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;
    int rowHeightHint() const override;
    const char* className() override;
  };

  class FiscalYearGroupMarker : public FancyDateGroupMarker
  {
    Q_DISABLE_COPY(FiscalYearGroupMarker)

  public:
    explicit FiscalYearGroupMarker(Register* getParent, const QDate& date, const QString& txt);
    ~FiscalYearGroupMarker() override;

    const char* className() override;
    int sortSamePostDate() const override;
  };

} // namespace

#endif
