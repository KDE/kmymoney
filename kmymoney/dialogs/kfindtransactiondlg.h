/***************************************************************************
                          kfindtransactiondlg.h
                             -------------------
    copyright            : (C) 2003 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

  void slotUpdateSelections();

  virtual void slotAmountSelected();
  virtual void slotAmountRangeSelected();

  virtual void slotSelectAllPayees();
  virtual void slotDeselectAllPayees();

  virtual void slotSelectAllTags();
  virtual void slotDeselectAllTags();

  virtual void slotNrSelected();
  virtual void slotNrRangeSelected();

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
