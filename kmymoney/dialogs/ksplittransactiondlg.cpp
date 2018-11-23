/***************************************************************************
                          ksplittransactiondlg.cpp  -  description
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

#include "ksplittransactiondlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QRadioButton>
#include <QCursor>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kapplication.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneyedit.h>
#include <kmymoneylineedit.h>
#include <mymoneyfile.h>
#include "kmymoneysplittable.h"

KSplitTransactionDlg::KSplitTransactionDlg(const MyMoneyTransaction& t,
    const MyMoneySplit& s,
    const MyMoneyAccount& acc,
    const bool amountValid,
    const bool deposit,
    const MyMoneyMoney& calculatedValue,
    const QMap<QString, MyMoneyMoney>& priceInfo,
    QWidget* parent) :
    KSplitTransactionDlgDecl(parent),
    m_account(acc),
    m_split(s),
    m_precision(2),
    m_amountValid(amountValid),
    m_isDeposit(deposit),
    m_calculatedValue(calculatedValue)
{
  setModal(true);

  setMainWidget(horizontalLayoutWidget);

  setButtons(KDialog::Ok | KDialog::Cancel | KDialog::User1 | KDialog::User2 | KDialog::User3);
  setButtonsOrientation(Qt::Vertical);

  //set custom buttons
  //clearAll button
  setButtonText(User1, i18n("Clear &All"));
  setButtonToolTip(User1, i18n("Clear all splits"));
  setButtonWhatsThis(User1, i18n("Use this to clear all splits of this transaction"));
  setButtonIcon(User1, KIcon("edit-clear"));

  //clearZero button
  setButtonText(User2, i18n("Clear &Zero"));
  setButtonToolTip(User2, i18n("Removes all splits that have a value of zero"));
  setButtonIcon(User2, KIcon("edit-clear"));

  //merge button
  setButtonText(User3, i18n("&Merge"));
  setButtonToolTip(User3, i18n("Merges splits with the same category to one split"));
  setButtonWhatsThis(User3, i18n("In case you have multiple split entries to the same category and you like to keep them as a single split, press this button. The amount for identical categories will be added and stored in a single split for that category."));

  // make finish the default
  button(KDialog::Cancel)->setDefault(true);

  // setup the focus
  button(KDialog::Cancel)->setFocusPolicy(Qt::NoFocus);
  button(KDialog::Ok)->setFocusPolicy(Qt::NoFocus);
  button(KDialog::User1)->setFocusPolicy(Qt::NoFocus);

  // connect signals with slots
  connect(transactionsTable, SIGNAL(transactionChanged(MyMoneyTransaction)),
          this, SLOT(slotSetTransaction(MyMoneyTransaction)));
  connect(transactionsTable, SIGNAL(createCategory(QString,QString&)), this, SLOT(slotCreateCategory(QString,QString&)));
  connect(transactionsTable, SIGNAL(createTag(QString,QString&)), this, SIGNAL(createTag(QString,QString&)));
  connect(transactionsTable, SIGNAL(objectCreation(bool)), this, SIGNAL(objectCreation(bool)));

  connect(transactionsTable, SIGNAL(returnPressed()), this, SLOT(accept()));
  connect(transactionsTable, SIGNAL(escapePressed()), this, SLOT(reject()));
  connect(transactionsTable, SIGNAL(editStarted()), this, SLOT(slotEditStarted()));
  connect(transactionsTable, SIGNAL(editFinished()), this, SLOT(slotUpdateButtons()));

  connect(button(KDialog::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
  connect(button(KDialog::Ok), SIGNAL(clicked()), this, SLOT(accept()));
  connect(button(KDialog::User1), SIGNAL(clicked()), this, SLOT(slotClearAllSplits()));
  connect(button(KDialog::User3), SIGNAL(clicked()), this, SLOT(slotMergeSplits()));
  connect(button(KDialog::User2), SIGNAL(clicked()), this, SLOT(slotClearUnusedSplits()));

  // setup the precision
  try {
    MyMoneySecurity currency = MyMoneyFile::instance()->currency(t.commodity());
    m_precision = MyMoneyMoney::denomToPrec(m_account.fraction(currency));
  } catch (const MyMoneyException &) {
  }

  slotSetTransaction(t);

  // pass on those vars
  transactionsTable->setup(priceInfo, m_precision);

  QSize size(width(), height());
  KConfigGroup grp = KGlobal::config()->group("SplitTransactionEditor");
  size = grp.readEntry("Geometry", size);
  size.setHeight(size.height() - 1);
  KDialog::resize(size.expandedTo(minimumSizeHint()));

  // Trick: it seems, that the initial sizing of the dialog does
  // not work correctly. At least, the columns do not get displayed
  // correct. Reason: the return value of transactionsTable->visibleWidth()
  // is incorrect. If the widget is visible, resizing works correctly.
  // So, we let the dialog show up and resize it then. It's not really
  // clean, but the only way I got the damned thing working.
  QTimer::singleShot(10, this, SLOT(initSize()));
}

KSplitTransactionDlg::~KSplitTransactionDlg()
{
  KConfigGroup grp =  KGlobal::config()->group("SplitTransactionEditor");
  grp.writeEntry("Geometry", size());
}

int KSplitTransactionDlg::exec()
{
  // for deposits, we invert the sign of all splits.
  // don't forget to revert when we're done ;-)
  if (m_isDeposit) {
    for (int i = 0; i < m_transaction.splits().count(); ++i) {
      MyMoneySplit split = m_transaction.splits()[i];
      split.setValue(-split.value());
      split.setShares(-split.shares());
      m_transaction.modifySplit(split);
    }
  }

  int rc;
  do {
    transactionsTable->setFocus();

    // initialize the display
    transactionsTable->setTransaction(m_transaction, m_split, m_account);
    updateSums();

    rc = KSplitTransactionDlgDecl::exec();

    if (rc == Accepted) {
      if (!diffAmount().isZero()) {
        KSplitCorrectionDlgDecl* corrDlg = new KSplitCorrectionDlgDecl(this);
        corrDlg->setMainWidget(corrDlg->findChild<QWidget*>("verticalLayout"));
        corrDlg->buttonGroup->setId(corrDlg->continueBtn, 0);
        corrDlg->buttonGroup->setId(corrDlg->changeBtn, 1);
        corrDlg->buttonGroup->setId(corrDlg->distributeBtn, 2);
        corrDlg->buttonGroup->setId(corrDlg->leaveBtn, 3);

        corrDlg->setModal(true);

        MyMoneySplit split = m_transaction.splits()[0];
        QString total = (-split.value()).formatMoney("", m_precision);
        QString sums = splitsValue().formatMoney("", m_precision);
        QString diff = diffAmount().formatMoney("", m_precision);

        // now modify the text items of the dialog to contain the correct values
        QString q = i18n("The total amount of this transaction is %1 while "
                         "the sum of the splits is %2. The remaining %3 are "
                         "unassigned.", total, sums, diff);
        corrDlg->explanation->setText(q);

        q = i18n("Change &total amount of transaction to %1.", sums);
        corrDlg->changeBtn->setText(q);

        q = i18n("&Distribute difference of %1 among all splits.", diff);
        corrDlg->distributeBtn->setText(q);
        // FIXME remove the following line once distribution among
        //       all splits is implemented
        corrDlg->distributeBtn->hide();


        // if we have only two splits left, we don't allow leaving sth. unassigned.
        if (m_transaction.splitCount() < 3) {
          q = i18n("&Leave total amount of transaction at %1.", total);
        } else {
          q = i18n("&Leave %1 unassigned.", diff);
        }
        corrDlg->leaveBtn->setText(q);

        if ((rc = corrDlg->exec()) == Accepted) {
          switch (corrDlg->buttonGroup->checkedId()) {
            case 0:       // continue to edit
              rc = Rejected;
              break;

            case 1:       // modify total
              split.setValue(-splitsValue());
              split.setShares(-splitsValue());
              m_transaction.modifySplit(split);
              break;

            case 2:       // distribute difference
              qDebug("distribution of difference not yet supported in KSplitTransactionDlg::slotFinishClicked()");
              break;

            case 3:       // leave unassigned
              break;
          }
        }
        delete corrDlg;
      }
    } else
      break;

  } while (rc != Accepted);

  // for deposits, we inverted the sign of all splits.
  // now we revert it back, so that things are left correct
  if (m_isDeposit) {
    for (int i = 0; i < m_transaction.splits().count(); ++i) {
      MyMoneySplit split = m_transaction.splits()[i];
      split.setValue(-split.value());
      split.setShares(-split.shares());
      m_transaction.modifySplit(split);
    }
  }

  return rc;
}

void KSplitTransactionDlg::initSize()
{
  KDialog::resize(width(), height() + 1);
}

void KSplitTransactionDlg::accept()
{
  transactionsTable->slotCancelEdit();
  KSplitTransactionDlgDecl::accept();
}

void KSplitTransactionDlg::reject()
{
  // cancel any edit activity in the split register
  transactionsTable->slotCancelEdit();
  KSplitTransactionDlgDecl::reject();
}

void KSplitTransactionDlg::slotClearAllSplits()
{
  int answer;
  answer = KMessageBox::warningContinueCancel(this,
           i18n("You are about to delete all splits of this transaction. "
                "Do you really want to continue?"),
           i18n("KMyMoney"),
           KGuiItem(i18n("Continue"))
                                             );

  if (answer == KMessageBox::Continue) {
    transactionsTable->slotCancelEdit();
    QList<MyMoneySplit> list = transactionsTable->getSplits(m_transaction);
    QList<MyMoneySplit>::ConstIterator it;

    // clear all but the one referencing the account
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
      m_transaction.removeSplit(*it);
    }

    transactionsTable->setTransaction(m_transaction, m_split, m_account);
    slotSetTransaction(m_transaction);
  }
}

void KSplitTransactionDlg::slotClearUnusedSplits()
{
  QList<MyMoneySplit> list = transactionsTable->getSplits(m_transaction);
  QList<MyMoneySplit>::ConstIterator it;

  try {
    // remove all splits that don't have a value assigned
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
      if ((*it).shares().isZero()) {
        m_transaction.removeSplit(*it);
      }
    }

    transactionsTable->setTransaction(m_transaction, m_split, m_account);
    slotSetTransaction(m_transaction);
  } catch (const MyMoneyException &) {
  }
}

void KSplitTransactionDlg::slotMergeSplits()
{
  QList<MyMoneySplit> list = transactionsTable->getSplits(m_transaction);
  QList<MyMoneySplit>::ConstIterator it;

  try {
    // collect all splits, merge them if needed and remove from transaction
    QList<MyMoneySplit> splits;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
      QList<MyMoneySplit>::iterator it_s;
      for (it_s = splits.begin(); it_s != splits.end(); ++it_s) {
        if ((*it_s).accountId() == (*it).accountId()
            && (*it_s).memo().isEmpty() && (*it).memo().isEmpty())
          break;
      }
      if (it_s != splits.end()) {
        (*it_s).setShares((*it).shares() + (*it_s).shares());
        (*it_s).setValue((*it).value() + (*it_s).value());
      } else {
        splits << *it;
      }
      m_transaction.removeSplit(*it);
    }

    // now add them back to the transaction
    QList<MyMoneySplit>::iterator it_s;
    for (it_s = splits.begin(); it_s != splits.end(); ++it_s) {
      (*it_s).clearId();
      m_transaction.addSplit(*it_s);
    }

    transactionsTable->setTransaction(m_transaction, m_split, m_account);
    slotSetTransaction(m_transaction);
  } catch (const MyMoneyException &) {
  }
}

void KSplitTransactionDlg::slotSetTransaction(const MyMoneyTransaction& t)
{
  m_transaction = t;
  slotUpdateButtons();
  updateSums();
}

void KSplitTransactionDlg::slotUpdateButtons()
{
  QList<MyMoneySplit> list = transactionsTable->getSplits(m_transaction);
  // check if we can merge splits or not, have zero splits or not
  QMap<QString, int> splits;
  bool haveZeroSplit = false;
  for (QList<MyMoneySplit>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it) {
    splits[(*it).accountId()]++;
    if (((*it).id() != m_split.id()) && ((*it).shares().isZero()))
      haveZeroSplit = true;
  }
  QMap<QString, int>::const_iterator it_s;
  for (it_s = splits.constBegin(); it_s != splits.constEnd(); ++it_s) {
    if ((*it_s) > 1)
      break;
  }
  enableButton(KDialog::User3, it_s != splits.constEnd());
  enableButton(KDialog::User2, haveZeroSplit);
}

void KSplitTransactionDlg::slotEditStarted()
{
  enableButton(KDialog::User3, false);
  enableButton(KDialog::User2, false);
}

void KSplitTransactionDlg::updateSums()
{
  MyMoneyMoney splits(splitsValue());

  if (m_amountValid == false) {
    m_split.setValue(-splits);
    m_transaction.modifySplit(m_split);
  }

  splitSum->setText("<b>" + splits.formatMoney("", m_precision) + ' ');
  splitUnassigned->setText("<b>" + diffAmount().formatMoney("", m_precision) + ' ');
  transactionAmount->setText("<b>" + (-m_split.value()).formatMoney("", m_precision) + ' ');
}

MyMoneyMoney KSplitTransactionDlg::splitsValue()
{
  MyMoneyMoney splitsValue(m_calculatedValue);
  QList<MyMoneySplit> list = transactionsTable->getSplits(m_transaction);
  QList<MyMoneySplit>::ConstIterator it;

  // calculate the current sum of all split parts
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    if ((*it).value() != MyMoneyMoney::autoCalc)
      splitsValue += (*it).value();
  }

  return splitsValue;
}

MyMoneyMoney KSplitTransactionDlg::diffAmount()
{
  MyMoneyMoney diff;

  // if there is an amount specified in the transaction, we need to calculate the
  // difference, otherwise we display the difference as 0 and display the same sum.
  if (m_amountValid) {
    MyMoneySplit split = m_transaction.splits()[0];

    diff = -(splitsValue() + split.value());
  }
  return diff;
}

void KSplitTransactionDlg::slotCreateCategory(const QString& name, QString& id)
{
  MyMoneyAccount acc, parent;
  acc.setName(name);

  if (m_isDeposit)
    parent = MyMoneyFile::instance()->income();
  else
    parent = MyMoneyFile::instance()->expense();

  // TODO extract possible first part of a hierarchy and check if it is one
  // of our top categories. If so, remove it and select the parent
  // according to this information.

  emit createCategory(acc, parent);

  // return id
  id = acc.id();
}

