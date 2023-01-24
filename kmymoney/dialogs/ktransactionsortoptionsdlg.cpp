/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ktransactionsortoptionsdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ktransactionsortoptionsdlg.h"

namespace Ui {
class KTransactionSortOptionsDlg;
}

class KTransactionSortOptionsDlgPrivate
{
public:
    KTransactionSortOptionsDlgPrivate(KTransactionSortOptionsDlg* qq)
        : ui(new Ui::KTransactionSortOptionsDlg)
    {
        ui->setupUi(qq);
    }

    Ui::KTransactionSortOptionsDlg* ui;
};

KTransactionSortOptionsDlg::KTransactionSortOptionsDlg(QWidget* parent)
    : QDialog(parent)
    , d_ptr(new KTransactionSortOptionsDlgPrivate(this))
{
}

KTransactionSortOptionsDlg::~KTransactionSortOptionsDlg()
{
    Q_D(KTransactionSortOptionsDlg);
    delete d;
}

void KTransactionSortOptionsDlg::setSortOption(const QString& option, const QString& def)
{
    Q_D(KTransactionSortOptionsDlg);
    if (option.isEmpty()) {
        d->ui->m_sortOption->setSettings(def);
        d->ui->m_useDefault->setChecked(true);
    } else {
        d->ui->m_sortOption->setSettings(option);
        d->ui->m_useDefault->setChecked(false);
    }
}

QString KTransactionSortOptionsDlg::sortOption() const
{
    Q_D(const KTransactionSortOptionsDlg);
    QString rc;
    if (!d->ui->m_useDefault->isChecked()) {
        rc = d->ui->m_sortOption->settings();
    }
    return rc;
}

void KTransactionSortOptionsDlg::hideDefaultButton()
{
    Q_D(KTransactionSortOptionsDlg);
    d->ui->m_useDefault->hide();
}
