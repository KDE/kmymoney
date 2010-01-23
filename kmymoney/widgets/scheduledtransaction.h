/***************************************************************************
                          scheduledtransaction.h
                             -------------------
    begin                : Tue Aug 19 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SCHEDULEDTRANSACTION_H
#define SCHEDULEDTRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <transaction.h>

namespace KMyMoneyTransactionForm
{
class TransactionForm;
} // namespace

namespace KMyMoneyRegister
{

class StdTransactionScheduled : public StdTransaction
{
public:
  StdTransactionScheduled(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~StdTransactionScheduled() {}

  virtual const char* className(void) {
    return "StdTransactionScheduled";
  }

  /**
   * This method sets the general parameters required for the painting of a cell
   * in the register. These are:
   *
   * - background color (alternating)
   * - background color (imported transaction)
   * - background color (matched transaction)
   * - background color (selected transaction)
   * - cellRect (area covering the cell)
   * - textRect (area covering the text)
   * - color of the pen to do the painting of text and lines
   *
   * @param painter pointer to the QPainter object
   * @param row vertical index of cell in register
   * @param col horizontal index of cell in register
   * @param cellRect ref to QRect object receiving the area information for the cell
   * @param textRect ref to QRect object receiving the area information for the text
   * @param cg ref to QColorGroup object receiving the color information to be used
   * @param brush ref to QBrush object receiveing the brush information to be used
   */
  virtual bool paintRegisterCellSetup(QPainter* painter, int& row, int& col, QRect& cellRect, QRect& textRect, QColorGroup& cg, QBrush& brush);

  bool isSelectable(void) const {
    return true;
  }
  bool canHaveFocus(void) const {
    return true;
  }
  virtual bool isScheduled(void) const {
    return true;
  }

  virtual int sortSamePostDate(void) const {
    return 4;
  }

//   virtual void paintRegisterGrid(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& cg) const;

//   void registerCellText(QString& txt, int& align, int row, int col, QPainter* painter = 0);

private:
  unsigned int         m_drawCounter;
};

} // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2:

