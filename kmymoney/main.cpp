/*
    SPDX-FileCopyrightText: 2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <config-kmymoney.h>
#include <config-kmymoney-version.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QCommandLineParser>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStandardPaths>
#include <QStringList>
#include <QWidget>
#ifdef KMM_DBUS
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#endif
// ----------------------------------------------------------------------------
// KDE Includes

#include <KCrash>

#define HAVE_ICON_THEME __has_include(<KIconTheme>)
#if HAVE_ICON_THEME
#include <KIconTheme>
#endif

#include <KLocalizedString>
#include <KMessageBox>

#define HAVE_STYLE_MANAGER __has_include(<KStyleManager>)
#if HAVE_STYLE_MANAGER
#include <KStyleManager>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include <alkimia/alkenvironment.h>

#include "amountedit.h"
#include "kcreditswindow.h"
#include "kmymoney.h"
#include "kmymoneysettings.h"
#include "kmymoneyutils.h"
#include "misc/webconnect.h"
#include "mymoney/mymoneyfile.h"
#include "mymoneyexception.h"
#include "platformtools.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef KMM_DEBUG
#include "mymoneyutils.h"
#include "mymoneytracer.h"
#endif

#ifdef IMPORT_SQLCIPHER_PLUGIN
Q_IMPORT_PLUGIN(QSQLCipherDriverPlugin)
#endif

bool timersOn = false;

KMyMoneyApp* kmymoney;

static int runKMyMoney(QApplication &a, const QUrl &file, bool noFile);
static void migrateConfigFiles();

static void emergencySaveFunction(int)
{
    if (kmymoney)
        kmymoney->slotFileClose();
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // enable console logging on Windows
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif

    /**
     * trigger initialisation of proper icon theme
     */
#if HAVE_ICON_THEME
#if KICONTHEMES_VERSION >= QT_VERSION_CHECK(6, 3, 0)
    KIconTheme::initTheme();
#endif
#endif

    /**
     * Create application first
     */
    QApplication app(argc, argv);

#if HAVE_STYLE_MANAGER
    /**
     * trigger initialisation of proper application style
     */
    KStyleManager::initStyle();
#else
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    /**
     * For Windows and macOS: use Breeze if available
     * Of all tested styles that works the best for us
     */
    QApplication::setStyle(QStringLiteral("breeze"));
#endif
#endif

    KLocalizedString::setApplicationDomain("kmymoney");

    AlkEnvironment::checkForAppImageEnvironment(argv[0]);

    KCrash::setEmergencySaveFunction(emergencySaveFunction);

    migrateConfigFiles();

    /**
     * construct and register about data
     */
    KAboutData aboutData(QStringLiteral("kmymoney"), i18n("KMyMoney"), QStringLiteral(VERSION));
    aboutData.setOrganizationDomain("kde.org");
    KAboutData::setApplicationData(aboutData);

    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kmymoney")));

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
        const QCommandLineOption langOption(QStringLiteral("lang"), i18n("language to be used"), "lang");
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
         * do the command line parsing (and handle --help and --version)
         */
        parser.process(QApplication::arguments());

        if (parser.isSet(QStringLiteral("author")) || parser.isSet(QStringLiteral("license"))) {
            aboutData = initializeCreditsData();
        }

        if (parser.isSet(QStringLiteral("lang"))) {
            QString language = parser.value(langOption);
            if (!language.isEmpty()) {
                KLocalizedString::setLanguages(QStringList() << language);
            }
        }

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

    MyMoneyMoney::detectCurrencyFormatting();

    qDebug() << "Long date format" << QLocale().dateFormat(QLocale::LongFormat);
    qDebug() << "Short date format" << QLocale().dateFormat(QLocale::ShortFormat);
    qDebug() << "Narrow date format" << QLocale().dateFormat(QLocale::NarrowFormat);

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
        needLeadIn &= !exp.match(fname).hasMatch() //
                      && !fname.startsWith(QLatin1String(".\\")) //
                      && !fname.startsWith(QLatin1String("\\"));
#endif
        needLeadIn &= !fname.startsWith(QLatin1String("file://")) //
                      && !fname.startsWith(QLatin1String("./")) //
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
            KMessageBox::detailedError(nullptr, i18n("Uncaught error. Please report the details to the developers"), QString::fromLatin1(e.what()));
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

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
            QMetaObject::invokeMethod(kmymoney, "slotFileOpenRecent", Qt::QueuedConnection, Q_ARG(QUrl, url));

        } else if (KMyMoneySettings::firstTimeRun()) {
            QMetaObject::invokeMethod(kmymoney, "slotFileNew", Qt::QueuedConnection);
        }

        KMyMoneySettings::setFirstTimeRun(false);

        if (!importfile.isEmpty()) {
            QMetaObject::invokeMethod(kmymoney, "webConnect", Qt::QueuedConnection, Q_ARG(QByteArray, QByteArray()));
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

        QFileInfo fileInfo(sMainConfigPath + sMainConfigName);
        QDir().mkpath(fileInfo.absolutePath());
        const QString sOldMainConfigPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
                                           QLatin1Char('/');

        // some files have changed their names during switch to KF5, so prepare map for name replacements
        QMap<QString, QString> configNamesChange {
            {QStringLiteral("printcheckpluginrc"), QStringLiteral("checkprintingrc")},
            {QStringLiteral("icalendarexportpluginrc"), QStringLiteral("icalendarexporterrc")},
        };

        for (const auto& sConfigName : qAsConst(sConfigNames)) {
            const QString sOldConfigFilename = sOldMainConfigPath + sConfigName;
            const QString sNewConfigFilename = sMainConfigPath + configNamesChange.value(sConfigName, sConfigName);
            if (QFile::exists(sOldConfigFilename)) {
                if (QFile::copy(sOldConfigFilename, sNewConfigFilename))
                    QFile::remove(sOldConfigFilename);
            }
        }
    }
    KConfig::setMainConfigName(sMainConfigSubdirectory + sMainConfigName); // otherwise it would be ~/.config/kmymoneyrc and not ~/.config/kmymoney/kmymoneyrc
}
