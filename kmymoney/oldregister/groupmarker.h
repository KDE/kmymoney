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

#ifndef GROUPMARKER_H
#define GROUPMARKER_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "registeritem.h"

namespace KMyMoneyRegister
{
class Register;

class GroupMarkerPrivate;
class KMM_OLDREGISTER_EXPORT GroupMarker : public RegisterItem
{
  Q_DISABLE_COPY(GroupMarker)

public:
  explicit GroupMarker(Register* getParent, const QString& txt);
  ~GroupMarker() override;

  void setText(const QString& txt);
  QString text() const;
  bool isSelectable() const override;
  bool canHaveFocus() const override;
  int numRows() const;
  const char* className() override;
  bool isErroneous() const override;
  void paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;
  void paintFormCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) override;

  int rowHeightHint() const override;

  bool matches(const RegisterFilter&) const override;
  int sortSamePostDate() const override;
  void setErroneous(bool condition = true);

protected:
  GroupMarker(GroupMarkerPrivate &dd, Register *parent, const QString& txt);
  Q_DECLARE_PRIVATE(GroupMarker)
};

} // namespace

#endif
