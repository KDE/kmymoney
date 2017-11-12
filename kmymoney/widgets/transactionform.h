/***************************************************************************
                             transactionform.h
                             ----------
    begin                : Sun May 14 2006
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

#ifndef TRANSACTIONFORM_H
#define TRANSACTIONFORM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyleOptionViewItem>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "transactioneditorcontainer.h"

class MyMoneyAccount;

namespace KMyMoneyRegister { class Transaction; }
namespace eWidgets { namespace eRegister { enum class Action; }
                     namespace eTransactionForm { enum class Column; } }
namespace KMyMoneyTransactionForm
{
  class TabBar;
  /**
  * @author Thomas Baumgart
  */
  class TransactionFormPrivate;
  class TransactionForm : public TransactionEditorContainer
  {
    Q_OBJECT
    Q_DISABLE_COPY(TransactionForm)

  public:
    explicit TransactionForm(QWidget* parent = nullptr);
    ~TransactionForm();

    /**
    * Override the QTable member function to avoid display of focus
    */
    void paintFocus(QPainter* /*p*/, const QRect& /*cr*/);

    void adjustColumn(eWidgets::eTransactionForm::Column col);
    void clear();

    void paintCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);

    void arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t)  override;
    void removeEditWidgets(QMap<QString, QWidget*>& editWidgets) override;
    void tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const override;

    /**
    * reimplemented to prevent normal cell selection behavior
    */
    void setCurrentCell(int, int);

    TabBar* getTabBar(QWidget* parent = nullptr);

    void setupForm(const MyMoneyAccount& acc);

    void enableTabBar(bool b);

  protected:

    /**
    * reimplemented to prevent normal mouse press behavior
    */
    void contentsMousePressEvent(QMouseEvent* ev);

    /**
    * reimplemented to prevent normal mouse move behavior
    */
    void contentsMouseMoveEvent(QMouseEvent* ev);

    /**
    * reimplemented to prevent normal mouse release behavior
    */
    void contentsMouseReleaseEvent(QMouseEvent* ev);

    /**
    * reimplemented to prevent normal mouse double click behavior
    */
    void contentsMouseDoubleClickEvent(QMouseEvent* ev);

    /**
    * reimplemented to prevent normal keyboard behavior
    */
    void keyPressEvent(QKeyEvent* ev) override;

    /**
    * Override logic and use standard QFrame behaviour
    */
    bool focusNextPrevChild(bool next) override;

  public slots:
    void slotSetTransaction(KMyMoneyRegister::Transaction* item);
    void resize(int col);

  protected slots:
    /**
    * Helper method to convert @a int into @a KMyMoneyRegister::Action
    */
    void slotActionSelected(int);

  signals:
    /**
    * This signal is emitted when a user selects a tab. @a id
    * contains the tab's id (e.g. KMyMoneyRegister::ActionDeposit)
    */
    void newTransaction(eWidgets::eRegister::Action id);

  private:
    TransactionFormPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(TransactionForm)
  };
} // namespace

#endif
