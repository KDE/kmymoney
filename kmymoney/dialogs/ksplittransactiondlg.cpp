/*
    SPDX-FileCopyrightText: 2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ksplittransactiondlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QRadioButton>
#include <QList>
#include <QIcon>
#include <QDialogButtonBox>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfig>
#include <KMessageBox>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksplittransactiondlg.h"
#include "ui_ksplitcorrectiondlg.h"

#include "kmymoneyutils.h"
#include "mymoneyfile.h"
#include "kmymoneysplittable.h"
#include "mymoneymoney.h"
#include "mymoneyexception.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "icons/icons.h"

using namespace Icons;

KSplitCorrectionDlg::KSplitCorrectionDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::KSplitCorrectionDlg)
{
  ui->setupUi(this);
}

KSplitCorrectionDlg::~KSplitCorrectionDlg()
{
  delete ui;
}

class KSplitTransactionDlgPrivate
{
  Q_DISABLE_COPY(KSplitTransactionDlgPrivate)
  Q_DECLARE_PUBLIC(KSplitTransactionDlg)

public:
  explicit KSplitTransactionDlgPrivate(KSplitTransactionDlg *qq) :
    q_ptr(qq),
    ui(new Ui::KSplitTransactionDlg),
    m_buttonBox(nullptr),
    m_precision(2),
    m_amountValid(false),
    m_isDeposit(false)
  {
  }

  ~KSplitTransactionDlgPrivate()
  {
    delete ui;
  }

  void init(const MyMoneyTransaction& t, const QMap<QString, MyMoneyMoney>& priceInfo)
  {
    Q_Q(KSplitTransactionDlg);
    ui->setupUi(q);
    q->setModal(true);

    auto okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    auto user1Button = new QPushButton;
    ui->buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    auto user2Button = new QPushButton;
    ui->buttonBox->addButton(user2Button, QDialogButtonBox::ActionRole);
    auto user3Button = new QPushButton;
    ui->buttonBox->addButton(user3Button, QDialogButtonBox::ActionRole);

    //set custom buttons
    //clearAll button
    user1Button->setText(i18n("Clear &All"));
    user1Button->setToolTip(i18n("Clear all splits"));
    user1Button->setWhatsThis(i18n("Use this to clear all splits of this transaction"));
    user1Button->setIcon(Icons::get(Icon::EditClear));

    //clearZero button
    user2Button->setText(i18n("Clear &Zero"));
    user2Button->setToolTip(i18n("Removes all splits that have a value of zero"));
    user2Button->setIcon(Icons::get(Icon::EditClear));

    //merge button
    user3Button->setText(i18n("&Merge"));
    user3Button->setToolTip(i18n("Merges splits with the same category to one split"));
    user3Button->setWhatsThis(i18n("In case you have multiple split entries to the same category and you like to keep them as a single split"));

    // make finish the default
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);

    // setup the focus
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setFocusPolicy(Qt::NoFocus);
    okButton->setFocusPolicy(Qt::NoFocus);
    user1Button->setFocusPolicy(Qt::NoFocus);

    // q->connect signals with slots
    q->connect(ui->transactionsTable, &KMyMoneySplitTable::transactionChanged,
            q, &KSplitTransactionDlg::slotSetTransaction);
    q->connect(ui->transactionsTable, &KMyMoneySplitTable::createCategory, q, &KSplitTransactionDlg::slotCreateCategory);
    q->connect(ui->transactionsTable, &KMyMoneySplitTable::createTag, q, &KSplitTransactionDlg::slotCreateTag);
    q->connect(ui->transactionsTable, &KMyMoneySplitTable::objectCreation, q, &KSplitTransactionDlg::objectCreation);

    q->connect(ui->transactionsTable, &KMyMoneySplitTable::returnPressed, q, &KSplitTransactionDlg::accept);
    q->connect(ui->transactionsTable, &KMyMoneySplitTable::escapePressed, q, &KSplitTransactionDlg::reject);
    q->connect(ui->transactionsTable, &KMyMoneySplitTable::editStarted, q, &KSplitTransactionDlg::slotEditStarted);
    q->connect(ui->transactionsTable, &KMyMoneySplitTable::editFinished, q, &KSplitTransactionDlg::slotUpdateButtons);

    q->connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QAbstractButton::clicked, q, &KSplitTransactionDlg::reject);
    q->connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, q, &KSplitTransactionDlg::accept);
    q->connect(user1Button, &QAbstractButton::clicked, q, &KSplitTransactionDlg::slotClearAllSplits);
    q->connect(user3Button, &QAbstractButton::clicked, q, &KSplitTransactionDlg::slotMergeSplits);
    q->connect(user2Button, &QAbstractButton::clicked, q, &KSplitTransactionDlg::slotClearUnusedSplits);

    // setup the precision
    try {
      auto currency = MyMoneyFile::instance()->currency(t.commodity());
      m_precision = MyMoneyMoney::denomToPrec(m_account.fraction(currency));
    } catch (const MyMoneyException &) {
    }

    q->slotSetTransaction(t);

    // pass on those vars
    ui->transactionsTable->setup(priceInfo, m_precision);

    QSize size(q->width(), q->height());
    KConfigGroup grp = KSharedConfig::openConfig()->group("SplitTransactionEditor");
    size = grp.readEntry("Geometry", size);
    size.setHeight(size.height() - 1);
    q->resize(size.expandedTo(q->minimumSizeHint()));

    // Trick: it seems, that the initial sizing of the dialog does
    // not work correctly. At least, the columns do not get displayed
    // correct. Reason: the return value of ui->transactionsTable->visibleWidth()
    // is incorrect. If the widget is visible, resizing works correctly.
    // So, we let the dialog show up and resize it then. It's not really
    // clean, but the only way I got the damned thing working.
    QTimer::singleShot(10, q, SLOT(initSize()));

  }

  /**
    * This method updates the display of the sums below the register
    */
  void updateSums()
  {
    Q_Q(KSplitTransactionDlg);
    MyMoneyMoney splits(q->splitsValue());

    if (m_amountValid == false) {
      m_split.setValue(-splits);
      m_transaction.modifySplit(m_split);
    }

    ui->splitSum->setText("<b>" + splits.formatMoney(QString(), m_precision) + ' ');
    ui->splitUnassigned->setText("<b>" + q->diffAmount().formatMoney(QString(), m_precision) + ' ');
    ui->transactionAmount->setText("<b>" + (-m_split.value()).formatMoney(QString(), m_precision) + ' ');
  }

  KSplitTransactionDlg      *q_ptr;
  Ui::KSplitTransactionDlg  *ui;
  QDialogButtonBox          *m_buttonBox;
  /**
    * This member keeps a copy of the current selected transaction
    */
  MyMoneyTransaction     m_transaction;

  /**
    * This member keeps a copy of the currently selected account
    */
  MyMoneyAccount         m_account;

  /**
    * This member keeps a copy of the currently selected split
    */
  MyMoneySplit           m_split;

  /**
    * This member keeps the precision for the values
    */
  int                    m_precision;

  /**
    * flag that shows that the amount specified in the constructor
    * should be used as fix value (true) or if it can be changed (false)
    */
  bool                   m_amountValid;

  /**
    * This member keeps track if the current transaction is of type
    * deposit (true) or withdrawal (false).
    */
  bool                   m_isDeposit;

  /**
    * This member keeps the amount that will be assigned to all the
    * splits that are marked 'will be calculated'.
    */
  MyMoneyMoney           m_calculatedValue;
};

