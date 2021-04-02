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

    try {
        int noItemsChanged = 0;

        if (to.splits().isEmpty())
            throw MYMONEYEXCEPTION(i18n("Transaction %1 has no splits", to.id()));
        if (tn.splits().isEmpty())
            throw MYMONEYEXCEPTION(i18n("Transaction %1 has no splits", tn.id()));

        QString po, pn;
        if (!to.splits().front().payeeId().isEmpty())
            po = file->payee(to.splits().front().payeeId()).name();
        if (!tn.splits().front().payeeId().isEmpty())
            pn = file->payee(tn.splits().front().payeeId()).name();

        if (po != pn) {
            noItemsChanged++;
            messageDetail += i18n("<p>Payee changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", po, pn);
        }

        if (to.splits().front().accountId() != tn.splits().front().accountId()) {
            noItemsChanged++;
            messageDetail += i18n("<p>Account changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>"
                                  , file->account(to.splits().front().accountId()).name()
                                  , file->account(tn.splits().front().accountId()).name());
        }

        if (file->isTransfer(to) && file->isTransfer(tn)) {
            if (to.splits()[1].accountId() != tn.splits()[1].accountId()) {
                noItemsChanged++;
                messageDetail += i18n("<p>Transfer account changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>"
                                      , file->account(to.splits()[1].accountId()).name()
                                      , file->account(tn.splits()[1].accountId()).name());
            }
        } else {
            QString co, cn;
            switch (to.splitCount()) {
            default:
                co = i18nc("Split transaction (category replacement)", "Split transaction");
                break;
            case 2:
                co = file->accountToCategory(to.splits()[1].accountId());
            case 1:
                break;
            }

            switch (tn.splitCount()) {
            default:
                cn = i18nc("Split transaction (category replacement)", "Split transaction");
                break;
            case 2:
                cn = file->accountToCategory(tn.splits()[1].accountId());
            case 1:
                break;
            }
            if (co != cn) {
                noItemsChanged++;
                messageDetail += i18n("<p>Category changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", co, cn);
            }
        }

        QString mo, mn;
        mo = to.splits().front().memo();
        mn = tn.splits().front().memo();
        if (mo.isEmpty())
            mo = QString("<i>") + i18nc("Empty memo", "empty") + QString("</i>");
        if (mn.isEmpty())
            mn = QString("<i>") + i18nc("Empty memo", "empty") + QString("</i>");
        if (mo != mn) {
            noItemsChanged++;
            messageDetail += i18n("<p>Memo changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", mo, mn);
        }

        QString no, nn;
        no = to.splits().front().number();
        nn = tn.splits().front().number();
        if (no.isEmpty())
            no = QString("<i>") + i18nc("No number", "empty") + QString("</i>");
        if (nn.isEmpty())
            nn = QString("<i>") + i18nc("No number", "empty") + QString("</i>");
        if (no != nn) {
            noItemsChanged++;
            messageDetail += i18n("<p>Number changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", no, nn);
        }

        const MyMoneySecurity& sec = MyMoneyFile::instance()->security(to.commodity());
        MyMoneyMoney ao, an;
        ao = to.splits().front().value();
        an = tn.splits().front().value();
        if (ao != an) {
            noItemsChanged++;
            messageDetail += i18n("<p>Amount changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>", ao.formatMoney(sec.smallestAccountFraction()), an.formatMoney(sec.smallestAccountFraction()));
        }

        eMyMoney::Split::State fo, fn;
        fo = to.splits().front().reconcileFlag();
        fn = tn.splits().front().reconcileFlag();
        if (fo != fn) {
            noItemsChanged++;
            messageDetail += i18n("<p>Reconciliation flag changed.<br/>&nbsp;&nbsp;&nbsp;Old: <b>%1</b>, New: <b>%2</b></p>",    KMyMoneyUtils::reconcileStateToString(fo, true), KMyMoneyUtils::reconcileStateToString(fn, true));
        }
    } catch (const MyMoneyException &e) {
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
