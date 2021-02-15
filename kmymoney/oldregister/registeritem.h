/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef REGISTERITEM_H
#define REGISTERITEM_H

#include "kmm_oldregister_export.h"

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
namespace eWidgets { namespace eRegister { enum class CashFlowDirection; } }

namespace KMyMoneyRegister
{
  struct RegisterFilter;
  class Register;

  /**
  * @author Thomas Baumgart
  */
  class RegisterItemPrivate;
  class KMM_OLDREGISTER_EXPORT RegisterItem
  {
    Q_DISABLE_COPY(RegisterItem)

  public:
    explicit RegisterItem(Register* getParent);
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
    virtual eWidgets::eRegister::CashFlowDirection sortType() const;
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

    virtual QString id() const;

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
    RegisterItemPrivate * const d_ptr;
    RegisterItem(RegisterItemPrivate &dd, Register *parent);

  private:
    Q_DECLARE_PRIVATE(RegisterItem)
  };
} // namespace

#endif
