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

#include <qglobal.h>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QDate;
class QPainter;
class QPoint;
class QRect;
class QStyleOptionViewItem;
class QModelIndex;

class MyMoneyMoney;

namespace eMyMoney { namespace Split { enum class State; } }

namespace KMyMoneyRegister
{

 enum CashFlowDirection : int {
  Deposit = 0,          //< transaction is deposit
  Payment,              //< transaction is payment
  Unknown               //< transaction cashflow is unknown
};

  enum Action : int {
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
};

/**
  * Used to filter items from the register.
  */
struct RegisterFilter {
  enum ItemState {
    Any,
    Imported,
    Matched,
    Erroneous,
    NotMarked,
    NotReconciled,
    Cleared
  };
  RegisterFilter(const QString &t, ItemState s) : state(s), text(t) {
  }
  ItemState state;
  QString text;
};

class Register;

/**
  * @author Thomas Baumgart
  */
class RegisterItemPrivate;
class RegisterItem
{
public:
  RegisterItem();
  explicit RegisterItem(Register* getParent);
  RegisterItem(const RegisterItem & other);
  RegisterItem(RegisterItem && other);
  friend void swap(RegisterItem& first, RegisterItem& second);

  virtual ~RegisterItem();

  virtual const char* className() = 0;

  virtual bool isSelectable() const = 0;
  virtual bool isSelected() const;
  virtual void setSelected(bool /* selected*/);

  virtual bool canHaveFocus() const = 0;
  virtual bool hasFocus() const;
  virtual bool hasEditorOpen() const;

  virtual void setFocus(bool /*focus*/, bool updateLens = true);

  virtual bool isErroneous() const = 0;

  // helper functions used for sorting
  virtual QDate sortPostDate() const;
  virtual int sortSamePostDate() const = 0;
  virtual QDate sortEntryDate() const;
  virtual const QString& sortPayee() const;
  virtual MyMoneyMoney sortValue() const;
  virtual QString sortNumber() const;
  virtual const QString& sortEntryOrder() const;
  virtual CashFlowDirection sortType() const;
  virtual const QString& sortCategory() const;
  virtual eMyMoney::Split::State sortReconcileState() const;
  virtual const QString sortSecurity() const;

  /**
    * This method sets the row offset of the item in the register
    * to row.
    *
    * @param row row offset
    *
    * @note The row offset is based on QTable rows, not register
    * items.
    */
  virtual void setStartRow(int row);
  int startRow() const;
  virtual int rowHeightHint() const;

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
  virtual void setNumRowsForm(int rows);

  /**
    * This method returns the number of rows required to display this item
    * in a Register
    */
  virtual int numRowsRegister() const;

  /**
    * This method returns the number of rows required to display this item
    * in a Form
    */
  virtual int numRowsForm() const;
  virtual int numColsForm() const;

  /**
    * This method sets up the register item to be shown in normal (@p alternate = @p false)
    * or alternate (@p alternate = @p true) background.
    *
    * @param alternate selects normal or alternate background
    */
  virtual void setAlternate(bool alternate);

  virtual void paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) = 0;
  virtual void paintFormCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) = 0;

  virtual const QString& id() const;

  /**
    * Sets the parent of this item to be the register @p parent
    *
    * @param parent pointer to register
    */
  void setParent(Register* getParent);

  /**
    * This member returns a pointer to the parent object
    *
    * @retval pointer to Register
    */
  Register* getParent() const;

  void setNeedResize();

  bool isVisible() const;

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

  void setNextItem(RegisterItem* p);
  void setPrevItem(RegisterItem* p);
  RegisterItem* nextItem() const;
  RegisterItem* prevItem() const;

  virtual bool matches(const RegisterFilter&) const = 0;

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
  virtual bool maybeTip(const QPoint& /* relpos */, int /* row */, int /* col */, QRect& /* r */, QString& /* msg */);

protected:
  RegisterItemPrivate *d_ptr;
  RegisterItem(RegisterItemPrivate &dd, Register *parent);
  RegisterItem(RegisterItemPrivate &dd);  //for copy-constructor of derived class

private:
  Q_DECLARE_PRIVATE(RegisterItem)
};

inline void swap(RegisterItem& first, RegisterItem& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
}

inline RegisterItem::RegisterItem(RegisterItem && other) : RegisterItem() // krazy:exclude=inline
{
  swap(*this, other);
}

} // namespace

#endif
