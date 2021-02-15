/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef GROUPMARKER_H
#define GROUPMARKER_H

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
class GroupMarker : public RegisterItem
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
