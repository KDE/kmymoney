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

#include <qwidget.h>
#include <q3table.h>
#include <qwidget.h>
#include <QPointer>
//Added by qt3to4:
#include <QResizeEvent>
#include <QEvent>
#include <QMouseEvent>
#include <Q3ValueList>

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

/**
  * @author Thomas Baumgart
  */
class kMyMoneySplitTable : public Q3Table
{
  Q_OBJECT
public:
  kMyMoneySplitTable(QWidget *parent=0, const char *name=0);
  virtual ~kMyMoneySplitTable();

  void paintCell(QPainter *p, int row, int col, const QRect& r, bool /*selected*/);
  void paintFocus(QPainter *p, const QRect &cr);

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
  const MyMoneyTransaction& transaction(void) const { return m_transaction; }

  /**
    * Returns a list of MyMoneySplit objects. It contains all but the one
    * referencing the account passed in setTransaction().
    *
    * @param t reference to transaction
    * @return list of splits
    */
  const Q3ValueList<MyMoneySplit> getSplits(const MyMoneyTransaction& t) const;

  void setup(const QMap<QString, MyMoneyMoney>& priceInfo);

protected:
  void contentsMousePressEvent( QMouseEvent* e );
  void contentsMouseReleaseEvent( QMouseEvent* e );
  void contentsMouseDoubleClickEvent( QMouseEvent* e );
  bool eventFilter(QObject *o, QEvent *e);
  void endEdit(int row, int col, bool accept, bool replace );

  void resizeEvent(QResizeEvent*);
  QWidget* createEditWidgets(void);
  void destroyEditWidgets(void);

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

  /**
    * convenience function for setCurrentCell(int row, int col)
    */
  void setCurrentCell(int row) { setCurrentCell(row, 0); }

  void updateTransactionTableSize(void);

  /**
    * This method returns the current state of the inline editing mode
    *
    * @return true if inline edit mode is on, false otherwise
    */
  bool isEditMode(void) const;

  /**
    * This method retuns the background color for a given @p row.
    *
    * @param row the row in question
    * @return the color as QColor object
    */
  const QColor rowBackgroundColor(const int row) const;

  void endEdit(bool keyboardDriven);

public slots:
  /** No descriptions */
  virtual void setCurrentCell(int row, int col);

  virtual void setNumRows(int r);

  QWidget* slotStartEdit(void);
  void slotEndEdit(void);
  void slotEndEditKeyboard(void);
  void slotDeleteSplit(void);
  void slotCancelEdit(void);
  void slotDuplicateSplit(void);

protected slots:
  virtual void columnWidthChanged(int col);

  /// move the focus to the selected @p row.
  void slotSetFocus(int row, int col = 0, int button = Qt::LeftButton, const QPoint & mousePos = QPoint(0, 0));

  /**
    * Calling this slot refills the widget with the data
    * passed in the argument @p t.
    *
    * @param t reference to transaction data
    */
  void slotUpdateData(const MyMoneyTransaction& t);

  void slotLoadEditWidgets(void);

signals:
  /**
    * This signal is emitted whenever the return key is pressed
    * and the widget is not in edit mode.
    */
  void escapePressed(void);

  /**
    * This signal is emitted whenever the return key is pressed
    * and the widget is not in edit mode.
    */
  void returnPressed(void);

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

  /// indication if inline editing mode is on or not
  bool                m_editMode;

  MyMoneyTransaction  m_transaction;
  MyMoneyAccount      m_account;
  MyMoneySplit        m_split;
  MyMoneySplit        m_hiddenSplit;

  unsigned            m_amountWidth;

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
