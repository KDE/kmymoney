/***************************************************************************
                          kmymoneysplittable.h  -  description
                             -------------------
    begin                : Thu Jan 10 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYSPLITTABLE_H
#define KMYMONEYSPLITTABLE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QTableWidget>
#include <QWidget>
#include <QPointer>
#include <QResizeEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

class KMenu;
class KPushButton;
class QFrame;
// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"
#include "mymoneyaccount.h"

class KMyMoneyCategory;
class kMyMoneyLineEdit;
class kMyMoneyEdit;
class KTagContainer;

/**
  * @author Thomas Baumgart
  */
class kMyMoneySplitTable : public QTableWidget
{
  Q_OBJECT
public:
  explicit kMyMoneySplitTable(QWidget *parent = 0);
  virtual ~kMyMoneySplitTable();

  /**
    * This method is used to load the widget with the information about
    * the transaction @p t. The split referencing the account @p acc is
    * not shown in the widget.
    *
    * @param t reference to transaction to be shown/modified
    * @param s reference to split that is to be hidden
    * @param acc reference to account
    */
  void setTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s, const MyMoneyAccount& acc);

  /**
    * This method is used to retrieve the transaction from the widget.
    */
  const MyMoneyTransaction& transaction() const {
    return m_transaction;
  }

  /**
    * Returns a list of MyMoneySplit objects. It contains all but the one
    * referencing the account passed in setTransaction().
    *
    * @param t reference to transaction
    * @return list of splits
    */
  const QList<MyMoneySplit> getSplits(const MyMoneyTransaction& t) const;

  void setup(const QMap<QString, MyMoneyMoney>& priceInfo, int precision);

  int currentRow() const;

protected:
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseDoubleClickEvent(QMouseEvent* e);
  bool eventFilter(QObject *o, QEvent *e);

  void resizeEvent(QResizeEvent*);
  KMyMoneyCategory* createEditWidgets(bool setFocus);
  void destroyEditWidgets();
  void destroyEditWidget(int r, int c);

  /**
    * This method handles the focus of the keyboard. When in edit mode
    * (m_editCategory widget is visible) the keyboard focus is handled
    * according to the widgets that are referenced in m_tabOrderWidgets.
    * If not in edit mode, the base class functionality is provided.
    *
    * @param next true if forward-tab, false if backward-tab was
    *             pressed by the user
    */
  virtual bool focusNextPrevChild(bool next);
  void addToTabOrder(QWidget* w);

  void updateTransactionTableSize();

  /**
    * This method returns the current state of the inline editing mode
    *
    * @return true if inline edit mode is on, false otherwise
    */
  bool isEditMode() const;

  /**
    * This method returns true if the currently edited split is valid
    * and can be entered.
    *
    * @return true if the split can be entered, false otherwise
    */
  bool isEditSplitValid() const;

  void endEdit(bool keyboardDriven, bool setFocusToNextRow = true);

public slots:
  /** No descriptions */
  virtual void setRowCount(int r);

  void selectRow(int row);

  KMyMoneyCategory* slotStartEdit();
  void slotEndEdit();
  void slotEndEditKeyboard();
  void slotDeleteSplit();
  void slotCancelEdit();
  void slotDuplicateSplit();

protected slots:
  /// move the focus to the selected @p row.
  void slotSetFocus(const QModelIndex& index, int button = Qt::LeftButton);

  /**
    * Calling this slot refills the widget with the data
    * passed in the argument @p t.
    *
    * @param t reference to transaction data
    */
  void slotUpdateData(const MyMoneyTransaction& t);

  void slotLoadEditWidgets();
  void slotCreateTag(const QString &txt, QString &id);
  void slotUpdateTag(QString &id);

signals:
  /**
    * This signal is emitted whenever the widget goes into edit mode.
    */
  void editStarted();

  /**
    * This signal is emitted whenever the widget ends edit mode.
    */
  void editFinished();

  /**
    * This signal is emitted whenever the return key is pressed
    * and the widget is not in edit mode.
    */
  void escapePressed();

  /**
    * This signal is emitted whenever the return key is pressed
    * and the widget is not in edit mode.
    */
  void returnPressed();

  /**
    * This signal is emitted whenever the transaction data has been changed
    *
    * @param t modified transaction data
    */
  void transactionChanged(const MyMoneyTransaction& t);

  /**
    * This signal is sent out, when a new category needs to be created
    * @sa KMyMoneyCombo::createItem()
    *
    * @param txt The name of the category to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createCategory(const QString& txt, QString& id);

  /**
    * This signal is sent out, when a new tag needs to be created
    * @sa KMyMoneyCombo::createItem()
    *
    * @param txt The name of the tag to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createTag(const QString& txt, QString& id);

  /**
    * Signal is emitted, if any of the widgets enters (@a state equals @a true)
    *  or leaves (@a state equals @a false) object creation mode.
    *
    * @param state Enter (@a true) or leave (@a false) object creation
    */
  void objectCreation(bool state);

private:
  /// the currently selected row (will be printed as selected)
  int                 m_currentRow;

  /// the number of rows filled with data
  int                 m_maxRows;

  MyMoneyTransaction  m_transaction;
  MyMoneyAccount      m_account;
  MyMoneySplit        m_split;
  MyMoneySplit        m_hiddenSplit;

  /**
    * This member keeps the precision for the values
    */
  int                 m_precision;

  /**
    * This member keeps a pointer to the context menu
    */
  KMenu*         m_contextMenu;

  /// keeps the QAction of the delete entry in the context menu
  QAction*       m_contextMenuDelete;

  /// keeps the QAction of the duplicate entry in the context menu
  QAction*       m_contextMenuDuplicate;

  /**
    * This member contains a pointer to the input widget for the category.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  QPointer<KMyMoneyCategory> m_editCategory;

  /**
    * This member contains a pointer to the input widget for the memo.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  QPointer<kMyMoneyLineEdit> m_editMemo;

  /**
    * This member contains a pointer to the input widget for the memo.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  QPointer<KTagContainer> m_editTag;

  /**
    * This member contains a pointer to the input widget for the amount.
    * The widget will be created and destroyed dynamically in createInputWidgets()
    * and destroyInputWidgets().
    */
  QPointer<kMyMoneyEdit>     m_editAmount;

  /**
    * This member keeps the tab order for the above widgets
    */
  QWidgetList         m_tabOrderWidgets;

  QPointer<QFrame>           m_registerButtonFrame;
  QPointer<KPushButton>      m_registerEnterButton;
  QPointer<KPushButton>      m_registerCancelButton;

  QMap<QString, MyMoneyMoney>  m_priceInfo;
};

#endif
