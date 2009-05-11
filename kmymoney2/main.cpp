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


# include <config-kmymoney.h>

#include <stdio.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <qeventloop.h>
//Added by qt3to4:
#include <Q3CString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <ktip.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyfile.h"
#include "kmymoney2.h"
#include "kstartuplogo.h"
#include "kmymoneyutils.h"
#include "kmymoneyglobalsettings.h"

static const char *description =
  I18N_NOOP("\nKMyMoney, the Personal Finance Manager for KDE.\n\nPlease consider contributing to this project with code and/or suggestions.");

static KCmdLineOptions options[] =
{
  { "lang <lang-code>", I18N_NOOP("language to be used"), 0 },
  { "n", I18N_NOOP("don't open last used file"), 0},
  { "timers", I18N_NOOP("enable performance timers"), 0},

#if KMM_DEBUG
  // The following options are only available when compiled in debug mode
  { "trace", I18N_NOOP("turn on program traces"), 0},
  { "dump-actions", I18N_NOOP("dump the names of all defined KAction objects to stdout and quit"), 0},
#endif

  // INSERT YOUR COMMANDLINE OPTIONS HERE
  { "+[File]", I18N_NOOP("file to open"), 0 },
  KCmdLineLastOption
};

QTime timer;
bool timersOn = false;

KMyMoney2App* kmymoney2;

static KCmdLineArgs* args = 0;

static void _cleanup(void)
{
#ifdef _CHECK_MEMORY
  chkmem.CheckMemoryLeak( false );
  _CheckMemory_End();
#endif
}

int main(int argc, char *argv[])
{
  timer.start();

  QString feature;

#ifdef _CHECK_MEMORY
  feature += "\t- " I18N_NOOP("Memory leakage detection") "\n";
#endif

  if(!feature.isEmpty())
    feature = I18N_NOOP("Compiled with the following settings:\n") + feature;

  KAboutData aboutData( "kmymoney2", I18N_NOOP("KMyMoney"),
    VERSION, description, KAboutData::License_GPL,
    "(c) 2000-2009 The KMyMoney development team", feature,
    "http://kmymoney2.sourceforge.net/",
    "kmymoney2-developer@lists.sourceforge.net");

  aboutData.addAuthor("Michael Edwardes.", I18N_NOOP("Initial idea, much initial source code, Project admin"), "mte@users.sourceforge.net");
  aboutData.addAuthor("Thomas Baumgart", I18N_NOOP("Core engine, Release Manager, Project admin"), "ipwizard@users.sourceforge.net");
  aboutData.addAuthor("Ace Jones", I18N_NOOP("Reporting logic, OFX Import"), "acejones@users.sourceforge.net");
  aboutData.addAuthor("Tony Bloomfield", I18N_NOOP("Database backend, maintainer stable branch"), "tonybloom@users.sourceforge.net");
  aboutData.addAuthor("Alvaro Soliverez", I18N_NOOP("Forecast, Reports"), "asoliverez@gmail.com");
  aboutData.addAuthor("Felix Rodriguez", I18N_NOOP("Project Admin"), "frodriguez@users.sourceforge.net");
  aboutData.addAuthor("John C", I18N_NOOP("Developer"), "tacoturtle@users.sourceforge.net");
  aboutData.addAuthor("Fernando Vilas", I18N_NOOP("Database backend"), "fvilas@iname.com");

  aboutData.addCredit("Kevin Tambascio", I18N_NOOP("Initial investment support"), "ktambascio@users.sourceforge.net");
  aboutData.addCredit("Javier Campos Morales", I18N_NOOP("Developer & Artist"), "javi_c@users.sourceforge.net");
  aboutData.addCredit("Robert Wadley", I18N_NOOP("Icons & splash screen"), "rob@robntina.fastmail.us");
  aboutData.addCredit("Laurent Montel", I18N_NOOP("Patches"), "montel@kde.org");
  aboutData.addCredit("Wolfgang Rohdewald", I18N_NOOP("Patches"), "woro@users.sourceforge.net");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  // create the singletons before we start memory checking
  // to avoid false error reports
  MyMoneyFile::instance();

#ifdef _CHECK_MEMORY
  _CheckMemory_Init(0);
#endif
  atexit(_cleanup);

  KMyMoneyUtils::checkConstants();

  KApplication* a = new KApplication();

  if(KGlobal::locale()->monetaryDecimalSymbol().isEmpty()) {
    KMessageBox::error(0, i18n("The monetary decimal symbol is not correctly set in the KDE Control Center's Country/Region & Language settings. Please set it to a reasonable value and start KMyMoney again."), i18n("Invalid settings"));
    delete a;
    exit(1);
  }

  // show startup logo
  KStartupLogo* splash = new KStartupLogo();
  a->processEvents();

  args = KCmdLineArgs::parsedArgs();

  // setup the MyMoneyMoney locale settings according to the KDE settings
  MyMoneyMoney::setThousandSeparator(*(KGlobal::locale()->monetaryThousandsSeparator().toLatin1()));
  MyMoneyMoney::setDecimalSeparator(*(KGlobal::locale()->monetaryDecimalSymbol().toLatin1()));
  MyMoneyMoney::setNegativeMonetarySignPosition(static_cast<MyMoneyMoney::signPosition>(KGlobal::locale()->negativeMonetarySignPosition()));
  MyMoneyMoney::setPositiveMonetarySignPosition(static_cast<MyMoneyMoney::signPosition>(KGlobal::locale()->positiveMonetarySignPosition()));
  MyMoneyMoney::setNegativePrefixCurrencySymbol(KGlobal::locale()->negativePrefixCurrencySymbol());
  MyMoneyMoney::setPositivePrefixCurrencySymbol(KGlobal::locale()->positivePrefixCurrencySymbol());

  Q3CString language = args->getOption("lang");
  if(!language.isEmpty()) {
    if(!KGlobal::locale()->setLanguage(language)) {
      qWarning("Unable to select language '%s'. This has one of two reasons:\n\ta) the standard KDE message catalogue is not installed\n\tb) the KMyMoney message catalogue is not installed", language.data());
    }
  }

#if KMM_DEBUG
  if(args->isSet("trace"))
    MyMoneyTracer::on();
  timersOn = args->isSet("timers");
#endif

  kmymoney2 = 0;
  kmymoney2 = new KMyMoney2App();
  a->setMainWidget( kmymoney2 );

#if KMM_DEBUG
  if(args->isSet("dump-actions")) {
    kmymoney2->dumpActions();

    // Before we delete the application, we make sure that we destroy all
    // widgets by running the event loop for some time to catch all those
    // widgets that are requested to be destroyed using the deleteLater() method.
    QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);

    delete kmymoney2;
    delete splash;
    delete a;
    exit(0);
  }
