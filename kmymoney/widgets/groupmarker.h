/***************************************************************************
                             groupmarker.h
                             ----------
    begin                : Fri Mar 10 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef GROUPMARKER_H
#define GROUPMARKER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "registeritem.h"

class MyMoneyTransaction;

namespace KMyMoneyRegister
{

class Register;
class RegisterItem;
class ItemPtrVector;

class GroupMarkerPrivate;
class GroupMarker : public RegisterItem
{
  Q_DISABLE_COPY(GroupMarker)

public:
  explicit GroupMarker(Register* getParent, const QString& txt);
  ~GroupMarker() override;

  void setText(const QString& txt);
  const QString& text() const;
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
