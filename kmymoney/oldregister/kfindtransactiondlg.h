/*
 * Copyright 2002-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KFINDTRANSACTIONDLG_H
#define KFINDTRANSACTIONDLG_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QTreeWidgetItem;

namespace Ui { class KSortOptionDlg; }

/**
  * @author Thomas Baumgart
  */
class KMM_OLDREGISTER_EXPORT KSortOptionDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KSortOptionDlg)

public:
  explicit KSortOptionDlg(QWidget *parent = nullptr);
  ~KSortOptionDlg();

  void setSortOption(const QString& option, const QString& def);
  QString sortOption() const;
  void hideDefaultButton();

private:
  Ui::KSortOptionDlg *ui;
};

class KFindTransactionDlgPrivate;
class KMM_OLDREGISTER_EXPORT KFindTransactionDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KFindTransactionDlg)

public:
  /**
   @param withEquityAccounts set to false to hide equity accounts in account page
  */
  explicit KFindTransactionDlg(QWidget *parent = nullptr, bool withEquityAccounts = false);
  virtual ~KFindTransactionDlg();

  bool eventFilter(QObject *o, QEvent *e) override;

protected Q_SLOTS:
  virtual void slotReset();
  virtual void slotSearch();

  /**
    * This slot opens the detailed help page in khelpcenter. The
    * anchor for the information is taken from m_helpAnchor.
    */
  virtual void slotShowHelp();

  void slotRefreshView();

  /**
    * This slot selects the current selected transaction/split and emits
    * the signal @a transactionSelected(const QString& accountId, const QString& transactionId)
    */
  void slotSelectTransaction();

  void slotRightSize();

  void slotSortOptions();

Q_SIGNALS:
  void transactionSelected(const QString& accountId, const QString& transactionId);

  /**
    * This signal is sent out when a selection has been made. It is
    * used to control the state of the Search button.
    * The Search button is only active when a selection has been made
    * (i.e. notEmpty == true)
    */
  void selectionNotEmpty(bool);

protected:
  KFindTransactionDlgPrivate * const d_ptr;
  KFindTransactionDlg(KFindTransactionDlgPrivate &dd, QWidget *parent, bool withEquityAccounts);

  void resizeEvent(QResizeEvent*) override;
  void showEvent(QShowEvent* event) override;

private:
  Q_DECLARE_PRIVATE(KFindTransactionDlg)
};

#endif
