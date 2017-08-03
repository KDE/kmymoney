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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <ktip.h>
#include <kmessagebox.h>
#include <kapplication.h>

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

static KCmdLineArgs* args = 0;

static int runKMyMoney(KApplication *a, KStartupLogo *splash);

int main(int argc, char *argv[])
{
  timer.start();

  QString feature;

  if (!feature.isEmpty())
    feature = I18N_NOOP("Compiled with the following settings:\n") + feature;

  KAboutData aboutData("kmymoney", 0, ki18n("KMyMoney"),
                       VERSION, ki18n("\nKMyMoney, the Personal Finance Manager for KDE.\n\nPlease consider contributing to this project with code and/or suggestions."), KAboutData::License_GPL,
                       ki18n("(c) 2000-2017 The KMyMoney development team"), /*feature*/KLocalizedString(),
                       I18N_NOOP("https://kmymoney.org/")/*,
                                                      "kmymoney-devel@kde.org")*/);

  //Temporarily, the product name to report bugs in BKO is different than the application name
  aboutData.setProductName("kmymoney4");

  aboutData.addAuthor(ki18n("Michael Edwardes."), ki18n("Initial idea, much initial source code, Project admin"), "mte@users.sourceforge.net");
  aboutData.addAuthor(ki18n("Thomas Baumgart"), ki18n("Core engine, Release Manager, Project admin"), "ipwizard@users.sourceforge.net");
  aboutData.addAuthor(ki18n("Alvaro Soliverez"), ki18n("Forecast, Reports"), "asoliverez@gmail.com");
  aboutData.addAuthor(ki18n("Cristian Oneț"), ki18n("Developer"), "onet.cristian@gmail.com");
  aboutData.addAuthor(ki18n("Christian Dávid"), ki18n("Developer"), "christian-david@web.de");
  aboutData.addAuthor(ki18n("Ace Jones"), ki18n("Reporting logic, OFX Import"), "acejones@users.sourceforge.net");
  aboutData.addAuthor(ki18n("Tony Bloomfield"), ki18n("Database backend, maintainer stable branch"), "tonybloom@users.sourceforge.net");
  aboutData.addAuthor(ki18n("Felix Rodriguez"), ki18n("Project Admin"), "frodriguez@users.sourceforge.net");
  aboutData.addAuthor(ki18n("John C"), ki18n("Developer"), "tacoturtle@users.sourceforge.net");
  aboutData.addAuthor(ki18n("Fernando Vilas"), ki18n("Database backend"), "fvilas@iname.com");

  aboutData.addCredit(ki18n("Kevin Tambascio"), ki18n("Initial investment support"), "ktambascio@users.sourceforge.net");
  aboutData.addCredit(ki18n("Javier Campos Morales"), ki18n("Developer & Artist"), "javi_c@users.sourceforge.net");
  aboutData.addCredit(ki18n("Robert Wadley"), ki18n("Icons & splash screen"), "rob@robntina.fastmail.us");
  aboutData.addCredit(ki18n("Laurent Montel"), ki18n("Patches and port to kde4"), "montel@kde.org");
  aboutData.addCredit(ki18n("Wolfgang Rohdewald"), ki18n("Patches"), "woro@users.sourceforge.net");
  aboutData.addCredit(ki18n("Marko Käning"), ki18n("Patches, packaging and KF5-CI for OS X"), "mk-lists@email.de");
  aboutData.addCredit(ki18n("Allan Anderson ✝"), ki18n("CSV import/export"));
  aboutData.addCredit(ki18n("Jack Ostroff"), ki18n("Documentation and user support"), "ostroffjh@users.sourceforge.net");
  aboutData.setOrganizationDomain("kde.org");
  KCmdLineOptions options;
  options.add("lang <lang-code>", ki18n("language to be used"));
  options.add("n", ki18n("do not open last used file"));
  options.add("timers", ki18n("enable performance timers"));
  options.add("nocatch", ki18n("do not globally catch uncaught exceptions"));

#ifdef KMM_DEBUG
  // The following options are only available when compiled in debug mode
  options.add("trace", ki18n("turn on program traces"));
  options.add("dump-actions", ki18n("dump the names of all defined KAction objects to stdout and quit"));
#endif

  // INSERT YOUR COMMANDLINE OPTIONS HERE
  options.add("+[File]", ki18n("file to open"));

  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions(options);   // Add our own options.

  // create the singletons before we start memory checking
  // to avoid false error reports
  MyMoneyFile::instance();

  KMyMoneyUtils::checkConstants();

  KApplication* a = new KApplication();

  if (KGlobal::locale()->monetaryDecimalSymbol().isEmpty()) {
    KMessageBox::error(0, i18n("The monetary decimal symbol is not correctly set in the KDE System Settings module Country/Region & Language. Please set it to a reasonable value and start KMyMoney again."), i18n("Invalid settings"));
    delete a;
    exit(1);
  }

  // show startup logo
  KStartupLogo* splash = new KStartupLogo();
  a->processEvents();

  args = KCmdLineArgs::parsedArgs();

  // setup the MyMoneyMoney locale settings according to the KDE settings
  MyMoneyMoney::setThousandSeparator(KGlobal::locale()->monetaryThousandsSeparator()[0]);
  MyMoneyMoney::setDecimalSeparator(KGlobal::locale()->monetaryDecimalSymbol()[0]);
  MyMoneyMoney::setNegativeMonetarySignPosition(static_cast<MyMoneyMoney::signPosition>(KGlobal::locale()->negativeMonetarySignPosition()));
  MyMoneyMoney::setPositiveMonetarySignPosition(static_cast<MyMoneyMoney::signPosition>(KGlobal::locale()->positiveMonetarySignPosition()));
  MyMoneyMoney::setNegativePrefixCurrencySymbol(KGlobal::locale()->negativePrefixCurrencySymbol());
  MyMoneyMoney::setPositivePrefixCurrencySymbol(KGlobal::locale()->positivePrefixCurrencySymbol());

  QString language = args->getOption("lang");
  if (!language.isEmpty()) {
    if (!KGlobal::locale()->setLanguage(QStringList() << language)) {
      qWarning("Unable to select language '%s'. This has one of two reasons:\n\ta) the standard KDE message catalog is not installed\n\tb) the KMyMoney message catalog is not installed", qPrintable(language));
    }
  }

#ifdef KMM_DEBUG
  if (args->isSet("trace"))
    MyMoneyTracer::on();
  timersOn = args->isSet("timers");
#endif

  kmymoney = 0;
  kmymoney = new KMyMoneyApp();

#ifdef KMM_DEBUG
  if (args->isSet("dump-actions")) {
    kmymoney->dumpActions();

    // Before we delete the application, we make sure that we destroy all
    // widgets by running the event loop for some time to catch all those
    // widgets that are requested to be destroyed using the deleteLater() method.
    //QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 10);

    delete kmymoney;
    delete splash;
    delete a;
    exit(0);
  }
#endif

  int rc = 0;
  if (args->isSet("catch") == false) {
    qDebug("Running w/o global try/catch block");
    rc = runKMyMoney(a, splash);
  } else {
    try {
      rc = runKMyMoney(a, splash);
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedError(0, i18n("Uncaught error. Please report the details to the developers"),
                                 i18n("%1 in file %2 line %3", e.what(), e.file(), e.line()));
      throw e;
    }
  }
  delete a;

  return rc;
}