#endif

  int rc = 0;
  try {
    do {
      // connect to DCOP server
      DCOPClient* client = a->dcopClient();
      if(client->registerAs("kmymoney", true) != false) {
        const QCStringList instances = kmymoney2->instanceList();
        if(instances.count() > 0) {
    
          // If the user launches a second copy of the app and includes a file to
          // open, they are probably attempting a "WebConnect" session.  In this case,
          // we'll check to make sure it's an importable file that's passed in, and if so, we'll
          // notify the primary instance of the file and kill ourselves.
    
          if(args->count() > 0) {
            KUrl url = args->url(0);
            if ( kmymoney2->isImportableFile( url.path() ) )
            {
              // if there are multiple instances, we'll send this to the first one
              Q3CString primary = instances[0];
    
              // send a message to the primary client to import this file
              QByteArray data;
              QDataStream arg(data, QIODevice::WriteOnly);
              arg << url.path();
              arg << kapp->startupId();
              if (!client->send(primary, "kmymoney2app", "webConnect(QString,QCString)",data))
                qDebug("Unable to launch WebConnect via DCOP.");
    
              // Before we delete the application, we make sure that we destroy all
              // widgets by running the event loop for some time to catch all those
              // widgets that are requested to be destroyed using the deleteLater() method.
              QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);
    
              delete kmymoney2;
              delete splash;
              break;
            }
          }
    
          if(KMessageBox::questionYesNo(0, i18n("Another instance of KMyMoney is already running. Do you want to quit?")) == KMessageBox::Yes) {
            rc = 1;
            delete kmymoney2;
            delete splash;
            break;
          }
        }
      } else {
        qDebug("DCOP registration failed. Some functions are not available.");
      }
    
      kmymoney2->show();
      kmymoney2->setEnabled(false);
    
      delete splash;
    
      // force complete paint of widgets
      qApp->processEvents();
    
      QString importfile;
      KUrl url;
      // make sure, we take the file provided on the command
      // line before we go and open the last one used
      if(args->count() > 0) {
        url = args->url(0);
    
        // Check to see if this is an importable file, as opposed to a loadable
        // file.  If it is importable, what we really want to do is load the
        // last used file anyway and then immediately import this file.  This
        // implements a "web connect" session where there is not already an
        // instance of the program running.
    
        if ( kmymoney2->isImportableFile( url.path() ) )
        {
          importfile = url.path();
          url = kmymoney2->readLastUsedFile();
        }
    
      } else {
        url = kmymoney2->readLastUsedFile();
      }
    
      KTipDialog::showTip(kmymoney2, "", false);
      if(url.isValid() && !args->isSet("n")) {
        kmymoney2->slotFileOpenRecent(url);
      } else if(KMyMoneyGlobalSettings::firstTimeRun()) {
        kmymoney2->slotFileNew();
      }
      KMyMoneyGlobalSettings::setFirstTimeRun(false);
    
      if ( ! importfile.isEmpty() )
        kmymoney2->webConnect( importfile, kapp->startupId() );
    
      if(kmymoney2 != 0) {
        kmymoney2->updateCaption();
        args->clear();
        kmymoney2->setEnabled(true);
        rc = a->exec();
      }
    } while(0);
  } catch(MyMoneyException *e) {
    KMessageBox::detailedError(0, i18n("Uncaught error. Please report the details to the developers"),
                               QString("%1 in file %2 line %3").arg(e->what()).arg(e->file()).arg(e->line()));
    throw e;
  }

  delete a;

  return rc;
}

void timestamp(char *txt)
{
  if(timersOn)
  {
    qDebug("Time(%s): %d", txt, timer.elapsed());
  }
}

void timetrace(const char *txt)
{
  if(timersOn)
  {
    qDebug("Timer(%s): %d elapsed", txt, timer.elapsed());
    timer.restart();
  }
}
// vim:cin:si:ai:et:ts=2:sw=2:
