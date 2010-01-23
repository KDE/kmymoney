/***************************************************************************
                          stdtransactiondownloaded.h
                             -------------------
    begin                : Sun May 11 2008
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

#ifndef STDTRANSACTIONDOWNLOADED_H
#define STDTRANSACTIONDOWNLOADED_H

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

class StdTransactionDownloaded : public StdTransaction
{
public:
  StdTransactionDownloaded(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~StdTransactionDownloaded() {}

  virtual const char* className(void) {
    return "StdTransactionDownloaded";
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

#if 0
  virtual void paintRegisterCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

  bool formCellText(QString& txt, int& align, int row, int col, QPainter* painter = 0);
  void registerCellText(QString& txt, int& align, int row, int col, QPainter* painter = 0);

  int numColsForm(void) const {
    return 4;
  }

  void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets);
  void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets);
  void tabOrderInForm(QWidgetList& tabOrderWidgets) const;
  void tabOrderInRegister(QWidgetList& tabOrderWidgets) const;

  int numRowsRegister(bool expanded) const;
#endif

  /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
  int numRowsRegister(void) const {
    return StdTransaction::numRowsRegister();
  }
};

class InvestTransactionDownloaded : public InvestTransaction
{
public:
  InvestTransactionDownloaded(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~InvestTransactionDownloaded() {}

  virtual const char* className(void) {
    return "InvestTransactionDownloaded";
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
  /**
     * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
   */
  int numRowsRegister(void) const {
    return InvestTransaction::numRowsRegister();
  }
};


} // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2:

