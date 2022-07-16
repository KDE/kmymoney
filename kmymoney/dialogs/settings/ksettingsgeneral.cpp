/*
    SPDX-FileCopyrightText: 2005-2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ksettingsgeneral.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFileDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsgeneral.h"

#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "widgethintframe.h"

class KSettingsGeneralPrivate
{
    Q_DISABLE_COPY(KSettingsGeneralPrivate)

public:
    KSettingsGeneralPrivate() :
        ui(new Ui::KSettingsGeneral),
        initialHideZeroBalanceEquities(false)
    {
    }

    ~KSettingsGeneralPrivate()
    {
        delete ui;
    }

    Ui::KSettingsGeneral *ui;
    bool initialHideZeroBalanceEquities;
};

KSettingsGeneral::KSettingsGeneral(QWidget* parent) :
    QWidget(parent),
    d_ptr(new KSettingsGeneralPrivate)
{
    Q_D(KSettingsGeneral);
    d->ui->setupUi(this);

    auto frameCollection = new WidgetHintFrameCollection(this);
    frameCollection->addFrame(new WidgetHintFrame(d->ui->kcfg_StartDate));
    connect(frameCollection, &WidgetHintFrameCollection::inputIsValid, this, &KSettingsGeneral::haveValidInput);
    connect(d->ui->kcfg_StartDate, &KMyMoneyDateEdit::dateValidityChanged, this, [&](const QDate& date) {
        Q_D(KSettingsGeneral);
        WidgetHintFrame::hide(d->ui->kcfg_StartDate, QString());
        if (!date.isValid()) {
            WidgetHintFrame::show(d->ui->kcfg_StartDate, i18nc("@info:tooltip", "The date is invalid."));
        }
    });
    connect(d->ui->choosePath, &QAbstractButton::pressed, this, &KSettingsGeneral::slotChooseLogPath);
    d->initialHideZeroBalanceEquities = d->ui->kcfg_HideZeroBalanceEquities->isChecked();
}

KSettingsGeneral::~KSettingsGeneral()
{
    Q_D(KSettingsGeneral);
    delete d;
}

void KSettingsGeneral::slotChooseLogPath()
{
    Q_D(KSettingsGeneral);
    QString filePath = QFileDialog::getExistingDirectory(this, i18n("Choose file path"), QDir::homePath());
    d->ui->kcfg_logPath->setText(filePath);
    slotUpdateLogTypes();
}

void KSettingsGeneral::slotUpdateLogTypes()
{
    Q_D(KSettingsGeneral);
    bool enable = d->ui->kcfg_logPath->text().isEmpty() ? false : true;
    d->ui->kcfg_logImportedStatements->setEnabled(enable);
    d->ui->kcfg_logOfxTransactions->setEnabled(enable);
    if (!enable) {
        d->ui->kcfg_logImportedStatements->setChecked(enable);
        d->ui->kcfg_logOfxTransactions->setChecked(enable);
    }
}

void KSettingsGeneral::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    QWidget::showEvent(event);
    slotUpdateLogTypes();
}
