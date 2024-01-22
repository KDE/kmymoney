/*
    SPDX-FileCopyrightText: 2007-2011 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kconfirmmanualenterdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QRadioButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KTextEdit>
#include <KMessageBox>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kconfirmmanualenterdlg.h"

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneypayee.h"
#include "mymoneysplit.h"
#include "mymoneyschedule.h"
#include "mymoneyexception.h"
#include "kmymoneyutils.h"
#include "mymoneytransaction.h"
#include "mymoneyenums.h"

KConfirmManualEnterDlg::KConfirmManualEnterDlg(const MyMoneySchedule& schedule, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::KConfirmManualEnterDlg)
{
    ui->setupUi(this);
    ui->buttonGroup1->setId(ui->m_discardRadio, 0);
    ui->buttonGroup1->setId(ui->m_onceRadio, 1);
    ui->buttonGroup1->setId(ui->m_setRadio, 2);

    ui->m_onceRadio->setChecked(true);

    if (schedule.type() == eMyMoney::Schedule::Type::LoanPayment) {
        ui->m_setRadio->setEnabled(false);
        ui->m_discardRadio->setEnabled(false);
    }
}

KConfirmManualEnterDlg::~KConfirmManualEnterDlg()
{
    delete ui;
}

void KConfirmManualEnterDlg::loadTransactions(const MyMoneyTransaction& to, const MyMoneyTransaction& tn)
{
    QString messageDetail("<qt>");
    auto file = MyMoneyFile::instance();

    const auto toSplits = to.splits();
    const auto tnSplits = tn.splits();

    try {
        if (toSplits.isEmpty())
            throw MYMONEYEXCEPTION(i18n("Transaction %1 has no splits", to.id()));
        if (tnSplits.isEmpty())
            throw MYMONEYEXCEPTION(i18n("Transaction %1 has no splits", tn.id()));

        QString po, pn;
        if (!toSplits.front().payeeId().isEmpty())
            po = file->payee(toSplits.front().payeeId()).name();
        if (!tnSplits.front().payeeId().isEmpty())
            pn = file->payee(tnSplits.front().payeeId()).name();

        if (po != pn) {
            messageDetail += i18n("<p>Payee changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", po, pn);
        }

        if (toSplits.front().accountId() != tnSplits.front().accountId()) {
            messageDetail += i18n("<p>Account changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>",
                                  file->account(toSplits.front().accountId()).name(),
                                  file->account(tnSplits.front().accountId()).name());
        }

        if (file->isTransfer(to) && file->isTransfer(tn)) {
            if (toSplits.at(1).accountId() != tnSplits.at(1).accountId()) {
                messageDetail += i18n("<p>Transfer account changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>",
                                      file->account(toSplits.at(1).accountId()).name(),
                                      file->account(tnSplits.at(1).accountId()).name());
            }
        } else {
            QString co, cn;
            switch (to.splitCount()) {
            default:
                co = i18nc("Split transaction (category replacement)", "Split transaction");
                break;
            case 2:
                co = file->accountToCategory(toSplits.at(1).accountId());
            case 1:
                break;
            }

            switch (tn.splitCount()) {
            default:
                cn = i18nc("Split transaction (category replacement)", "Split transaction");
                break;
            case 2:
                cn = file->accountToCategory(tnSplits.at(1).accountId());
            case 1:
                break;
            }
            if (co != cn) {
                messageDetail += i18n("<p>Category changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", co, cn);
            }
        }

        QString mo, mn;
        mo = toSplits.front().memo();
        mn = tnSplits.front().memo();
        if (mo.isEmpty())
            mo = QString("<i>") + i18nc("Empty memo", "empty") + QString("</i>");
        if (mn.isEmpty())
            mn = QString("<i>") + i18nc("Empty memo", "empty") + QString("</i>");
        if (mo != mn) {
            messageDetail += i18n("<p>Memo changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", mo, mn);
        }

        QString no, nn;
        no = toSplits.front().number();
        nn = tnSplits.front().number();
        if (no.isEmpty())
            no = QString("<i>") + i18nc("No number", "empty") + QString("</i>");
        if (nn.isEmpty())
            nn = QString("<i>") + i18nc("No number", "empty") + QString("</i>");
        if (no != nn) {
            messageDetail += i18n("<p>Number changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", no, nn);
        }

        const MyMoneySecurity& sec = MyMoneyFile::instance()->security(to.commodity());
        MyMoneyMoney ao, an;
        ao = toSplits.front().value();
        an = tnSplits.front().value();
        if (ao != an) {
            messageDetail += i18n("<p>Amount changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", ao.formatMoney(sec.smallestAccountFraction()), an.formatMoney(sec.smallestAccountFraction()));
        }

        eMyMoney::Split::State fo, fn;
        fo = toSplits.front().reconcileFlag();
        fn = tnSplits.front().reconcileFlag();
        if (fo != fn) {
            messageDetail += i18n("<p>Reconciliation flag changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>",    KMyMoneyUtils::reconcileStateToString(fo, true), KMyMoneyUtils::reconcileStateToString(fn, true));
        }
    } catch (const MyMoneyException& e) {
        KMessageBox::error(this, i18n("Fatal error in determining data: %1", QString::fromLatin1(e.what())));
    }

    messageDetail += "</qt>";
    ui->m_details->setText(messageDetail);
    return;
}

KConfirmManualEnterDlg::Action KConfirmManualEnterDlg::action() const
{
    if (ui->m_discardRadio->isChecked())
        return UseOriginal;
    if (ui->m_setRadio->isChecked())
        return ModifyAlways;
    return ModifyOnce;
}
