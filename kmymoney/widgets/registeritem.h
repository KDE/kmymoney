/***************************************************************************
                          registeritem.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
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

#ifndef REGISTERITEM_H
#define REGISTERITEM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDateTime>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneymoney.h>
#include <mymoneysplit.h>
#include <mymoneyobject.h>

namespace KMyMoneyRegister
{

typedef enum {
  Deposit = 0,          //< transaction is deposit
  Payment,              //< transaction is payment
  Unknown               //< transaction cashflow is unknown
} CashFlowDirection;

typedef enum {
  ActionNone = -1,
  ActionCheck = 0,
  /* these should be values which qt 3.3 never uses for QTab:
   * qt starts upwards from 0
   */
  ActionDeposit = 12201,
  ActionTransfer = 12202,
  ActionWithdrawal = 12203,
  ActionAtm,
  // insert new values above this line
  MaxAction
} Action;


class Register;

/**
  * @author Thomas Baumgart
  */
class RegisterItem
{
public:
  RegisterItem();
  RegisterItem(Register* parent);
  virtual ~RegisterItem();

  virtual const char* className(void) = 0;

  virtual bool isSelectable(void) const = 0;
  virtual bool isSelected(void) const {
    return false;
  }
  virtual void setSelected(bool /* selected*/) {}

  virtual bool canHaveFocus(void) const = 0;
  virtual bool hasFocus(void) const {
    return false;
  }
  virtual bool hasEditorOpen(void) const {
    return false;
  }

  virtual void setFocus(bool /*focus*/, bool updateLens = true) {
    Q_UNUSED(updateLens);
  }

  virtual bool isErroneous(void) const = 0;

  // helper functions used for sorting
  virtual const QDate sortPostDate(void) const {
    return nullDate;
  }
  virtual int sortSamePostDate(void) const = 0;
  virtual const QDate sortEntryDate(void) const {
    return nullDate;
  }
  virtual const QString sortPayee(void) const {
    return QString();
  }
  virtual const MyMoneyMoney sortValue(void) const {
    return nullValue;
  }
  virtual const QString sortNumber(void) const {
    return QString();
  }
  virtual const QString sortEntryOrder(void) const {
    return QString();
  }
  virtual CashFlowDirection sortType(void) const {
    return Deposit;
  }
  virtual const QString sortCategory(void) const {
    return QString();
  }
  virtual MyMoneySplit::reconcileFlagE sortReconcileState(void) const {
    return MyMoneySplit::MaxReconcileState;
  }
  virtual const QString sortSecurity(void) const {
    return QString();
  }

  /**
    * This method sets the row offset of the item in the register
    * to row.
    *
    * @param row row offset
    *
    * @note The row offset is based on QTable rows, not register
    * items.
    */
  virtual void setStartRow(int row) {
    m_startRow = row;
  }
  int startRow(void) const {
    return m_startRow;
  }
  virtual int rowHeightHint(void) const;

  /**
    * This method modifies the number of rows required to display this item
    * in a Register.
    * It calls Register::forceUpdateLists() when the number differs.
    */
  virtual void setNumRowsRegister(int rows);

  /**
    * This method modifies the number of rows required to display this item
    * in a Form.
    */
  virtual void setNumRowsForm(int rows) {
    m_rowsForm = rows;
  }

  /**
    * This method returns the number of rows required to display this item
    * in a Register
    */
  virtual int numRowsRegister(void) const {
    return m_rowsRegister;
  }

  /**
    * This method returns the number of rows required to display this item
    * in a Form
    */
  virtual int numRowsForm(void) const {
    return m_rowsForm;
  }
  virtual int numColsForm(void) const {
    return 1;
  }

  /**
    * This method sets up the register item to be shown in normal (@p alternate = @p false)
    * or alternate (@p alternate = @p true) background.
    *
    * @param alternate selects normal or alternate background
    */
  virtual void setAlternate(bool alternate) {
    m_alternate = alternate;
  }

  virtual void paintRegisterCell(QPainter *painter, QStyleOptionViewItemV4 &option, const QModelIndex &index) = 0;
  virtual void paintFormCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) = 0;

  virtual const QString& id(void) const {
    return MyMoneyObject::emptyId();
  }

  /**
    * Sets the parent of this item to be the register @p parent
    *
    * @param parent pointer to register
    */
  void setParent(Register* parent);

  /**
    * This member returns a pointer to the parent object
    *
    * @retval pointer to Register
    */
  Register* parent(void) const {
    return m_parent;
  }

  void setNeedResize(void) {
    m_needResize = true;
  }

  bool isVisible(void) const {
    return m_visible;
  }

  /**
    * Marks the item visible depending on @a visible and
    * updates the underlying register object
    */
  virtual void setVisible(bool visible);

  /**
    * Marks the item visible depending on @a visible but
    * does not update the underlying register object. Returns
    * true, if visibility has changed.
    */
  virtual bool markVisible(bool visible);

  void setNextItem(RegisterItem* p) {
    m_next = p;
  }
  void setPrevItem(RegisterItem* p) {
    m_prev = p;
  }
  RegisterItem* nextItem(void) const {
    return m_next;
  }
  RegisterItem* prevItem(void) const {
    return m_prev;
  }

  virtual bool matches(const QString&) const = 0;

  /**
    * Checks if the mouse hovered over an area that has a tooltip associated with it.
    * The mouse position is given in relative coordinates to the @a startRow and the
    * @a row and @a col of the item are also passed as relative values.
    *
    * If a tooltip shall be shown, this method presets the rectangle @a r with the
    * area in register coordinates and @a msg with the string that will be passed
    * to QToolTip::tip. @a true is returned in this case.
    *
    * If no tooltip is available, @a false will be returned.
    */
  virtual bool maybeTip(const QPoint& /* relpos */, int /* row */, int /* col */, QRect& /* r */, QString& /* msg */) {
    return false;
  }

protected:
  /// This method serves as helper for all constructors
  void init(void);

protected:
  Register*                m_parent;
  RegisterItem*            m_prev;
  RegisterItem*            m_next;
  int                      m_startRow;
  int                      m_rowsRegister;
  int                      m_rowsForm;
  bool                     m_alternate;
  bool                     m_needResize;
  bool                     m_visible;

private:
  static QDate             nullDate;
  static MyMoneyMoney      nullValue;
};

} // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
