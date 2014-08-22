/***************************************************************************
                          main.cpp
                             -------------------
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <config-kmymoney.h>
#include <config-kmymoney-version.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
#include <QDateTime>
#include <QStringList>
#include <QEventLoop>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QCommandLineParser>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KAboutData>
#include <KStartupInfo>
#include <klocale.h>
#include <ktip.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyfile.h"
#include "kmymoney.h"
#include "kstartuplogo.h"
#include "kmymoneyutils.h"
#include "kmymoneyglobalsettings.h"


QTime timer;
bool timersOn = false;

KMyMoneyApp* kmymoney;

static int runKMyMoney(QApplication *a, KStartupLogo *splash, const QUrl & file, bool noFile);

int main(int argc, char *argv[])
{
  timer.start();

  /**
   * Create application first
   */
  QApplication app(argc, argv);

  /**
   * construct about data
   */
  QStringList features;
#if KF5Gpgmepp_FOUND
  features << i18n("GPG encryption");
#endif
#if KMM_ADDRESSBOOK_FOUND
  features << i18n("Addressbook integration");
#endif
#if KF5Holidays_FOUND
  features << i18n("Holiday regions integration");
#endif
  QString featuresDescription;
  if (!features.empty())
      featuresDescription = i18n("Compiled with the following optional features:\n%1", features.join(QLatin1Char('\n')));
  KAboutData aboutData(QStringLiteral("kmymoney"), i18n("KMyMoney"), QStringLiteral(VERSION),
                       i18n("\nKMyMoney, the Personal Finance Manager for KDE.\n\nPlease consider contributing to this project with code and/or suggestions."), KAboutLicense::GPL,
                       i18n("(c) 2000-2014 The KMyMoney development team"),
                       featuresDescription,
                       QStringLiteral("http://kmymoney.org/"));

  //Temporarily, the product name to report bugs in BKO is different than the application name
  aboutData.setProductName("kmymoney");

  aboutData.addAuthor(i18n("Michael Edwardes."), i18n("Initial idea, much initial source code, Project admin"), "mte@users.sourceforge.net");
  aboutData.addAuthor(i18n("Thomas Baumgart"), i18n("Core engine, Release Manager, Project admin"), "ipwizard@users.sourceforge.net");
  aboutData.addAuthor(i18n("Ace Jones"), i18n("Reporting logic, OFX Import"), "acejones@users.sourceforge.net");
  aboutData.addAuthor(i18n("Tony Bloomfield"), i18n("Database backend, maintainer stable branch"), "tonybloom@users.sourceforge.net");
  aboutData.addAuthor(i18n("Alvaro Soliverez"), i18n("Forecast, Reports"), "asoliverez@gmail.com");
  aboutData.addAuthor(i18n("Felix Rodriguez"), i18n("Project Admin"), "frodriguez@users.sourceforge.net");
  aboutData.addAuthor(i18n("John C"), i18n("Developer"), "tacoturtle@users.sourceforge.net");
  aboutData.addAuthor(i18n("Fernando Vilas"), i18n("Database backend"), "fvilas@iname.com");
  aboutData.addAuthor(i18n("Cristian Oneț"), i18n("Developer"), "onet.cristian@gmail.com");

  aboutData.addCredit(i18n("Kevin Tambascio"), i18n("Initial investment support"), "ktambascio@users.sourceforge.net");
  aboutData.addCredit(i18n("Javier Campos Morales"), i18n("Developer & Artist"), "javi_c@users.sourceforge.net");
  aboutData.addCredit(i18n("Robert Wadley"), i18n("Icons & splash screen"), "rob@robntina.fastmail.us");
  aboutData.addCredit(i18n("Laurent Montel"), i18n("Patches and port to kde4"), "montel@kde.org");
  aboutData.addCredit(i18n("Wolfgang Rohdewald"), i18n("Patches"), "woro@users.sourceforge.net");
  aboutData.addCredit(i18n("Marko Käning"), i18n("Patches and packaging for OS X"), "mk-lists@email.de");
  aboutData.setOrganizationDomain("kde.org");

  /**
   * register about data
   */
  KAboutData::setApplicationData(aboutData);

  /**
   * take component name and org. name from KAboutData
   */
  app.setApplicationName(aboutData.componentName());
  app.setApplicationDisplayName(aboutData.displayName());
  app.setOrganizationDomain(aboutData.organizationDomain());
  app.setApplicationVersion(aboutData.version());
  app.setQuitOnLastWindowClosed(false);

  /**
   * Create command line parser and feed it with known options
   */
  QCommandLineParser parser;
  aboutData.setupCommandLine(&parser);
  parser.setApplicationDescription(aboutData.shortDescription());
  parser.addHelpOption();
  parser.addVersionOption();

  // language
  const QCommandLineOption langOption(QStringLiteral("lang"), i18n("language to be used"));
  parser.addOption(langOption);

  // no file
  const QCommandLineOption noFileOption(QStringLiteral("n"), i18n("do not open last used file"));
  parser.addOption(noFileOption);

  // timers
  const QCommandLineOption timersOption(QStringLiteral("timers"), i18n("enable performance timers"));
  parser.addOption(timersOption);

  // no catch
  const QCommandLineOption noCatchOption(QStringLiteral("nocatch"), i18n("do not globally catch uncaught exceptions"));
  parser.addOption(noCatchOption);

