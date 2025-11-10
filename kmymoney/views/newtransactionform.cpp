/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newtransactionform.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QRegularExpression>
#include <mymoneypayee.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "mymoneyfile.h"
#include "tagsmodel.h"
#include "statusmodel.h"

#include "ui_newtransactionform.h"

#include <klocalizedstring.h>
#include <mymoneyexception.h>

class NewTransactionForm::Private
{
    Q_DISABLE_COPY_MOVE(Private)

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
    void updateMemoLink();

    Ui_NewTransactionForm*  ui;
    int                     row;
};


NewTransactionForm::NewTransactionForm(QWidget* parent)
    : QFrame(parent)
    , d(new Private)
{
    const auto journalModel = MyMoneyFile::instance()->journalModel();
    d->ui->setupUi(this);

#ifndef ENABLE_COSTCENTER
    d->ui->costCenterLabel->hide();
    d->ui->costCenterEdit->hide();
#endif

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

void NewTransactionForm::Private::updateMemoLink()
{
    try {
        const MyMoneyPayee& payeeObj = MyMoneyFile::instance()->payeeByName(ui->payeeEdit->text());
        QUrl url = payeeObj.payeeLink(ui->memoEdit->toPlainText());
        if (url.isEmpty()) {
            ui->linkLabel->setText("");
            return;
        }
        ui->linkLabel->setTextFormat(Qt::RichText);
        ui->linkLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        ui->linkLabel->setOpenExternalLinks(true);
        ui->linkLabel->setText(QString("<a href=\"%1\">%2</a>").arg(url.toString(), i18n("Link")));
        ui->linkLabel->setToolTip(url.toString());
        qDebug() << url;
    } catch (MyMoneyException&) {
        ui->linkLabel->setText("");
    }
}

void NewTransactionForm::showTransaction(const QModelIndex& idx)
{
    const auto index = MyMoneyFile::baseModel()->mapToBaseSource(idx);
    d->row = index.row();

    // make sure to have a four digit year display
    auto format(d->ui->dateEdit->displayFormat());
    static const QRegularExpression twoYearDigits(QLatin1String("^([^y]*)yy([^y]*)$"));
    format.replace(twoYearDigits, QLatin1String("\\1yyyy\\2"));
    d->ui->dateEdit->setDisplayFormat(format);

    d->ui->dateEdit->setDate(index.data(eMyMoney::Model::TransactionPostDateRole).toDate());
    d->ui->payeeEdit->setText(index.data(eMyMoney::Model::SplitPayeeRole).toString());
    d->ui->memoEdit->clear();
    d->ui->memoEdit->insertPlainText(index.data(eMyMoney::Model::SplitMemoRole).toString());
    d->ui->memoEdit->moveCursor(QTextCursor::Start);
    d->ui->memoEdit->ensureCursorVisible();
    d->ui->accountEdit->setText(index.data(eMyMoney::Model::TransactionCounterAccountRole).toString());
    d->ui->accountEdit->home(false);
    d->updateMemoLink();

    d->ui->numberEdit->setText(index.data(eMyMoney::Model::SplitNumberRole).toString());

    d->ui->amountEdit->setText(i18nc("@item:intext Amount, %1 transaction amount, %2 payment direction enclosed in parenthesis",
                                     "%1 %2",
                                     index.data(eMyMoney::Model::SplitSharesFormattedRole).toString(),
                                     idx.data(eMyMoney::Model::SplitSharesSuffixRole).toString()));

    const auto status = index.data(eMyMoney::Model::SplitReconcileFlagRole).toInt();
    const auto statusIdx = MyMoneyFile::instance()->statusModel()->index(status, 0);
    d->ui->statusEdit->setText(statusIdx.data(eMyMoney::Model::SplitReconcileStatusRole).toString());

    d->ui->tagEdit->clear();

    // fill tag list but only if two splits in transaction
    const auto list = index.model()->match(index.model()->index(0, 0),
                                           eMyMoney::Model::JournalTransactionIdRole,
                                           index.data(eMyMoney::Model::JournalTransactionIdRole),
                                           -1, // all splits
                                           Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));

    if (list.count() == 2) {
        for (const auto& splitIdx : list) {
            if (splitIdx.row() != d->row) {
                QStringList splitTagList = splitIdx.data(eMyMoney::Model::SplitTagIdRole).toStringList();
                if (!splitTagList.isEmpty()) {
                    std::transform(splitTagList.begin(), splitTagList.end(), splitTagList.begin(), [](const QString& tagId) {
                        return MyMoneyFile::instance()->tagsModel()->itemById(tagId).name();
                    });
                    d->ui->tagEdit->setText(splitTagList.join(", "));
                    d->ui->tagEdit->home(false);
                }
            }
        }
    }
}

void NewTransactionForm::modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    if ((topLeft.row() <= d->row) && (bottomRight.row() >= d->row)) {
        const auto idx = MyMoneyFile::instance()->journalModel()->index(d->row, 0);
        showTransaction(idx);
    }
}
