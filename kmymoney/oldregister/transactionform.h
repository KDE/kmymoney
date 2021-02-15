/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRANSACTIONFORM_H
#define TRANSACTIONFORM_H

#include "kmm_oldregister_export.h"

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
  class KMM_OLDREGISTER_EXPORT TransactionForm : public TransactionEditorContainer
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

  public Q_SLOTS:
    void slotSetTransaction(KMyMoneyRegister::Transaction* item);
    void resize(int col);

  protected Q_SLOTS:
    /**
    * Helper method to convert @a int into @a KMyMoneyRegister::Action
    */
    void slotActionSelected(int);

  Q_SIGNALS:
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
