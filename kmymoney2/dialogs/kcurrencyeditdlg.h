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

#include <qwidget.h>
//Added by qt3to4:
#include <QResizeEvent>

// ----------------------------------------------------------------------------
// KDE Includes

class KMenu;

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcurrencyeditdlgdecl.h"
#include "../mymoney/mymoneysecurity.h"

/**
  * @author Thomas Baumgart
  */

class KCurrencyEditDlgDecl : public QDialog, public Ui::KCurrencyEditDlgDecl
{
public:
  KCurrencyEditDlgDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};

class KCurrencyEditDlg : public KCurrencyEditDlgDecl
{
  Q_OBJECT
public:
  KCurrencyEditDlg(QWidget *parent=0);
  ~KCurrencyEditDlg();

public slots:
  void slotSelectCurrency(const QString& id);

protected:
  /// the resize event
  virtual void resizeEvent(QResizeEvent*);
  void updateCurrency(void);

protected slots:
  void slotSelectCurrency(Q3ListViewItem *);

  void slotClose(void);
  void slotStartRename(void);
  void slotListClicked(Q3ListViewItem* item, const QPoint&, int);
  void slotRenameCurrency(Q3ListViewItem* item, int col, const QString& txt);
  void slotLoadCurrencies(void);

private slots:
  void timerDone(void);
  void slotSelectBaseCurrency(void);

signals:
  void selectObject(const MyMoneySecurity& currency);
  void openContextMenu(const MyMoneySecurity& currency);
  void renameCurrency(Q3ListViewItem* item, int, const QString& txt);
  void selectBaseCurrency(const MyMoneySecurity& currency);

private:
  MyMoneySecurity      m_currency;
};

#endif
