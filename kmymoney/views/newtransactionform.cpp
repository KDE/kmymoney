/*
 * Copyright 2015-2019  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2020       Robert Szczesiak <dev.rszczesiak@gmail.com>
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

#include "newtransactionform.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_newtransactionform.h"
#include "modelenums.h"
#include "journalmodel.h"
#include "mymoneyfile.h"
#include "tagsmodel.h"
#include "statusmodel.h"

using namespace eLedgerModel;

class NewTransactionForm::Private
{
public:
  Private()
  : ui(new Ui_NewTransactionForm)
  , row(-1)
  {
  }

  ~Private()
  {
    delete ui;
  }

  Ui_NewTransactionForm*  ui;
  int                     row;
};


NewTransactionForm::NewTransactionForm(QWidget* parent)
  : QFrame(parent)
  , d(new Private)
{
  const auto journalModel = MyMoneyFile::instance()->journalModel();
  d->ui->setupUi(this);
  connect(journalModel, &QAbstractItemModel::rowsInserted, this, &NewTransactionForm::rowsInserted);
  connect(journalModel, &QAbstractItemModel::rowsRemoved, this, &NewTransactionForm::rowsRemoved);
  connect(journalModel, &QAbstractItemModel::dataChanged, this, &NewTransactionForm::modelDataChanged);
}

NewTransactionForm::~NewTransactionForm()
{
  delete d;
}

void NewTransactionForm::rowsInserted(const QModelIndex& parent, int first, int last)
{
  Q_UNUSED(parent);
  if (first <= d->row) {
    d->row += last - first + 1;
  }
}

void NewTransactionForm::rowsRemoved(const QModelIndex& parent, int first, int last)
{
  Q_UNUSED(parent);
  if (first <= d->row) {
    d->row -= last - first + 1;
  }
}

void NewTransactionForm::showTransaction(const QModelIndex& idx)
{
  const auto index = MyMoneyFile::baseModel()->mapToBaseSource(idx);
  d->row = index.row();

  d->ui->dateEdit->setText(QLocale().toString(index.data(eMyMoney::Model::TransactionPostDateRole).toDate(),
                                              QLocale::ShortFormat));
  d->ui->payeeEdit->setText(index.data(eMyMoney::Model::SplitPayeeRole).toString());
  d->ui->memoEdit->clear();
  d->ui->memoEdit->insertPlainText(index.data(eMyMoney::Model::SplitMemoRole).toString());
  d->ui->memoEdit->moveCursor(QTextCursor::Start);
  d->ui->memoEdit->ensureCursorVisible();
  d->ui->accountEdit->setText(index.data(eMyMoney::Model::TransactionCounterAccountRole).toString());
  d->ui->accountEdit->home(false);

  d->ui->tagEdit->clear();
  QStringList splitTagList = index.data(eMyMoney::Model::SplitTagIdRole).toStringList();
  if (!splitTagList.isEmpty()) {
    std::transform(splitTagList.begin(), splitTagList.end(), splitTagList.begin(),
                   [] (const QString& tagId) { return MyMoneyFile::instance()->tagsModel()->itemById(tagId).name(); });
    d->ui->tagEdit->setText(splitTagList.join(", "));
    d->ui->tagEdit->home(false);
  }

  d->ui->numberEdit->setText(index.data(eMyMoney::Model::SplitNumberRole).toString());

  d->ui->amountEdit->setText(i18nc("@item:intext Amount, %1 transaction amount, %2 abbreviated Credit/Debit suffix: Cr. or Dr.",
                                   "%1 %2",
                                   index.data(eMyMoney::Model::SplitSharesFormattedRole).toString(),
                                   index.data(eMyMoney::Model::SplitSharesSuffixRole).toString()));

  const auto status = index.data(eMyMoney::Model::SplitReconcileFlagRole).toInt();
  const auto statusIdx = MyMoneyFile::instance()->statusModel()->index(status, 0);
  d->ui->statusEdit->setText(statusIdx.data(eMyMoney::Model::SplitReconcileStatusRole).toString());
}

void NewTransactionForm::modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  if ((topLeft.row() <= d->row) && (bottomRight.row() >= d->row)) {
    const auto idx = MyMoneyFile::instance()->journalModel()->index(d->row, 0);
    showTransaction(idx);
  }
}