KSplitTransactionDlg::KSplitTransactionDlg(const MyMoneyTransaction& t,
    const MyMoneySplit& s,
    const MyMoneyAccount& acc,
    const bool amountValid,
    const bool deposit,
    const MyMoneyMoney& calculatedValue,
    const QMap<QString, MyMoneyMoney>& priceInfo,
    QWidget* parent) :
  QDialog(parent),
  d_ptr(new KSplitTransactionDlgPrivate(this))
{
  Q_D(KSplitTransactionDlg);
  d->ui->buttonBox = nullptr;
  d->m_account = acc;
  d->m_split = s;
  d->m_precision = 2;
  d->m_amountValid = amountValid;
  d->m_isDeposit = deposit;
  d->m_calculatedValue = calculatedValue;
  d->init(t, priceInfo);
}

KSplitTransactionDlg::~KSplitTransactionDlg()
{
  Q_D(KSplitTransactionDlg);
  auto grp =  KSharedConfig::openConfig()->group("SplitTransactionEditor");
  grp.writeEntry("Geometry", size());
  delete d;
}

int KSplitTransactionDlg::exec()
{
  Q_D(KSplitTransactionDlg);
  // for deposits, we invert the sign of all splits.
  // don't forget to revert when we're done ;-)
  if (d->m_isDeposit) {
    for (auto i = 0; i < d->m_transaction.splits().count(); ++i) {
      MyMoneySplit split = d->m_transaction.splits()[i];
      split.setValue(-split.value());
      split.setShares(-split.shares());
      d->m_transaction.modifySplit(split);
    }
  }

  int rc;
  do {
    d->ui->transactionsTable->setFocus();

    // initialize the display
    d->ui->transactionsTable->setTransaction(d->m_transaction, d->m_split, d->m_account);
    d->updateSums();

    rc = QDialog::exec();

    if (rc == Accepted) {
      if (!diffAmount().isZero()) {
        QPointer<KSplitCorrectionDlg> corrDlg = new KSplitCorrectionDlg(this);
        connect(corrDlg->ui->buttonBox, &QDialogButtonBox::accepted, corrDlg.data(), &QDialog::accept);
        connect(corrDlg->ui->buttonBox, &QDialogButtonBox::rejected, corrDlg.data(), &QDialog::reject);
        corrDlg->ui->buttonGroup->setId(corrDlg->ui->continueBtn, 0);
        corrDlg->ui->buttonGroup->setId(corrDlg->ui->changeBtn, 1);
        corrDlg->ui->buttonGroup->setId(corrDlg->ui->distributeBtn, 2);
        corrDlg->ui->buttonGroup->setId(corrDlg->ui->leaveBtn, 3);

        MyMoneySplit split = d->m_transaction.splits()[0];
        QString total = (-split.value()).formatMoney(QString(), d->m_precision);
        QString sums = splitsValue().formatMoney(QString(), d->m_precision);
        QString diff = diffAmount().formatMoney(QString(), d->m_precision);

        // now modify the text items of the dialog to contain the correct values
        QString q = i18n("The total amount of this transaction is %1 while "
                         "the sum of the splits is %2. The remaining %3 are "
                         "unassigned.", total, sums, diff);
        corrDlg->ui->explanation->setText(q);

        q = i18n("Change &total amount of transaction to %1.", sums);
        corrDlg->ui->changeBtn->setText(q);

        q = i18n("&Distribute difference of %1 among all splits.", diff);
        corrDlg->ui->distributeBtn->setText(q);
        // FIXME remove the following line once distribution among
        //       all splits is implemented
        corrDlg->ui->distributeBtn->hide();


        // if we have only two splits left, we don't allow leaving sth. unassigned.
        if (d->m_transaction.splitCount() < 3) {
          q = i18n("&Leave total amount of transaction at %1.", total);
        } else {
          q = i18n("&Leave %1 unassigned.", diff);
        }
        corrDlg->ui->leaveBtn->setText(q);

        if ((rc = corrDlg->exec()) == Accepted) {
          switch (corrDlg->ui->buttonGroup->checkedId()) {
            case 0:       // continue to edit
              rc = Rejected;
              break;

            case 1:       // modify total
              split.setValue(-splitsValue());
              split.setShares(-splitsValue());
              d->m_transaction.modifySplit(split);
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
  if (d->m_isDeposit) {
    for (auto i = 0; i < d->m_transaction.splits().count(); ++i) {
      auto split = d->m_transaction.splits()[i];
      split.setValue(-split.value());
      split.setShares(-split.shares());
      d->m_transaction.modifySplit(split);
    }
  }

  return rc;
}

void KSplitTransactionDlg::initSize()
{
  QDialog::resize(width(), height() + 1);
}

void KSplitTransactionDlg::accept()
{
  Q_D(KSplitTransactionDlg);
  d->ui->transactionsTable->slotCancelEdit();
  QDialog::accept();
}

void KSplitTransactionDlg::reject()
{
  Q_D(KSplitTransactionDlg);
  // cancel any edit activity in the split register
  d->ui->transactionsTable->slotCancelEdit();
  QDialog::reject();
}

void KSplitTransactionDlg::slotClearAllSplits()
{
  Q_D(KSplitTransactionDlg);
  int answer;
  answer = KMessageBox::warningContinueCancel(this,
           i18n("You are about to delete all splits of this transaction. "
                "Do you really want to continue?"),
           i18n("KMyMoney"));

  if (answer == KMessageBox::Continue) {
    d->ui->transactionsTable->slotCancelEdit();
    QList<MyMoneySplit> list = d->ui->transactionsTable->getSplits(d->m_transaction);
    QList<MyMoneySplit>::ConstIterator it;

    // clear all but the one referencing the account
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
      d->m_transaction.removeSplit(*it);
    }

    d->ui->transactionsTable->setTransaction(d->m_transaction, d->m_split, d->m_account);
    slotSetTransaction(d->m_transaction);
  }
}

void KSplitTransactionDlg::slotClearUnusedSplits()
{
  Q_D(KSplitTransactionDlg);
  QList<MyMoneySplit> list = d->ui->transactionsTable->getSplits(d->m_transaction);
  QList<MyMoneySplit>::ConstIterator it;

  try {
    // remove all splits that don't have a value assigned
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
      if ((*it).shares().isZero()) {
        d->m_transaction.removeSplit(*it);
      }
    }

    d->ui->transactionsTable->setTransaction(d->m_transaction, d->m_split, d->m_account);
    slotSetTransaction(d->m_transaction);
  } catch (const MyMoneyException &) {
  }
}

void KSplitTransactionDlg::slotMergeSplits()
{
  Q_D(KSplitTransactionDlg);

  try {
    // collect all splits, merge them if needed and remove from transaction
    QList<MyMoneySplit> splits;
    foreach (const auto lsplit, d->ui->transactionsTable->getSplits(d->m_transaction)) {
      auto found = false;
      for (auto& split : splits) {
        if (split.accountId() == lsplit.accountId()
            && split.memo().isEmpty() && lsplit.memo().isEmpty()) {
          split.setShares(lsplit.shares() + split.shares());
          split.setValue(lsplit.value() + split.value());
          found = true;
          break;
        }
      }
      if (!found)
        splits << lsplit;

      d->m_transaction.removeSplit(lsplit);
    }

    // now add them back to the transaction
    for (auto& split : splits) {
      split.clearId();
      d->m_transaction.addSplit(split);
    }

    d->ui->transactionsTable->setTransaction(d->m_transaction, d->m_split, d->m_account);
    slotSetTransaction(d->m_transaction);
  } catch (const MyMoneyException &) {
  }
}

void KSplitTransactionDlg::slotSetTransaction(const MyMoneyTransaction& t)
{
  Q_D(KSplitTransactionDlg);
  d->m_transaction = t;
  slotUpdateButtons();
  d->updateSums();
}

void KSplitTransactionDlg::slotUpdateButtons()
{
  Q_D(KSplitTransactionDlg);
  QList<MyMoneySplit> list = d->ui->transactionsTable->getSplits(d->m_transaction);
  // check if we can merge splits or not, have zero splits or not
  QMap<QString, int> splits;
  bool haveZeroSplit = false;
  for (QList<MyMoneySplit>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it) {
    splits[(*it).accountId()]++;
    if (((*it).id() != d->m_split.id()) && ((*it).shares().isZero()))
      haveZeroSplit = true;
  }
  QMap<QString, int>::const_iterator it_s;
  for (it_s = splits.constBegin(); it_s != splits.constEnd(); ++it_s) {
    if ((*it_s) > 1)
      break;
  }
  d->ui->buttonBox->buttons().at(4)->setEnabled(it_s != splits.constEnd());
  d->ui->buttonBox->buttons().at(3)->setEnabled(haveZeroSplit);
}

void KSplitTransactionDlg::slotEditStarted()
{
  Q_D(KSplitTransactionDlg);
  d->ui->buttonBox->buttons().at(4)->setEnabled(false);
  d->ui->buttonBox->buttons().at(3)->setEnabled(false);
}

MyMoneyMoney KSplitTransactionDlg::splitsValue()
{
  Q_D(KSplitTransactionDlg);
  MyMoneyMoney splitsValue(d->m_calculatedValue);
  QList<MyMoneySplit> list = d->ui->transactionsTable->getSplits(d->m_transaction);
  QList<MyMoneySplit>::ConstIterator it;

  // calculate the current sum of all split parts
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    if ((*it).value() != MyMoneyMoney::autoCalc)
      splitsValue += (*it).value();
  }

  return splitsValue;
}

MyMoneyTransaction KSplitTransactionDlg::transaction() const
{
  Q_D(const KSplitTransactionDlg);
  return d->m_transaction;
}

MyMoneyMoney KSplitTransactionDlg::diffAmount()
{
  Q_D(KSplitTransactionDlg);
  MyMoneyMoney diff;

  // if there is an amount specified in the transaction, we need to calculate the
  // difference, otherwise we display the difference as 0 and display the same sum.
  if (d->m_amountValid) {
    MyMoneySplit split = d->m_transaction.splits()[0];

    diff = -(splitsValue() + split.value());
  }
  return diff;
}

void KSplitTransactionDlg::slotCreateCategory(const QString& name, QString& id)
{
  Q_D(KSplitTransactionDlg);
  MyMoneyAccount acc, parent;
  acc.setName(name);

  if (d->m_isDeposit)
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

void KSplitTransactionDlg::slotCreateTag(const QString& txt, QString& id)
{
  KMyMoneyUtils::newTag(txt, id);
  emit createTag(txt, id);
}
