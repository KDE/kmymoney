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

#include <q3table.h>
#include <q3valuelist.h>
#include <q3valuevector.h>
#include <qpalette.h>
#include <qwidget.h>
#include <qtabbar.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyobject.h>
#include <kmymoney/register.h>

#include "../kmymoneysettings.h"

class MyMoneyObjectContainer;

namespace KMyMoneyTransactionForm {

/**
  * @author Thomas Baumgart
  */
class TabBar : public QTabBar
{
  Q_OBJECT
public:
  typedef enum {
    SignalNormal = 0,      // standard signal behaviour
    SignalNever,           // don't signal selection of a tab at all
    SignalAlways           // always signal selection of a tab
  } SignalEmissionE;

  TabBar(QWidget* parent = 0, const char* name = 0);
  virtual ~TabBar() {}

  SignalEmissionE setSignalEmission(SignalEmissionE type);

  void copyTabs(const TabBar* otabbar);

  void addTab(QTab* tab, int id);

  void setIdentifier(QTab* tab, int newId);

  QTab* tab(int id) const;

  int currentTab(void) const;

public slots:
  /**
    * overridden for internal reasons, API not changed
    */
  virtual void setCurrentTab( int );

  /**
    * overridden for internal reasons, API not changed
    */
  virtual void setCurrentTab( QTab * );

  /**
    * overridden for internal reasons, API not changed
    */
  virtual void show(void);

protected slots:
  void slotTabSelected(int id);

signals:
  void tabSelected(int id);

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

/**
  * @author Thomas Baumgart
  */
class TransactionForm : public TransactionEditorContainer
{
  Q_OBJECT
public:
  TransactionForm(QWidget *parent = 0, const char *name = 0);
  virtual ~TransactionForm() {}

  /**
    * Override the QTable member function to avoid display of focus
    */
  void paintFocus(QPainter* /*p*/, const QRect& /*cr*/ ) {}

  QSize tableSize(void) const;
  QSize sizeHint(void) const;
  void adjustColumn(Column col);
  void clear(void);

  void paintCell(QPainter* painter, int row, int col, const QRect& r, bool selected, const QColorGroup& cg);

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
    * reimplemented to support QWidget::WState_BlockUpdates
    */
  void drawContents(QPainter *p, int cx, int cy, int cw, int ch);

  /**
    * reimplemented to prevent normal mouse press behavior
    */
  void contentsMousePressEvent(QMouseEvent* ev) { ev->ignore(); }

  /**
    * reimplemented to prevent normal mouse move behavior
    */
  void contentsMouseMoveEvent(QMouseEvent* ev) { ev->ignore(); }

  /**
    * reimplemented to prevent normal mouse release behavior
    */
  void contentsMouseReleaseEvent(QMouseEvent* ev) { ev->ignore(); }

  /**
    * reimplemented to prevent normal mouse double click behavior
    */
  void contentsMouseDoubleClickEvent(QMouseEvent* ev) { ev->ignore(); }

  /**
    * reimplemented to prevent normal keyboard behavior
    */
  void keyPressEvent(QKeyEvent* ev) { ev->ignore(); }

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
};


} // namespace

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
