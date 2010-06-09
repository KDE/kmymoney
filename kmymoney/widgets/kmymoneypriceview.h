/***************************************************************************
                          kmymoneypriceview.h  -  description
                             -------------------
    begin                : Wed Mar 24 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

#ifndef KMYMONEYPRICEVIEW_H
#define KMYMONEYPRICEVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
#include <QResizeEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneylistviewitem.h>
//#include <mymoneymoney.h>
#include <mymoneyprice.h>

/**
  * @author Thomas Baumgart
  */

class KMyMoneyPriceItem : public KMyMoneyListViewItem
{
public:
  KMyMoneyPriceItem(K3ListView *, const MyMoneyPrice& pr);
  ~KMyMoneyPriceItem() {}

  int compare(Q3ListViewItem *p, int col, bool ascending) const;

  const MyMoneyPrice& price(void) const {
    return m_pr;
  };

private:
  MyMoneyPrice  m_pr;
};


class KMyMoneyPriceView : public K3ListView
{
  Q_OBJECT
public:
  KMyMoneyPriceView(QWidget *parent = 0);
  ~KMyMoneyPriceView();

protected:
  /// the resize event
  virtual void resizeEvent(QResizeEvent*);

protected slots:
  void slotListClicked(Q3ListViewItem* item, const QPoint&, int);

private slots:
  void slotTimerDone(void);

signals:
  void newPrice(void);
  void deletePrice(void);
  void editPrice(void);
  void onlinePriceUpdate(void);
  void openContextMenu(const MyMoneyPrice& price);

private:
  bool            m_showAll;
};

#endif
