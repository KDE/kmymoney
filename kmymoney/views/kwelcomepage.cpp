/***************************************************************************
                          kwelcomepage.cpp  -  description
                             -------------------
    begin                : Sat Sep 5 2009
    copyright            : (C) 2009 by Alvaro Soliverez <asoliverez@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// Project Includes
#include "kwelcomepage.h"
#include "kmymoneyutils.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QString>
#include <QStringList>
#include <QUrl>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <k4aboutdata.h>
#include <kglobal.h>

KWelcomePage::KWelcomePage()
{
}

KWelcomePage::~KWelcomePage()
{
}

const QString KWelcomePage::welcomePage(void)
{
  QString header;
  QString footer;
  QString body;

  //header
  header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">");
  header = QString("<html>");
  header += QString("<head>");
  header += QString("<title>" + i18n("Home Page") + "</title>");
  header += QString("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">");

  const QString welcomeFilename = KGlobal::dirs()->findResource("appdata", "html/welcome.css");
  header += QString("<link href=\"%1\" rel=\"stylesheet\" type=\"text/css\">").arg(QUrl::fromLocalFile(welcomeFilename).url());
  header += QString("</head>");

  //body of the page
  body = QString("<body>");
  //"background_image", if enabled, displays an image in the background of this page.
  //If you wish to use a background, un-comment the following line
  const QString backgroundFilename = KGlobal::dirs()->findResource("appdata", "html/images/background.png");
  body += QString("<img id=\"background_image\" src=\"%1\" height=\"100%\">").arg(QUrl::fromLocalFile(backgroundFilename).url());
  const QString logoFilename = KGlobal::dirs()->findResource("appdata", "html/images/trans_logo.png");
  body += QString("<img id=\"KMyMoneyLogo\" src=\"%1\">").arg(QUrl::fromLocalFile(logoFilename).url());
  body += QString("<h3 id=\"title\">" + i18n("Welcome to KMyMoney") + "</h3>");
  body += QString("<h4 id=\"subtitle\">" + i18n("The free, easy to use, personal finance manager for KDE") + "</h4>");
  const QString backArrowFilename = KGlobal::dirs()->findResource("appdata", "html/images/backarrow.png");
  body += QString("<div id=\"returnLink\"><a href=\"/home\"><img src=\"%1\">").arg(QUrl::fromLocalFile(backArrowFilename).url());
  body += QString(i18n("Go to My Financial Summary"));
  body += QString("</a></div>");
  body += QString("<div id=\"topleft\">");

  //topright
  body += QString("<div id=\"topright\"></div>");
  body += QString("<div id=\"rightborder\">");
  body += QString("<table style=\"width: 100%;\">");
  body += QString("<tbody>");
  body += QString("<tr>");
  body += QString("<td></td>");
  body += QString("<td style=\"height: 150px;\">");
  body += QString("<div id=\"welcomeMenu\">");
  body += QString("<h4>" + i18n("Start with one of the following activities...") + "</h4>");

  //Welcome menu
  body += QString("<ul>");
  const QString newFilename = KGlobal::dirs()->findResource("appdata", "html/images/filenew.png");
  body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(newFilename).url());
  body += QString("<a href=\"/action?id=file_new\">" + i18n("Get started and setup accounts") + "</a></li>");
  const QString dataFilename = KGlobal::dirs()->findResource("appdata", "html/images/kmymoneydata.png");
  body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(dataFilename).url());
  body += QString("<a href=\"/action?id=file_open\">" + i18n("Open an existing data file") + "</a></li>");
  const QString manualFilename = KGlobal::dirs()->findResource("appdata", "html/images/manual.png");
  body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(manualFilename).url());
  body += QString("<a href=\"/action?id=help_contents\">" + i18n("Learn how to use KMyMoney") + "</a></li>");
  const QString konquerorFilename = KGlobal::dirs()->findResource("appdata", "html/images/konqueror.png");
  body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(konquerorFilename).url());
  body += QString("<a href=\"http://kmymoney2.sf.net\">" + i18n("Visit our website") + "</a></li>");
  const QString aboutFilename = KGlobal::dirs()->findResource("appdata", "html/images/about_kde.png");
  body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(aboutFilename).url());
  body += QString("<a href=\"http://forum.kde.org/viewforum.php?f=69\">" + i18n("Get help from our community") + "</a></li>");
  const QString messageFilename = KGlobal::dirs()->findResource("appdata", "html/images/messagebox_info.png");
  body += QString("<li><img src=\"%1\">").arg(QUrl::fromLocalFile(messageFilename).url());
  body += QString("<a href=\"/welcome?mode=whatsnew\">" + i18n("See what's new in this version") + "</a></li>");
  body += QString("</ul>");

  body += QString("</div>");
  body += QString("</td>");
  body += QString("<td></td>");
  body += QString("</tr>");
  body += QString("</tbody>");
  body += QString("</table>");
  //right border
  body += QString("</div>");
  body += QString("<div id=\"bottomleft\">");

  //bottom right
  body += QString("<div id=\"bottomright\"></div>");
  //bottom left
  body += QString("</div>");
  //top left
  body += QString("</div>");
  body += QString("</body>");

  //footer
  footer = "</html>";

  return header + body + footer;
}

const QString KWelcomePage::whatsNewPage(void)
{
  QString header;
  QString footer;
  QString body;

  header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">");
  header += QString("<html>");
  header += QString("<head>");
  header += QString("<title>" + i18n("What's new in this version") + "</title>");

  const QString welcomeFilename = KGlobal::dirs()->findResource("appdata", "html/welcome.css");
  header += QString("<link href=\"%1\" rel=\"stylesheet\" type=\"text/css\">").arg(QUrl::fromLocalFile(welcomeFilename).url());
  header += QString("</head>");

  //body of the page
  body = QString("<body>");
  //"background_image", if enabled, displays an image in the background of this page.
  //If you wish to use a background, un-comment the following line
  const QString backgroundFilename = KGlobal::dirs()->findResource("appdata", "html/images/background.png");
  body += QString("<img id=\"background_image\" src=\"%1\" height=\"100%\">").arg(QUrl::fromLocalFile(backgroundFilename).url());
  const QString logoFilename = KGlobal::dirs()->findResource("appdata", "html/images/trans_logo.png");
  body += QString("<img id=\"KMyMoneyLogo\" src=\"%1\">").arg(QUrl::fromLocalFile(logoFilename).url());
  body += QString("<h3 id=\"title\">" + i18n("What's new in KMyMoney %1", KCmdLineArgs::aboutData()->version()) + "</h3>");
  const QString backArrowFilename = KGlobal::dirs()->findResource("appdata", "html/images/backarrow.png");
  body += QString("<div id=\"returnLink\"><img src=\"%1\">").arg(QUrl::fromLocalFile(backArrowFilename).url());
  body += QString("<a href=\"/welcome\">" + i18n("Return to the Welcome page") + "</a></div>");

  body += QString("<div id=\"topleft\">");

  body += QString("<div id=\"topright\"></div>");
  body += QString("<div id=\"rightborder\"><table><tr><td>");
//This is where the content should be put to show up inside the decorative frame
//Begin content

  body += QString("<p>" + i18n("We are pleased to announce a major step forward for what has been described as \"the BEST personal finance manager for FREE users\".") + "</p>");
  body += QString("<h4>" + i18n("What's new in this version:") + "</h4>");

  body += QString("<ul>");

  QStringList featuresList = KWelcomePage::featuresList();

  QStringList::ConstIterator feature_it;
  for (feature_it = featuresList.constBegin(); feature_it != featuresList.constEnd(); ++feature_it) {
    body += QString("<li>");
    body += *feature_it;
    body += QString("</li>");
  }
  body += QString("</ul>");

  body += QString("<p>" + i18n("Let us know what you think. We hope that you enjoy using the application.") + "</p>");
  body += QString("<p>" + i18n("Please let us know about any abnormal behavior in the program by selecting <a href=\"/action?id=help_report_bug\">\"Report bug...\"</a> from the help menu or by sending an e-mail to the developers mailing list."));
  body += QString("<font color=\"blue\"><a href=\"mailto:kmymoney-devel@kde.org\">kmymoney-devel@kde.org</a></font></p>");
  body += QString("<p><div align=\"right\">");
  body += QString(i18n("The KMyMoney Development Team"));
  body += QString("</div></p>");

  //End content
  body += QString("</td></tr></table>");
  body += QString("</div>");
  body += QString("<div id=\"bottomleft\"><div id=\"bottomright\"></div>");
  body += QString("</div>");
  body += QString("</div>");
  body += QString("<div style=\"margin-bottom: 65px\"></div>");
  body += QString("</body>");

  //footer
  footer += QString("</html>");

  return header + body + footer;
}

const QStringList KWelcomePage::featuresList(void)
{
  QStringList featuresList;

  featuresList.append(i18n("Can import csv files with the new CSV Import plugin"));
  featuresList.append(i18n("Documentation and translations have been improved"));
  featuresList.append(i18n("Now works with latest version of AqBanking"));
  featuresList.append(i18n("Better performance of the user interface"));
  featuresList.append(i18n("Improved usability of the online banking features"));
  featuresList.append(i18n("Allow to base the payee name from either the PAYEEID, NAME or MEMO field in an OFX transaction"));
  featuresList.append(i18n("Runs on all operating systems supported by KDE"));


  return featuresList;
}

