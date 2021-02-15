/*
 * SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KFINDTRANSACTIONDLG_H
#define KFINDTRANSACTIONDLG_H

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
class KSortOptionDlg : public QDialog
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
class KFindTransactionDlg : public QDialog
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
