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
#include <QStringList>
#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#ifdef KMM_DBUS
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#endif
// ----------------------------------------------------------------------------
// KDE Includes

#include <KTipDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <Kdelibs4ConfigMigrator>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyfile.h"
#include "mymoneyexception.h"
#include "kmymoney.h"
#include "kcreditswindow.h"
#include "kmymoneyutils.h"
#include "kmymoneysettings.h"
#include "misc/webconnect.h"
#include "platformtools.h"

#ifdef KMM_DEBUG
#include "mymoneyutils.h"
#include "mymoneytracer.h"
#endif

bool timersOn = false;

KMyMoneyApp* kmymoney;

static int runKMyMoney(QApplication &a, const QUrl &file, bool noFile);
static void migrateConfigFiles();

int main(int argc, char *argv[])
{
#ifdef KMM_DEBUG
  // make sure the DOM attributes are stored in the same order
  // each time a file is saved. Don't use QHash for security
  // relevant things from here on. See
  // https://doc.qt.io/qt-5/qhash.html#algorithmic-complexity-attacks
  // for details.
  qSetGlobalQHashSeed(0);
#endif

  /**
   * Create application first
   */
  QApplication app(argc, argv);
  KLocalizedString::setApplicationDomain("kmymoney");

  migrateConfigFiles();

  /**
   * construct and register about data
   */
  KAboutData aboutData(QStringLiteral("kmymoney"), i18n("KMyMoney"), QStringLiteral(VERSION));
  aboutData.setOrganizationDomain("kde.org");
  KAboutData::setApplicationData(aboutData);

  QStringList fileUrls;
  bool isNoCatchOption = false;
  bool isNoFileOption = false;

#ifdef KMM_DEBUG
  bool isDumpActionsOption = false;
#endif

  if (argc != 0) {
    /**
     * Create command line parser and feed it with known options
     */
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    // language
    //    const QCommandLineOption langOption(QStringLiteral("lang"), i18n("language to be used"));
    //    parser.addOption(langOption);

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
    parser.parse(QApplication::arguments());

    bool ishelpSet = parser.isSet(QStringLiteral("help"));
    if (ishelpSet || parser.isSet(QStringLiteral("author")) || parser.isSet(QStringLiteral("license"))) {
      aboutData = initializeCreditsData();
      if (ishelpSet)
        parser.showHelp();
    }

    if (parser.isSet(QStringLiteral("version")))
      parser.showVersion();

    /**
     * handle standard options
     */
    aboutData.processCommandLine(&parser);

#ifdef KMM_DEBUG
    if (parser.isSet(traceOption))
      MyMoneyTracer::on();
    timersOn = parser.isSet(timersOption);
    isDumpActionsOption = parser.isSet(dumpActionsOption);
#endif

    isNoCatchOption = parser.isSet(noCatchOption);
    isNoFileOption = parser.isSet(noFileOption);
    fileUrls = parser.positionalArguments();
  }

  // create the singletons before we start memory checking
  // to avoid false error reports
  auto file = MyMoneyFile::instance();
  Q_UNUSED(file)

  KMyMoneyUtils::checkConstants();

  // setup the MyMoneyMoney locale settings according to the KDE settings
  MyMoneyMoney::setThousandSeparator(QLocale().groupSeparator());
  MyMoneyMoney::setDecimalSeparator(QLocale().decimalPoint());

  // setup format of negative values
  MyMoneyMoney::setNegativeSpaceSeparatesSymbol(false);
  MyMoneyMoney::setNegativePrefixCurrencySymbol(false);
  MyMoneyMoney::setNegativeMonetarySignPosition(static_cast<eMyMoney::Money::signPosition>(platformTools::currencySignPosition(true)));
  switch(platformTools::currencySymbolPosition(true)) {
    case platformTools::AfterQuantityMoneyWithSpace:
      MyMoneyMoney::setNegativeSpaceSeparatesSymbol(true);
      // intentional fall through
    case platformTools::AfterQuantityMoney:
      break;

    case platformTools::BeforeQuantityMoneyWithSpace:
      MyMoneyMoney::setNegativeSpaceSeparatesSymbol(true);
      // intentional fall through
    case platformTools::BeforeQuantityMoney:
      MyMoneyMoney::setNegativePrefixCurrencySymbol(true);
      break;
  }
  // setup format of positive values
  MyMoneyMoney::setPositiveSpaceSeparatesSymbol(false);
  MyMoneyMoney::setPositivePrefixCurrencySymbol(false);
  MyMoneyMoney::setPositiveMonetarySignPosition(static_cast<eMyMoney::Money::signPosition>(platformTools::currencySignPosition(false)));
  switch(platformTools::currencySymbolPosition(false)) {
    case platformTools::AfterQuantityMoneyWithSpace:
      MyMoneyMoney::setPositiveSpaceSeparatesSymbol(true);
      // intentional fall through
    case platformTools::AfterQuantityMoney:
      break;
    case platformTools::BeforeQuantityMoneyWithSpace:
      MyMoneyMoney::setPositiveSpaceSeparatesSymbol(true);
      // intentional fall through
    case platformTools::BeforeQuantityMoney:
      MyMoneyMoney::setPositivePrefixCurrencySymbol(true);
      break;
  }

//  QString language = parser.value(langOption);
//  if (!language.isEmpty()) {
    //if (!KLocale::global()->setLanguage(QStringList() << language)) {
    //  qWarning("Unable to select language '%s'. This has one of two reasons:\n\ta) the standard KDE message catalog is not installed\n\tb) the KMyMoney message catalog is not installed", qPrintable(language));
    //}
//  }

  kmymoney = new KMyMoneyApp();

#ifdef KMM_DEBUG
  if (isDumpActionsOption) {
    kmymoney->dumpActions();

    // Before we delete the application, we make sure that we destroy all
    // widgets by running the event loop for some time to catch all those
    // widgets that are requested to be destroyed using the deleteLater() method.
    //QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 10);

    delete kmymoney;
    exit(0);
  }
#endif

  QString fname;
  // in case a filename is provided we need to check if it is a local
  // file. In case the name does not start with "file://" or "./" or "/"
  // we need to prepend "./" to fake a relative filename. Otherwise, QUrl prepends
  // "http://" and uses the full path which will not work.
  // On MS-Windows we also need to check if the filename starts with a
  // drive letter or the backslash variants.
  //
  // The handling might be different on other OSes
  if (!fileUrls.isEmpty()) {
    fname = fileUrls.front();
    QFileInfo fi(fname);
    auto needLeadIn = fi.isFile();
#ifdef Q_OS_WIN
    QRegularExpression exp("^[a-z]:", QRegularExpression::CaseInsensitiveOption);
    needLeadIn &= !exp.match(fname).hasMatch()
                  && !fname.startsWith(QLatin1String(".\\"))
                  && !fname.startsWith(QLatin1String("\\"));
#endif
    needLeadIn &= !fname.startsWith(QLatin1String("file://"))
                  && !fname.startsWith(QLatin1String("./"))
                  && !fname.startsWith(QLatin1String("/"));
    if (needLeadIn) {
        fname.prepend(QLatin1String("./"));
    }
  }

  const QUrl url = QUrl::fromUserInput(fname, QLatin1String("."), QUrl::AssumeLocalFile);
  int rc = 0;
  if (isNoCatchOption) {
    qDebug("Running w/o global try/catch block");
    rc = runKMyMoney(app, url, isNoFileOption);
  } else {
    try {
      rc = runKMyMoney(app, url, isNoFileOption);
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedError(0, i18n("Uncaught error. Please report the details to the developers"), QString::fromLatin1(e.what()));
      throw;
    }
  }

  return rc;
}

int runKMyMoney(QApplication &a, const QUrl &file, bool noFile)
{

#ifdef Q_OS_MAC
  kmymoney->setUnifiedTitleAndToolBarOnMac(true);
  kmymoney->setAttribute(Qt::WA_TranslucentBackground, true);
  kmymoney->setWindowFlag(Qt::MacWindowToolBarButtonHint);
#endif

  bool instantQuit = false;

  /**
   * enable high dpi icons
   */
  a.setAttribute(Qt::AA_UseHighDpiPixmaps);

  if (kmymoney->webConnect()->isClient()) {
    // If the user launches a second copy of the app and includes a file to
    // open, they are probably attempting a "WebConnect" session.  In this case,
    // we'll check to make sure it's an importable file that's passed in, and if so, we'll
    // notify the primary instance of the file and kill ourselves.

    if (file.isValid()) {
      if (kmymoney->isImportableFile(file)) {
        instantQuit = true;
        kmymoney->webConnect()->loadFile(file);
      }
    }
  }

  // show the main window and force it to repaint
  kmymoney->show();
  qApp->processEvents();

  // now disable input and repaint again
  kmymoney->centralWidget()->setEnabled(false);
  qApp->processEvents();

  if (!instantQuit) {
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

        if (kmymoney->isImportableFile(file)) {
          importfile = file.path();
          url = QUrl::fromUserInput(kmymoney->readLastUsedFile());
        } else {
          url = file;
        }

    } else {
      url = QUrl::fromUserInput(kmymoney->readLastUsedFile());
    }

    if (url.isValid() && !noFile) {
      if (importfile.isEmpty()) {
        KTipDialog::showTip(kmymoney, QString(), false);
      }
      kmymoney->slotFileOpenRecent(url);

    } else if (KMyMoneySettings::firstTimeRun()) {
      kmymoney->slotFileNew();
    }

    KMyMoneySettings::setFirstTimeRun(false);

    if (!importfile.isEmpty()) {
      kmymoney->webConnect(importfile, QByteArray());
    }

  } else {
    // the instantQuit flag is set, so we force the app to quit right away
    kmymoney->slotFileQuit();
  }

  kmymoney->centralWidget()->setEnabled(true);

  const int rc = a.exec();      //krazy:exclude=crashy
  return rc;
}

