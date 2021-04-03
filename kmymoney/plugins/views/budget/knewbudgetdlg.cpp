/*
    SPDX-FileCopyrightText: 2000-2004 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knewbudgetdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewbudgetdlg.h"

class KNewBudgetDlgPrivate
{
    Q_DISABLE_COPY(KNewBudgetDlgPrivate)

public:
    KNewBudgetDlgPrivate() :
        ui(new Ui::KNewBudgetDlg)
    {
    }

    ~KNewBudgetDlgPrivate()
    {
        delete ui;
    }

    Ui::KNewBudgetDlg *ui;
    QString m_year;
    QString m_name;
};

// the combobox should look m_icNextYears into the future
static const int icFutureYears = 5;
static const int icPastYears = 2;

KNewBudgetDlg::KNewBudgetDlg(QWidget* parent) :
    QDialog(parent),
    d_ptr(new KNewBudgetDlgPrivate)
{
    Q_D(KNewBudgetDlg);
    d->ui->setupUi(this);
    QStringList slYear;
    auto dToday = QDate::currentDate();
    auto iYear = dToday.year();

    for (auto i = 0; i <= icFutureYears; ++i)
        d->ui->m_cbYear->addItem(QString::number(iYear++)); // krazy:exclude=postfixop

    iYear = dToday.year();
    for (auto i = 0; i <= icPastYears; ++i)
        d->ui->m_cbYear->addItem(QString::number(--iYear));

    connect(d->ui->buttonBox, &QDialogButtonBox::accepted, this, &KNewBudgetDlg::m_pbOk_clicked);
    connect(d->ui->buttonBox, &QDialogButtonBox::rejected, this, &KNewBudgetDlg::m_pbCancel_clicked);
}

KNewBudgetDlg::~KNewBudgetDlg()
{
    Q_D(KNewBudgetDlg);
    delete d;
}

void KNewBudgetDlg::m_pbCancel_clicked()
{
    reject();
}

void KNewBudgetDlg::m_pbOk_clicked()
{
    Q_D(KNewBudgetDlg);
    // force focus change to update all data
    d->ui->buttonBox->button(QDialogButtonBox::Ok)->setFocus();

    if (d->ui->m_leBudgetName->displayText().isEmpty()) {
        KMessageBox::information(this, i18n("Please specify a budget name"));
        d->ui->m_leBudgetName->setFocus();
        return;
    }

    d->m_year = d->ui->m_cbYear->currentText();
    d->m_name = d->ui->m_leBudgetName->displayText();

    accept();
}

QString KNewBudgetDlg::getYear() const
{
    Q_D(const KNewBudgetDlg);
    return d->m_year;
}

QString KNewBudgetDlg::getName() const
{
    Q_D(const KNewBudgetDlg);
    return d->m_name;
}