#ifdef KMM_DEBUG
  // The following options are only available when compiled in debug mode
  // trace
  const QCommandLineOption traceOption(QStringLiteral("trace"), i18n("turn on program traces"));
  parser.addOption(traceOption);

  // dump actions
  const QCommandLineOption dumpActionsOption(QStringLiteral("dump-actions"), i18n("dump the names of all defined QAction objects to stdout and quit"));
  parser.addOption(dumpActionsOption);
#endif

  // INSERT YOUR COMMANDLINE OPTIONS HERE
  // url to open
  parser.addPositionalArgument(QStringLiteral("url"), i18n("file to open"));

  /**
   * do the command line parsing
   */
  parser.process(app);

  /**
   * handle standard options
   */
  aboutData.processCommandLine(&parser);

  // create the singletons before we start memory checking
  // to avoid false error reports
  MyMoneyFile::instance();

  KMyMoneyUtils::checkConstants();

  if (KLocale::global()->monetaryDecimalSymbol().isEmpty()) {
    KMessageBox::error(0, i18n("The monetary decimal symbol is not correctly set in the KDE System Settings module Country/Region & Language. Please set it to a reasonable value and start KMyMoney again."), i18n("Invalid settings"));
    exit(1);
  }

  // show startup logo
  KStartupLogo* splash = new KStartupLogo();
  app.processEvents();

  // setup the MyMoneyMoney locale settings according to the KDE settings
  MyMoneyMoney::setThousandSeparator(KLocale::global()->monetaryThousandsSeparator()[0]);
  MyMoneyMoney::setDecimalSeparator(KLocale::global()->monetaryDecimalSymbol()[0]);
  MyMoneyMoney::setNegativeMonetarySignPosition(static_cast<MyMoneyMoney::signPosition>(KLocale::global()->negativeMonetarySignPosition()));
  MyMoneyMoney::setPositiveMonetarySignPosition(static_cast<MyMoneyMoney::signPosition>(KLocale::global()->positiveMonetarySignPosition()));
  MyMoneyMoney::setNegativePrefixCurrencySymbol(KLocale::global()->negativePrefixCurrencySymbol());
  MyMoneyMoney::setPositivePrefixCurrencySymbol(KLocale::global()->positivePrefixCurrencySymbol());

  QString language = parser.value(langOption);
  if (!language.isEmpty()) {
    if (!KLocale::global()->setLanguage(QStringList() << language)) {
      qWarning("Unable to select language '%s'. This has one of two reasons:\n\ta) the standard KDE message catalog is not installed\n\tb) the KMyMoney message catalog is not installed", qPrintable(language));
    }
  }

#ifdef KMM_DEBUG
  if (parser.isSet(traceOption))
    MyMoneyTracer::on();
  timersOn = parser.isSet(timersOption);
#endif

  kmymoney = 0;
  kmymoney = new KMyMoneyApp();