static void migrateConfigFiles()
{
  const QString sMainConfigName(QStringLiteral("kmymoneyrc"));
  const QString sMainConfigSubdirectory(QStringLiteral("kmymoney/")); // all KMM config files should be in ~/.config/kmymoney/
  const QString sMainConfigPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
                                  QLatin1Char('/') +
                                  sMainConfigSubdirectory;

  if (!QFile::exists(sMainConfigPath + sMainConfigName)) { // if main config file doesn't exist, then it's first run

    // it could be migration from KDE4 to KF5 so prepare list of configuration files to migrate
    QStringList sConfigNames
    {
      sMainConfigName,
      QStringLiteral("csvimporterrc"),
      QStringLiteral("printcheckpluginrc"),
      QStringLiteral("icalendarexportpluginrc"),
      QStringLiteral("kbankingrc"),
    };

    // Copy KDE 4 config files to the KF5 location
    Kdelibs4ConfigMigrator migrator(QStringLiteral("kmymoney"));
    migrator.setConfigFiles(sConfigNames);
    migrator.setUiFiles(QStringList{QStringLiteral("kmymoneyui.rc")});
    migrator.migrate();

    QFileInfo fileInfo(sMainConfigPath + sMainConfigName);
    QDir().mkpath(fileInfo.absolutePath());
    const QString sOldMainConfigPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
                                       QLatin1Char('/');

    // some files have changed their names during switch to KF5, so prepare map for name replacements
    QMap<QString, QString> configNamesChange {
      {QStringLiteral("printcheckpluginrc"), QStringLiteral("checkprintingrc")},
      {QStringLiteral("icalendarexportpluginrc"), QStringLiteral("icalendarexporterrc")}
    };

    for (const auto& sConfigName : qAsConst(sConfigNames)) {
      const auto sOldConfigFilename = sOldMainConfigPath + sConfigName;
      const auto sNewConfigFilename = sMainConfigPath + configNamesChange.value(sConfigName, sConfigName);
      if (QFile::exists(sOldConfigFilename)) {
        if (QFile::copy(sOldConfigFilename, sNewConfigFilename))
          QFile::remove(sOldConfigFilename);
      }
    }
  }
  KConfig::setMainConfigName(sMainConfigSubdirectory + sMainConfigName); // otherwise it would be ~/.config/kmymoneyrc and not ~/.config/kmymoney/kmymoneyrc
}
