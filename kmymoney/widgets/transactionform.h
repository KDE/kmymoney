/***************************************************************************
                             transactionform.h
                             ----------
    begin                : Sun May 14 2006
    copyright            : (C) 2006 by Thomas Baumgart
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

#ifndef TRANSACTIONFORM_H
#define TRANSACTIONFORM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <Q3Table>
#include <Q3ValueVector>
#include <QPalette>
#include <QWidget>
#include <QMouseEvent>
#include <QKeyEvent>

// ----------------------------------------------------------------------------
// KDE Includes
#include <ktabwidget.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneyobject.h>
#include <register.h>

#include "kmymoneysettings.h"


namespace KMyMoneyTransactionForm
{

/**
  * @author Thomas Baumgart
  */
class TabBar : public KTabWidget
{
  Q_OBJECT
public:
  typedef enum {
    SignalNormal = 0,      // standard signal behaviour
    SignalNever,           // don't signal selection of a tab at all
    SignalAlways           // always signal selection of a tab
  } SignalEmissionE;

  explicit TabBar(QWidget* parent = 0);
  virtual ~TabBar() {}

  SignalEmissionE setSignalEmission(SignalEmissionE type);

  void copyTabs(const TabBar* otabbar);

  void insertTab(int id, QWidget* tab, QString title = QString());

  void setIdentifier(QWidget* tab, int newId);

  QWidget* widget(int id) const;


  int currentIndex(void) const;

public slots:

  /**
    * overridden for internal reasons, API not changed
    */
  virtual void setCurrentIndex(int);

  /**
    * overridden for internal reasons, API not changed
    */
  virtual void setCurrentWidget(QWidget *);

  /**
    * overridden for internal reasons, API not changed
    */
  virtual void showEvent(QShowEvent* event);

protected slots:
  void slotTabCurrentChanged(int id);

signals:
  void tabCurrentChanged(int id);

private:
  SignalEmissionE    m_signalType;

  /**
    * maps our internal action ids to those used by
    * qt3. Since it does not seem possible to tell
    * qt3 to use our ids everywhere (in QAccel) we
    * need to know which is which
    */
  QMap<int, int>     m_idMap;
};

typedef enum {
  LabelColumn1 = 0,
  ValueColumn1,
  LabelColumn2,
  ValueColumn2,
  // insert new values above this line
  MaxColumns
} Column;

class TransactionForm;
class TransactionFormItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit TransactionFormItemDelegate(TransactionForm *parent);
  ~TransactionFormItemDelegate();

   void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;


private:
  TransactionForm *m_transactionForm;
};

/**
  * @author Thomas Baumgart
  */
class TransactionForm : public TransactionEditorContainer
{
  Q_OBJECT
public:
  explicit TransactionForm(QWidget *parent = 0);
  virtual ~TransactionForm() {}

  /**
    * Override the QTable member function to avoid display of focus
    */
  void paintFocus(QPainter* /*p*/, const QRect& /*cr*/) {}

  void adjustColumn(Column col);
  void clear(void);

  void paintCell(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);

  void resize(int col);

  void arrangeEditWidgets(QMap<QString, QWidget*>& editWidgets, KMyMoneyRegister::Transaction* t);
  void removeEditWidgets(QMap<QString, QWidget*>& editWidgets);
  void tabOrder(QWidgetList& tabOrderWidgets, KMyMoneyRegister::Transaction* t) const;

  /**
    * reimplemented to prevent normal cell selection behavior
    */
  void setCurrentCell(int, int) {}

  TabBar* tabBar(QWidget* parent = 0);

  void setupForm(const MyMoneyAccount& acc);

  void enableTabBar(bool b);

protected:

  /**
    * reimplemented to prevent normal mouse press behavior
    */
  void contentsMousePressEvent(QMouseEvent* ev) {
    ev->ignore();
  }

  /**
    * reimplemented to prevent normal mouse move behavior
    */
  void contentsMouseMoveEvent(QMouseEvent* ev) {
    ev->ignore();
  }

  /**
    * reimplemented to prevent normal mouse release behavior
    */
  void contentsMouseReleaseEvent(QMouseEvent* ev) {
    ev->ignore();
  }

  /**
    * reimplemented to prevent normal mouse double click behavior
    */
  void contentsMouseDoubleClickEvent(QMouseEvent* ev) {
    ev->ignore();
  }

  /**
    * reimplemented to prevent normal keyboard behavior
    */
  void keyPressEvent(QKeyEvent* ev) {
    ev->ignore();
  }

  /**
    * Override logic and use standard QFrame behaviour
    */
  bool focusNextPrevChild(bool next);

public slots:
  void slotSetTransaction(KMyMoneyRegister::Transaction* item);

protected slots:
  void resize(void);

  /**
    * Helper method to convert @a int into @a KMyMoneyRegister::Action
    */
  void slotActionSelected(int);

signals:
  /**
    * This signal is emitted when a user selects a tab. @a id
    * contains the tab's id (e.g. KMyMoneyRegister::ActionDeposit)
    */
  void newTransaction(KMyMoneyRegister::Action id);

protected:
  KMyMoneyRegister::Transaction*       m_transaction;
  QColorGroup                          m_cellColorGroup;
  TabBar*                              m_tabBar;
  TransactionFormItemDelegate         *m_itemDelegate;
};


} // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
