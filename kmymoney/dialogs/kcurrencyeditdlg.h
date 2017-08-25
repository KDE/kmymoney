/***************************************************************************
                          kcurrencyeditdlg.h  -  description
                             -------------------
    begin                : Wed Mar 24 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Alvaro Soliverez <asoliverez@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KCURRENCYEDITDLG_H
#define KCURRENCYEDITDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KDialog>
#include <ktreewidgetsearchline.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcurrencyeditdlgdecl.h"
#include "mymoneysecurity.h"

class QTreeWidgetItem;

class KCurrencyEditDlgDecl : public KDialog, public Ui::KCurrencyEditDlgDecl
{
public:
  KCurrencyEditDlgDecl(QWidget *parent) : KDialog(parent) {
    setupUi(this);
  }
};

/**
  * @author Thomas Baumgart
  */
class KCurrencyEditDlg : public KCurrencyEditDlgDecl
{
  Q_OBJECT
public:
  KCurrencyEditDlg(QWidget *parent = 0);
  ~KCurrencyEditDlg();

public slots:
  void slotSelectCurrency(const QString& id);

protected:


protected slots:
  void slotSelectCurrency(QTreeWidgetItem *);
  void slotStartRename();
  void slotOpenContextMenu(const QPoint& p);
  void slotLoadCurrencies();
  void slotUpdateCurrency(QTreeWidgetItem *item);
  void slotDeleteCurrency();

private slots:
  void timerDone();
  void slotSelectBaseCurrency();

signals:
  void selectObject(const MyMoneySecurity& currency);
  void openContextMenu(const MyMoneySecurity& currency);
  void updateCurrency(const QString &currencyId, const QString& currencyName, const QString& currencyTradingSymbol);
  void selectBaseCurrency(const MyMoneySecurity& currency);
  void deleteCurrency(const MyMoneySecurity& currency);
  void newCurrency();

private:
  MyMoneySecurity               m_currency;
  /**
    * Search widget for the list
    */
  KTreeWidgetSearchLineWidget*  m_searchWidget;
};

#endif
