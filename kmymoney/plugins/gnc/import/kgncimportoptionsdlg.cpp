/*
    SPDX-FileCopyrightText: 2005 Tony Bloomfield <tonybloom@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgncimportoptionsdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QByteArray>
#include <QCheckBox>
#include <QList>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif
// ----------------------------------------------------------------------------
// KDE Includes

#include <KHelpClient>
#include <QDialogButtonBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_codec.h"
#include "ui_kgncimportoptionsdlg.h"

class KGncImportOptionsDlgPrivate
{
    Q_DISABLE_COPY(KGncImportOptionsDlgPrivate)
    Q_DECLARE_PUBLIC(KGncImportOptionsDlg)

public:
    explicit KGncImportOptionsDlgPrivate(KGncImportOptionsDlg* qq)
        : q_ptr(qq)
        , ui(new Ui::KGncImportOptionsDlg)
    {
    }

    ~KGncImportOptionsDlgPrivate()
    {
        delete ui;
    }

    void init()
    {
        Q_Q(KGncImportOptionsDlg);
        ui->setupUi(q);

        ui->buttonInvestGroup->setId(ui->radioInvest1, 0); // one invest acct per stock
        ui->buttonInvestGroup->setId(ui->radioInvest2, 1); // one invest acct for all stocks
        ui->buttonInvestGroup->setId(ui->radioInvest3, 2); // prompt for each stock

        ui->buttonGroup5->setExclusive(false);
        ui->checkFinanceQuote->setChecked(true);

        ui->buttonGroup2->setExclusive(false);
        ui->checkSchedules->setChecked(false);

        buildCodecList();  // build list of codecs and insert into combo box

        ui->buttonGroup4->setExclusive(false);
        ui->checkDecode->setChecked(false);
        ui->comboDecode->setEnabled(false);

        ui->buttonGroup18->setExclusive(false);
        ui->checkTxNotes->setChecked(false);

        ui->buttonGroup3->setExclusive(false);
        ui->checkDebugGeneral->setChecked(false);
        ui->checkDebugXML->setChecked(false);
        ui->checkAnonymize->setChecked(false);

        q->connect(ui->checkDecode, &QAbstractButton::toggled, q, &KGncImportOptionsDlg::slotDecodeOptionChanged);
        q->connect(ui->buttonBox, &QDialogButtonBox::helpRequested, q, &KGncImportOptionsDlg::slotHelp);
    }

    void buildCodecList()
    {
        KMM_Codec::loadComboBox(ui->comboDecode);
    }

    KGncImportOptionsDlg      *q_ptr;
    Ui::KGncImportOptionsDlg* ui;
};

KGncImportOptionsDlg::KGncImportOptionsDlg(QWidget *parent) :
    QDialog(parent),
    d_ptr(new KGncImportOptionsDlgPrivate(this))
{
    Q_D(KGncImportOptionsDlg);
    d->init();
}

KGncImportOptionsDlg::~KGncImportOptionsDlg()
{
    Q_D(KGncImportOptionsDlg);
    delete d;
}

// enable the combo box for selection if required
void KGncImportOptionsDlg::slotDecodeOptionChanged(bool isOn)
{
    Q_D(KGncImportOptionsDlg);
    if (isOn) {
        d->ui->comboDecode->setEnabled(true);
        d->ui->comboDecode->setCurrentItem(nullptr);
    } else {
        d->ui->comboDecode->setEnabled(false);
    }
}

int KGncImportOptionsDlg::investmentOption() const
{
    Q_D(const KGncImportOptionsDlg);
    return (d->ui->buttonInvestGroup->checkedId());
};

bool KGncImportOptionsDlg::quoteOption() const
{
    Q_D(const KGncImportOptionsDlg);
    return (d->ui->checkFinanceQuote->isChecked());
};

bool KGncImportOptionsDlg::scheduleOption() const
{
    Q_D(const KGncImportOptionsDlg);
    return (d->ui->checkSchedules->isChecked());
};

// return selected codec or 0
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QTextCodec* KGncImportOptionsDlg::decodeOption()
{
    Q_D(const KGncImportOptionsDlg);
    if (!d->ui->checkDecode->isChecked()) {
        return nullptr;
    } else {
        return (QTextCodec::codecForName(d->ui->comboDecode->currentText().toUtf8()));
    }
}
#endif

bool KGncImportOptionsDlg::txNotesOption() const
{
    Q_D(const KGncImportOptionsDlg);
    return (d->ui->checkTxNotes->isChecked());
}

bool KGncImportOptionsDlg::generalDebugOption() const
{
    Q_D(const KGncImportOptionsDlg);
    return (d->ui->checkDebugGeneral->isChecked());
}

bool KGncImportOptionsDlg::xmlDebugOption() const
{
    Q_D(const KGncImportOptionsDlg);
    return (d->ui->checkDebugXML->isChecked());
}

bool KGncImportOptionsDlg::anonymizeOption() const
{
    Q_D(const KGncImportOptionsDlg);
    return (d->ui->checkAnonymize->isChecked());
}

void KGncImportOptionsDlg::slotHelp()
{
    KHelpClient::invokeHelp("details.impexp.gncoptions");
}