#ifdef KMM_DEBUG
  if (parser.isSet(dumpActionsOption)) {
    kmymoney->dumpActions();

    // Before we delete the application, we make sure that we destroy all
    // widgets by running the event loop for some time to catch all those
    // widgets that are requested to be destroyed using the deleteLater() method.
    //QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 10);

    delete kmymoney;
    delete splash;
    exit(0);
  }
#endif

  const QStringList urls = parser.positionalArguments();
  int rc = 0;
  if (parser.isSet(noCatchOption)) {
    qDebug("Running w/o global try/catch block");
    rc = runKMyMoney(&app, splash, urls.isEmpty() ? QUrl() : urls.front(), parser.isSet(noFileOption));
  } else {
    try {
      rc = runKMyMoney(&app, splash, urls.isEmpty() ? QUrl() : urls.front(), parser.isSet(noFileOption));
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedError(0, i18n("Uncaught error. Please report the details to the developers"),
                                 i18n("%1 in file %2 line %3", e.what(), e.file(), e.line()));
      throw e;
    }
  }

  return rc;
}

int runKMyMoney(QApplication *a, KStartupLogo *splash, const QUrl & file, bool noFile)
{
  int rc = 0;
  do {
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kmymoney")) {
      const QList<QString> instances = kmymoney->instanceList();
      if (instances.count() > 0) {

        // If the user launches a second copy of the app and includes a file to
        // open, they are probably attempting a "WebConnect" session.  In this case,
        // we'll check to make sure it's an importable file that's passed in, and if so, we'll
        // notify the primary instance of the file and kill ourselves.

        if (file.isValid()) {
          if (kmymoney->isImportableFile(file.path())) {
            // if there are multiple instances, we'll send this to the first one
            QString primary = instances[0];

            // send a message to the primary client to import this file
            QDBusInterface remoteApp(primary, "/KMymoney", "org.kde.kmymoney");
            remoteApp.call("webConnect", file.path(), KStartupInfo::startupId());

            // Before we delete the application, we make sure that we destroy all
            // widgets by running the event loop for some time to catch all those
            // widgets that are requested to be destroyed using the deleteLater() method.
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 10);

            delete kmymoney;
            delete splash;
            break;
          }
        }

        if (KMessageBox::questionYesNo(0, i18n("Another instance of KMyMoney is already running. Do you want to quit?")) == KMessageBox::Yes) {
          rc = 1;
          delete kmymoney;
          delete splash;
          break;
        }
      }
    } else {
      qDebug("D-Bus registration failed. Some functions are not available.");
    }
    kmymoney->show();
    kmymoney->centralWidget()->setEnabled(false);

    delete splash;

    // force complete paint of widgets
    qApp->processEvents();

    QString importfile;
    QUrl url;
    // make sure, we take the file provided on the command
    // line before we go and open the last one used
    if (file.isValid()) {
      // Check to see if this is an importable file, as opposed to a loadable
      // file.  If it is importable, what we really want to do is load the
      // last used file anyway and then immediately import this file.  This
      // implements a "web connect" session where there is not already an
      // instance of the program running.

      if (kmymoney->isImportableFile(file.path())) {
        importfile = file.path();
        url = QUrl::fromUserInput(kmymoney->readLastUsedFile());
      }

    } else {
      url = QUrl::fromUserInput(kmymoney->readLastUsedFile());
    }

    KTipDialog::showTip(kmymoney, "", false);
    if (url.isValid() && !noFile) {
      kmymoney->slotFileOpenRecent(url);
    } else if (KMyMoneyGlobalSettings::firstTimeRun()) {
      kmymoney->slotFileNew();
    }
    KMyMoneyGlobalSettings::setFirstTimeRun(false);

    if (!importfile.isEmpty())
      kmymoney->webConnect(importfile, KStartupInfo::startupId());

    if (kmymoney != 0) {
      kmymoney->updateCaption();
      kmymoney->centralWidget()->setEnabled(true);
      rc = a->exec();
    }
  } while (0);
  return rc;
}


void timestamp(char *txt)
{
  if (timersOn) {
    qDebug("Time(%s): %d", txt, timer.elapsed());
  }
}
