/*
 * SPDX-FileCopyrightText: 2004-2020 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2009-2010 Alvaro Soliverez <asoliverez@gmail.com>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KCURRENCYEDITDLG_H
#define KCURRENCYEDITDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// Project Includes

class QTreeWidgetItem;
class KAvailableCurrencyDlg;
class KCurrencyEditorDlg;
class KTreeWidgetSearchLineWidget;

class MyMoneySecurity;
/**
  * @author Thomas Baumgart
  */
class KCurrencyEditDlgPrivate;
class KCurrencyEditDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KCurrencyEditDlg)

public:
  explicit KCurrencyEditDlg(QWidget *parent = nullptr);
  ~KCurrencyEditDlg();

public Q_SLOTS:
  void slotSelectCurrency(const QString& id);

protected Q_SLOTS:
  void slotSelectCurrency(QTreeWidgetItem *citem, QTreeWidgetItem *pitem);
  void slotSelectCurrency(QTreeWidgetItem *item);
  void slotItemSelectionChanged();
  void slotShowCurrencyMenu(const QPoint& p);
  void slotLoadCurrencies();
  void slotUpdateCurrency(QTreeWidgetItem* citem, int column);
  void slotUpdateCurrency(QTreeWidgetItem *citem, QTreeWidgetItem *pitem);

private:
  KCurrencyEditDlgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KCurrencyEditDlg)

private Q_SLOTS:
  void timerDone();
  void slotSelectBaseCurrency();
  void slotAddCurrency();
  void slotRemoveCurrency();
  void slotRemoveUnusedCurrency();
  void slotEditCurrency();

  void slotNewCurrency();
  void slotRenameCurrency();
  void slotDeleteCurrency();
  void slotSetBaseCurrency();
};

#endif
