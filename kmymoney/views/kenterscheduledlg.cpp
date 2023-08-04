/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kenterscheduledlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QIcon>
#include <QWindow>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KGuiItem>
#include <KHelpClient>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardGuiItem>
#include <KWindowConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kenterscheduledlg.h"

#include "dialogenums.h"
#include "icons.h"
#include "kmymoneyutils.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"

using namespace Icons;

class KEnterScheduleDlgPrivate
{
    Q_DISABLE_COPY(KEnterScheduleDlgPrivate)

public:
    KEnterScheduleDlgPrivate()
        : ui(new Ui::KEnterScheduleDlg)
        , m_editor(nullptr)
        , m_showWarningOnce(true)
        , m_extendedReturnCode(eDialogs::ScheduleResultCode::Cancel)
    {
    }

    ~KEnterScheduleDlgPrivate()
    {
        delete ui;
    }

    Ui::KEnterScheduleDlg* ui;
    NewTransactionEditor* m_editor;
    MyMoneySchedule m_schedule;
    bool m_showWarningOnce;
    eDialogs::ScheduleResultCode m_extendedReturnCode;
};

KEnterScheduleDlg::KEnterScheduleDlg(QWidget* parent, const MyMoneySchedule& schedule)
    : QDialog(parent)
    , d_ptr(new KEnterScheduleDlgPrivate)
{
    Q_D(KEnterScheduleDlg);

    // restore the last used dialog size
    KConfigGroup grp = KSharedConfig::openConfig()->group("KEnterScheduleDlg");
    if (grp.isValid()) {
        KWindowConfig::restoreWindowSize(windowHandle(), grp);
    }
    // let the minimum size be 780x410
    resize(QSize(780, 410).expandedTo(windowHandle() ? windowHandle()->size() : QSize()));

    // position the dialog centered on the application (for some reason without
    // a call to winId() the dialog is positioned in the upper left corner of
    // the screen, but winId() crashes on MS-Windows ...
    if (parent)
        move(parent->pos() + QPoint(parent->width() / 2, parent->height() / 2) - QPoint(width() / 2, height() / 2));

    d->ui->setupUi(this);

    d->m_editor = d->ui->m_editor;
    d->m_editor->setShowButtons(false);
    d->m_editor->layout()->setContentsMargins(0, 0, 0, 0);

    d->m_schedule = schedule;
    const auto account = schedule.account();
    if (!account.id().isEmpty()) {
        d->m_editor->setAccountId(account.id());
    }

    d->m_extendedReturnCode = eDialogs::ScheduleResultCode::Enter;

    d->ui->buttonOk->setIcon(Icons::get(Icon::KeyEnter));
    d->ui->buttonSkip->setIcon(Icons::get(Icon::SeekForward));
    KGuiItem::assign(d->ui->buttonCancel, KStandardGuiItem::cancel());
    KGuiItem::assign(d->ui->buttonHelp, KStandardGuiItem::help());
    d->ui->buttonIgnore->setHidden(true);
    d->ui->buttonSkip->setHidden(true);

    try {
        if (d->m_schedule.type() == eMyMoney::Schedule::Type::LoanPayment) {
            // in case of a loan payment we need to adjust the schedule locally
            // to contain the actual values for the next transaction. We do that
            // on a copy of the schedule.
            auto loanSchedule(d->m_schedule);
            auto t = loanSchedule.transaction();
            KMyMoneyUtils::calculateAutoLoan(loanSchedule, t, QMap<QString, MyMoneyMoney>());
            loanSchedule.setTransaction(t);
            d->m_editor->loadSchedule(loanSchedule);
        } else {
            d->m_editor->loadSchedule(d->m_schedule);
            d->m_editor->setKeepCategoryAmount(d->m_schedule.keepMultiCurrencyAmount());
        }
    } catch (const MyMoneyException& e) {
        KMessageBox::detailedError(this, i18n("Unable to load schedule details"), QString::fromLatin1(e.what()));
    }

    // setup name and type
    d->ui->m_scheduleName->setText(d->m_schedule.name());
    d->ui->m_type->setText(KMyMoneyUtils::scheduleTypeToString(d->m_schedule.type()));

    connect(d->ui->buttonHelp, &QAbstractButton::clicked, this, [&]() {
        KHelpClient::invokeHelp("details.schedules.entering");
    });

    connect(d->ui->buttonIgnore, &QAbstractButton::clicked, this, [&]() {
        Q_D(KEnterScheduleDlg);
        d->m_extendedReturnCode = eDialogs::ScheduleResultCode::Ignore;
        accept();
    });
    connect(d->ui->buttonSkip, &QAbstractButton::clicked, this, [&]() {
        Q_D(KEnterScheduleDlg);
        d->m_extendedReturnCode = eDialogs::ScheduleResultCode::Skip;
        accept();
    });
}

KEnterScheduleDlg::~KEnterScheduleDlg()
{
    Q_D(KEnterScheduleDlg);

    // store the last used dialog size
    KConfigGroup grp = KSharedConfig::openConfig()->group("KEnterScheduleDlg");
    if (grp.isValid()) {
        KWindowConfig::saveWindowSize(windowHandle(), grp);
    }

    delete d;
}

eDialogs::ScheduleResultCode KEnterScheduleDlg::resultCode() const
{
    Q_D(const KEnterScheduleDlg);
    if (result() == QDialog::Accepted)
        return d->m_extendedReturnCode;
    return eDialogs::ScheduleResultCode::Cancel;
}

void KEnterScheduleDlg::setShowExtendedKeys(bool visible)
{
    Q_D(KEnterScheduleDlg);
    d->ui->buttonIgnore->setVisible(visible);
    d->ui->buttonSkip->setVisible(visible);
}

MyMoneyTransaction KEnterScheduleDlg::transaction()
{
    Q_D(const KEnterScheduleDlg);
    auto t = d->m_editor->transaction();
    t.clearId();
    t.setEntryDate(QDate());
    return t;
}

QDate KEnterScheduleDlg::date(const QDate& _date) const
{
    Q_D(const KEnterScheduleDlg);
    auto date(_date);
    return d->m_schedule.adjustedDate(date, d->m_schedule.weekendOption());
}

int KEnterScheduleDlg::exec()
{
    Q_D(KEnterScheduleDlg);
    if (d->m_showWarningOnce) {
        d->m_showWarningOnce = false;
        KMessageBox::information(parentWidget(),
                                 QString("<qt>%1</qt>")
                                     .arg(i18n("<p>Please check that all the details in the following dialog are correct and press OK.</p><p>Editable data can "
                                               "be changed and can either be applied to just this occurrence or for all subsequent occurrences for this "
                                               "schedule.  (You will be asked what you intend after pressing OK in the following dialog)</p>")),
                                 i18n("Enter scheduled transaction"),
                                 "EnterScheduleDlgInfo");
    }

    return QDialog::exec();
}