int runKMyMoney(KApplication *a, KStartupLogo *splash)
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

        if (args->count() > 0) {
          KUrl url = args->url(0);
          if (kmymoney->isImportableFile(url.path())) {
            // if there are multiple instances, we'll send this to the first one
            QString primary = instances[0];

            // send a message to the primary client to import this file
            QDBusInterface remoteApp(primary, "/KMymoney", "org.kde.kmymoney");
            remoteApp.call("webConnect", url.path(), kapp->startupId());

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
    KUrl url;
    // make sure, we take the file provided on the command
    // line before we go and open the last one used
    if (args->count() > 0) {
      url = args->url(0);

      // Check to see if this is an importable file, as opposed to a loadable
      // file.  If it is importable, what we really want to do is load the
      // last used file anyway and then immediately import this file.  This
      // implements a "web connect" session where there is not already an
      // instance of the program running.

      if (kmymoney->isImportableFile(url.path())) {
        importfile = url.path();
        url = kmymoney->readLastUsedFile();
      }

    } else {
      url = kmymoney->readLastUsedFile();
    }

    KTipDialog::showTip(kmymoney, "", false);
    if (url.isValid() && !args->isSet("n")) {
      kmymoney->slotFileOpenRecent(url);
    } else if (KMyMoneyGlobalSettings::firstTimeRun()) {
      kmymoney->slotFileNew();
    }
    KMyMoneyGlobalSettings::setFirstTimeRun(false);

    if (! importfile.isEmpty())
      kmymoney->webConnect(importfile, kapp->startupId());

    if (kmymoney != 0) {
      kmymoney->updateCaption();
      args->clear();
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
