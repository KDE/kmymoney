/***************************************************************************
                          khomeview.cpp  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "khomeview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

KHomeView::KHomeView(QWidget *parent) :
  KMyMoneyViewBase(*new KHomeViewPrivate(this), parent)
{
}

KHomeView::~KHomeView()
{
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
    d->m_currentPrinter = new QPrinter();
    QPointer<QPrintDialog> dialog = new QPrintDialog(d->m_currentPrinter, this);
    dialog->setWindowTitle(QString());
    if (dialog->exec() != QDialog::Accepted) {
      delete d->m_currentPrinter;
      d->m_currentPrinter = nullptr;
      return;
    }
    #ifdef ENABLE_WEBENGINE
    d->m_view->page()->print(d->m_currentPrinter, [=] (bool) {delete d->m_currentPrinter; d->m_currentPrinter = nullptr;});
    #else
      d->m_view->print(d->m_currentPrinter);
    #endif
  }
}

void KHomeView::slotOpenUrl(const QUrl &url)
{
  Q_D(KHomeView);
  QString protocol = url.scheme();
  QString view = url.fileName();
  if (view.isEmpty())
    return;
  QUrlQuery query(url);
  QString id = query.queryItemValue("id");
  QString mode = query.queryItemValue("mode");

  const auto file = MyMoneyFile::instance();

  if (protocol == QLatin1String("http")) {
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
      MyMoneyFile::instance()->accountList(list);
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
