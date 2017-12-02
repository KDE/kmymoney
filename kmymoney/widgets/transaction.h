/***************************************************************************
                          transaction.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
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

#ifndef TRANSACTION_H
#define TRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidgetList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "registeritem.h"

class QWidget;
class QPalette;
class QFontMetrics;
class QTableWidget;
class TransactionEditor;
class TransactionEditorContainer;

class MyMoneySplit;
class MyMoneyTransaction;

template <class Key, class Value> class QMap;

namespace KMyMoneyTransactionForm { class TransactionForm; }
namespace eWidgets { namespace eRegister { enum class Action; } }

namespace KMyMoneyRegister
{
  class SelectedTransactions;
  // keep the following list in sync with code in the constructor
  // of KMyMoneyRegister::Register in register.cpp

  class TransactionPrivate;
  class Transaction : public RegisterItem
  {
    Q_DISABLE_COPY(Transaction)

  public:
    explicit Transaction(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    virtual ~Transaction();

    virtual const char* className() override;
    bool isSelectable() const override;
    bool isSelected() const override;
    void setSelected(bool selected) override;
    bool canHaveFocus() const override;
    bool hasFocus() const override;
    bool hasEditorOpen() const override;
    virtual bool isScheduled() const;
    void setFocus(bool focus, bool updateLens = true) override;
    bool isErroneous() const override;
    QDate sortPostDate() const override;
    virtual int sortSamePostDate() const override;
    QDate sortEntryDate() const override;
    virtual const QString& sortPayee() const override;
    virtual const QList<QString>& sortTagList() const;
    MyMoneyMoney sortValue() const override;
    QString sortNumber() const override;
    virtual const QString& sortEntryOrder() const override;
    virtual eWidgets::eRegister::CashFlowDirection sortType() const override;
    virtual const QString& sortCategory() const override;
    virtual eMyMoney::Split::State sortReconcileState() const override;
    virtual QString id() const override;
    const MyMoneyTransaction& transaction() const;
    const MyMoneySplit& split() const;
    void setBalance(const MyMoneyMoney& balance);
    const MyMoneyMoney& balance() const;

    virtual int rowHeightHint() const override ;

    virtual bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index);
    virtual void paintRegisterCell(QPainter* painter, QStyleOptionViewItem& option, const QModelIndex& index) override;

    virtual void paintFormCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    virtual bool formCellText(QString& /* txt */, Qt::Alignment& /* align */, int /* row */, int /* col */, QPainter* /* painter */);
    virtual void registerCellText(QString& /* txt */, Qt::Alignment& /* align */, int /* row */, int /* col */, QPainter* /* painter */);
    virtual int registerColWidth(int /* col */, const QFontMetrics& /* cellFontMetrics */);

    /**
    * Helper method for the above method.
    */
    void registerCellText(QString& txt, int row, int col);

    virtual int formRowHeight(int row);
    virtual int formRowHeight() const;

    virtual void setupForm(KMyMoneyTransactionForm::TransactionForm* form);
    virtual void setupFormPalette(QMap<QString, QWidget*>& editWidgets);
    virtual void setupRegisterPalette(QMap<QString, QWidget*>& editWidgets);
    virtual void loadTab(KMyMoneyTransactionForm::TransactionForm* form) = 0;

    virtual void arrangeWidgetsInForm(QMap<QString, QWidget*>& editWidgets) = 0;
    virtual void arrangeWidgetsInRegister(QMap<QString, QWidget*>& editWidgets) = 0;
    virtual void tabOrderInForm(QWidgetList& tabOrderWidgets) const = 0;
    virtual void tabOrderInRegister(QWidgetList& tabOrderWidgets) const = 0;

    virtual eWidgets::eRegister::Action actionType() const = 0;

    QWidget* focusWidget(QWidget*) const;
    void arrangeWidget(QTableWidget* tbl, int row, int col, QWidget* w) const;

    bool haveNumberField() const;

    bool matches(const RegisterFilter&) const override;

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
    virtual bool maybeTip(const QPoint& relpos, int row, int col, QRect& r, QString& msg) override;

    /**
    * This method returns the number of register rows required for a certain
    * item in expanded (@p expanded equals @a true) or collapsed (@p expanded
    * is @a false) mode.
    *
    * @param expanded returns number of maximum rows required for this item to
    *                 display all information (used for ledger lens and register
    *                 edit mode) or the minimum number of rows required.
    * @return number of rows required for mode selected by @p expanded
    */
    virtual int numRowsRegister(bool expanded) const = 0;

    /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
    int numRowsRegister() const override;

    void leaveEditMode();
    void startEditMode();

    /**
    * This method creates an editor for the transaction
    */
    virtual TransactionEditor* createEditor(TransactionEditorContainer* regForm, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate) = 0;

    virtual void setVisible(bool visible) override;

    virtual void setShowBalance(bool showBalance);

    /**
    * Return information if @a row should be shown (@a true )
    * or hidden (@a false ) in the form. Default is true.
    */
    virtual bool showRowInForm(int row) const;

    /**
    * Control visibility of @a row in the transaction form.
    * Only row 0 has an effect, others return @a true.
    */
    virtual void setShowRowInForm(int row, bool show);

    virtual void setReducedIntensity(bool reduced);

  protected:
    /**
    * This method converts m_split.reconcileFlag() into a readable string
    *
    * @param text Return textual representation e.g. "Cleared" (@a true) or just
    *             a flag e.g. "C" (@a false). Defaults to textual representation.
    * @return Textual representation or flag as selected via @p text of the
    *         reconciliation state of the split
    */
    QString reconcileState(bool text = true) const;

    /**
    * Helper method to reduce a multi line memo text into a single line.
    *
    * @param txt QString that will receive the single line memo text
    * @param split const reference to the split to take the memo from
    */
    void singleLineMemo(QString& txt, const MyMoneySplit& split) const;

    virtual void setupPalette(const QPalette& palette, QMap<QString, QWidget*>& editWidgets);

    TransactionPrivate *d_ptr;
    Transaction(TransactionPrivate &dd, Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    Transaction(TransactionPrivate &dd); //for copy-constructor of derived class

  private:
    Q_DECLARE_PRIVATE(Transaction)
  };
} // namespace

#endif
