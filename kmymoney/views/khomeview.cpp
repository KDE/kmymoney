/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2002-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "khomeview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_printer.h"

KHomeView::KHomeView(QWidget *parent) :
    KMyMoneyViewBase(*new KHomeViewPrivate(this), parent)
{
}

KHomeView::~KHomeView()
{
}

void KHomeView::slotAdjustScrollPos()
{
#ifndef ENABLE_WEBENGINE
    Q_D(KHomeView);
    if (d && d->m_view && d->m_view->page() && d->m_view->page()->mainFrame()) {
        d->m_view->page()->mainFrame()->setScrollBarValue(Qt::Vertical, d->m_scrollBarPos);
    }
#endif
}

bool KHomeView::eventFilter(QObject* o, QEvent* e)
{
    Q_D(KHomeView);
    if (o == d->m_view) {
        // we simply suppress the context menu as it
        // does not provide useful functions
        if (e->type() == QEvent::ContextMenu) {
            return true;
        }
    }
    return KMyMoneyViewBase::eventFilter(o, e);
}

void KHomeView::wheelEvent(QWheelEvent* event)
{
    Q_D(KHomeView);
    // Zoom text on Ctrl + Scroll
    if (event->modifiers() & Qt::CTRL) {
        qreal factor = d->m_view->zoomFactor();
        if (event->delta() > 0)
            factor += 0.1;
        else if (event->delta() < 0)
            factor -= 0.1;
        d->m_view->setZoomFactor(factor);
        event->accept();
        return;
    }
}

void KHomeView::executeCustomAction(eView::Action action)
{
    Q_D(KHomeView);
    switch(action) {
    case eView::Action::Refresh:
        refresh();
        break;

    case eView::Action::Print:
        slotPrintView();
        break;

    case eView::Action::CleanupBeforeFileClose:
        d->m_view->setHtml(KWelcomePage::welcomePage(), QUrl("file://"));
        break;

    default:
        break;
    }
}

void KHomeView::refresh()
{
    Q_D(KHomeView);
    if (isVisible()) {
        d->loadView();
        d->m_needsRefresh = false;
    } else {
        d->m_needsRefresh = true;
    }
}

void KHomeView::showEvent(QShowEvent* event)
{
    Q_D(KHomeView);
    if (d->m_needLoad)
        d->init();

    emit customActionRequested(View::Home, eView::Action::AboutToShow);

    if (d->m_needsRefresh)
        refresh();

    QWidget::showEvent(event);
}

void KHomeView::slotPrintView()
{
    Q_D(KHomeView);
    if (d->m_view) {
        auto printer = KMyMoneyPrinter::startPrint();
        if (printer != nullptr) {
#ifdef ENABLE_WEBENGINE
            d->m_view->page()->print(printer, [=] (bool) {});
#else
            d->m_view->print(printer);
#endif
        }
    }
}

void KHomeView::slotOpenUrl(const QUrl &url)
{
    Q_D(KHomeView);

    QString protocol = url.scheme();
    QString view = url.fileName();

    // empty view -> bail out
    if (view.isEmpty())
        return;

    QUrlQuery query(url);
    QString id = query.queryItemValue("id");
    QString mode = query.queryItemValue("mode");

    const auto file = MyMoneyFile::instance();

    if (protocol == QLatin1String("https")) {
        QDesktopServices::openUrl(url);
    } else if (protocol == QLatin1String("mailto")) {
        QDesktopServices::openUrl(url);
    } else {
        KXmlGuiWindow* mw = KMyMoneyUtils::mainWindow();
        Q_CHECK_PTR(mw);
        if (view == VIEW_LEDGER) {
            emit selectByVariant(QVariantList {QVariant(id), QVariant(QString())}, eView::Intent::ShowTransaction);

        } else if (view == VIEW_SCHEDULE) {
            if (mode == QLatin1String("enter")) {
                emit selectByObject(file->schedule(id), eView::Intent::None);
                QTimer::singleShot(0, pActions[eMenu::Action::EnterSchedule], SLOT(trigger()));
            } else if (mode == QLatin1String("edit")) {
                emit selectByObject(file->schedule(id), eView::Intent::None);
                QTimer::singleShot(0, pActions[eMenu::Action::EditSchedule], SLOT(trigger()));
            } else if (mode == QLatin1String("skip")) {
                emit selectByObject(file->schedule(id), eView::Intent::None);
                QTimer::singleShot(0, pActions[eMenu::Action::SkipSchedule], SLOT(trigger()));
            } else if (mode == QLatin1String("full")) {
                d->m_showAllSchedules = true;
                d->loadView();

            } else if (mode == QLatin1String("reduced")) {
                d->m_showAllSchedules = false;
                d->loadView();
            }

        } else if (view == VIEW_REPORTS) {
            emit selectByObject(file->report(id), eView::Intent::OpenObject);
//      emit openObjectRequested(file->report(id));

        } else if (view == VIEW_WELCOME) {
            if (mode == QLatin1String("whatsnew"))
                d->m_view->setHtml(KWelcomePage::whatsNewPage(), QUrl("file://"));
            else
                d->m_view->setHtml(KWelcomePage::welcomePage(), QUrl("file://"));

        } else if (view == QLatin1String("action")) {
            QTimer::singleShot(0, mw->actionCollection()->action(id), SLOT(trigger()));
        } else if (view == VIEW_HOME) {
            QList<MyMoneyAccount> list;
            // it could be, that we don't even have a storage object attached.
            // in this case the call to accountList() will throw an MyMoneyException
            // which we catch here and treat it as 'no accounts found'.
            try {
                MyMoneyFile::instance()->accountList(list);
            } catch(const MyMoneyException& e) {
            }
            if (list.count() == 0) {
                KMessageBox::information(this, i18n("Before KMyMoney can give you detailed information about your financial status, you need to create at least one account. Until then, KMyMoney shows the welcome page instead."));
            }
            d->loadView();

        } else {
            qDebug("Unknown view '%s' in slotOpenURL()", qPrintable(view));
        }
    }
}
// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef VIEW_LEDGER
#undef VIEW_SCHEDULE
#undef VIEW_WELCOME
#undef VIEW_HOME
#undef VIEW_REPORTS
