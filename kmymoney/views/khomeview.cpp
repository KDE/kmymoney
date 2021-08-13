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
#include <QPrintPreviewDialog>

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

void KHomeView::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    Q_UNUSED(selections)

    Q_D(KHomeView);
    switch (action) {
    case eMenu::Action::Print:
        if (d->isActiveView()) {
            slotPrintView();
        }
        break;
    case eMenu::Action::PrintPreview:
        if (d->isActiveView()) {
            slotPrintPreviewView();
        }
        break;
    case eMenu::Action::FileClose:
        d->m_view->setHtml(KWelcomePage::welcomePage(), QUrl("file://"));
        break;
    default:
        break;
    }
}

void KHomeView::executeCustomAction(eView::Action action)
{
    switch(action) {
    case eView::Action::Refresh:
        refresh();
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

void KHomeView::slotPrintPreviewView()
{
    Q_D(KHomeView);
    if (d->m_view) {
        QPrintPreviewDialog dlg(KMyMoneyPrinter::instance(), d->m_view);
        connect(&dlg, &QPrintPreviewDialog::paintRequested, d->m_view, [&](QPrinter* printer) {
            Q_D(KHomeView);
#ifdef ENABLE_WEBENGINE
            QEventLoop loop;
            bool result = true;
            auto printPreview = [&](bool success) {
                result = success;
                loop.quit();
            };
            d->m_view->page()->print(printer, std::move(printPreview));
            loop.exec();
            if (!result) {
                QPainter painter;
                if (painter.begin(printer)) {
                    QFont font = painter.font();
                    font.setPixelSize(20);
                    painter.setFont(font);
                    painter.drawText(QPointF(10, 25), QStringLiteral("Could not generate print preview."));
                    painter.end();
                }
            }
#else
            d->m_view->print(printer);
#endif
        });
        dlg.exec();
    }
}

void KHomeView::slotOpenUrl(const QUrl &url)
{
    Q_D(KHomeView);

    auto triggerAction = [&](eMenu::Action action, const QString& id) {
        pActions[action]->setData(id);
        emit requestActionTrigger(action);
    };

    QString protocol = url.scheme();
    QString view = url.fileName();

    // empty view -> bail out
    if (view.isEmpty())
        return;

    QUrlQuery query(url);
    QString id = query.queryItemValue("id");
    QString mode = query.queryItemValue("mode");

    if (protocol == QLatin1String("https")) {
        QDesktopServices::openUrl(url);
    } else if (protocol == QLatin1String("mailto")) {
        QDesktopServices::openUrl(url);
    } else {
        KXmlGuiWindow* mw = KMyMoneyUtils::mainWindow();
        Q_CHECK_PTR(mw);
        if (view == VIEW_LEDGER) {
            pActions[eMenu::Action::GoToAccount]->setData(id);
            emit requestActionTrigger(eMenu::Action::GoToAccount);

        } else if (view == VIEW_SCHEDULE) {
            if (mode == QLatin1String("enter")) {
                triggerAction(eMenu::Action::EnterSchedule, id);

            } else if (mode == QLatin1String("edit")) {
                triggerAction(eMenu::Action::EditSchedule, id);

            } else if (mode == QLatin1String("skip")) {
                triggerAction(eMenu::Action::SkipSchedule, id);

            } else if (mode == QLatin1String("full")) {
                d->m_showAllSchedules = true;
                d->loadView();

            } else if (mode == QLatin1String("reduced")) {
                d->m_showAllSchedules = false;
                d->loadView();
            }

        } else if (view == VIEW_REPORTS) {
            triggerAction(eMenu::Action::ReportOpen, id);

        } else if (view == VIEW_WELCOME) {
            if (mode == QLatin1String("whatsnew"))
                d->m_view->setHtml(KWelcomePage::whatsNewPage(), QUrl("file://"));
            else
                d->m_view->setHtml(KWelcomePage::welcomePage(), QUrl("file://"));

        } else if (view == QLatin1String("action")) {
            QMetaObject::invokeMethod(mw->actionCollection()->action(id), "trigger");

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

void KHomeView::slotSettingsChanged()
{
    refresh();
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
