/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kenterscheduledlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QIcon>
#include <QKeyEvent>
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
#include "taborder.h"

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
        , m_tabOrder(QLatin1String("enterScheduleTransactionEditor"),
                     QStringList{
                         // the std transaction editor (see also newtransactioneditor.cpp
                         QLatin1String("dateEdit"),
                         QLatin1String("creditDebitEdit"),
                         QLatin1String("payeeEdit"),
                         QLatin1String("numberEdit"),
                         QLatin1String("categoryCombo"),
                         QLatin1String("costCenterCombo"),
                         QLatin1String("tagContainer"),
                         QLatin1String("statusCombo"),
                         QLatin1String("memoEdit"),
                         // the schedule options
                         QLatin1String("buttonHelp"),
                         QLatin1String("buttonOk"),
                         QLatin1String("buttonSkip"),
                         QLatin1String("buttonIgnore"),
                         QLatin1String("buttonCancel"),
                     })
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
    TabOrder m_tabOrder;
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
    d->m_editor->setShowAccountCombo(true);
    d->m_editor->layout()->setContentsMargins(0, 0, 0, 0);

    // in case transaction editor is asked for the tab order
    // it should point back to ours
    d->m_editor->setExternalTabOrder(&d->m_tabOrder);

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
            d->m_editor->loadSchedule(loanSchedule, NewTransactionEditor::EnterSchedule);
        } else {
            d->m_editor->loadSchedule(d->m_schedule, NewTransactionEditor::EnterSchedule);
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

    // we delay setting up the tab order until we drop back to the event loop.
    // this will make sure that all widgets are visible and the tab order logic
    // works properly
    QMetaObject::invokeMethod(
        this,
        [&]() {
            Q_D(KEnterScheduleDlg);
            d->m_tabOrder.setWidget(this);

            const auto widget = d->m_tabOrder.initialFocusWidget(nullptr);
            if (widget) {
                QMetaObject::invokeMethod(widget, "setFocus", Qt::QueuedConnection);
            }
        },
        Qt::QueuedConnection);
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

bool KEnterScheduleDlg::focusNextPrevChild(bool next)
{
    Q_D(KEnterScheduleDlg);
    const auto widget = d->m_tabOrder.tabFocusHelper(next);

    if (widget != nullptr) {
        widget->setFocus(next ? Qt::TabFocusReason : Qt::BacktabFocusReason);
        return true;
    }
    // In case we're going backward from the buttonbox at the bottom,
    // the default focusNextPrevChild() implementation sets the focus
    // to the account combo box instead of the last option. We catch
    // this here and set the focus accordingly.
    const auto pushButton = focusWidget();
    const auto dialogButtonBox = focusWidget()->parentWidget();
    if (pushButton->qt_metacast("QPushButton") && dialogButtonBox->qt_metacast("QDialogButtonBox") && (!next)) {
        if (dialogButtonBox->nextInFocusChain() == pushButton) {
            const auto widgetName = focusWidget()->parentWidget()->objectName();
            const QStringList tabOrder(property("kmm_currenttaborder").toStringList());
            auto idx = tabOrder.indexOf(widgetName);
            if (idx >= 0) {
                QWidget* w = nullptr;
                auto previousTabWidget = [&]() {
                    --idx;
                    if (idx < 0) {
                        idx = tabOrder.count() - 1;
                    }
                    w = findChild<QWidget*>(tabOrder.at(idx));
                };
                previousTabWidget();
                while (w && !w->isEnabled()) {
                    previousTabWidget();
                }
                if (w) {
                    w->setFocus();
                    return true;
                }
            }
        }
    }
    return QWidget::focusNextPrevChild(next);
}

void KEnterScheduleDlg::keyPressEvent(QKeyEvent* e)
{
    if (!e->modifiers() || ((e->modifiers() & Qt::KeypadModifier) && (e->key() == Qt::Key_Enter))) {
        Q_D(KEnterScheduleDlg);
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            // move focus so that some focusOutEvent handlers are called
            d->ui->buttonOk->setFocus();
            break;
        default:
            break;
        }
    }
    QDialog::keyPressEvent(e);
}
