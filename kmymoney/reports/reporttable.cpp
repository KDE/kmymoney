/***************************************************************************
                          reporttable.cpp  -  description
                             -------------------
    begin                : Fr Apr 16 2010
    copyright            : (C) 2010 Bernd Gonsior
    email                : bgo@freeplexx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "reporttable.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"

reports::ReportTable::ReportTable():
    m_resourceHtml("html"),
    m_reportStyleSheet("reportstylesheet"),
    m_cssFileDefault("kmymoney.css")

{
  // set up default values
  m_resourceType = QLatin1String("appdata").latin1();
}

QString reports::ReportTable::cssFileNameGet()
{
  QString cssfilename;

  if (!MyMoneyFile::instance()->value(m_reportStyleSheet).isEmpty()) {
    // try to find the stylesheet specific for this report
    cssfilename = KGlobal::dirs()->
                  findResource(m_resourceType, m_resourceHtml + '/'
                               + MyMoneyFile::instance()->
                               value(m_reportStyleSheet));
  }

  if (cssfilename.isEmpty()) {
    // if no report specific stylesheet was found, try to use the configured one
    cssfilename = KMyMoneyGlobalSettings::cssFileDefault();
  }

  if (cssfilename.isEmpty()) {
    // if there still is nothing, try to use the installation default
    cssfilename = KGlobal::dirs()->
                  findResource(m_resourceType, m_resourceHtml + '/'
                               + m_cssFileDefault);
  }

  return cssfilename;
}

QString reports::ReportTable::renderHeader(const QString& title, bool includeCSS)
{
  QString header = QString("<!DOCTYPE HTML PUBLIC")
                   + " \"-//W3C//DTD HTML 4.01 //EN\""
                   + " \"http://www.w3.org/TR/html4/strict.dtd\">"
                   + "\n<html>\n<head>"
                   + "\n<meta http-equiv=\"Content-Type\""
                   + " content=\"text/html; charset=" + m_encoding + "\" />"
                   + "\n<title>" + title + "</title>";

  QString cssfilename = cssFileNameGet();

  if (includeCSS) {
    // include css inline
    QFile cssFile(cssfilename);
    if (cssFile.open(QIODevice::ReadOnly)) {
      QTextStream cssStream(&cssFile);
      header += QString("\n<style type=\"text/css\">")
                + "\n<!--\n"
                + cssStream.readAll()
                + "\n-->\n</style>\n";
      cssFile.close();
    } else {
      qDebug() << "reports::ReportTable::htmlHeaderGet: could not open file "
      << cssfilename << " readonly";
    }
  } else {
    // do not include css inline instead use a link to the css file
    header += "\n<link rel=\"stylesheet\" type=\"text/css\" href=\""
              + cssfilename + "\">\n";
  }

  header += KMyMoneyUtils::variableCSS();

  header += "</head>\n<body>\n";

  return header;
}

QString reports::ReportTable::renderFooter()
{
  return "</body>\n</html>\n";
}

QString reports::ReportTable::renderHTML(QWidget* widget,
    const QByteArray& encoding, const QString& title, bool includeCSS)
{

  m_encoding = encoding;

  //this render the HEAD tag and sets the correct css file
  QString html = renderHeader(title, includeCSS);

  try {
    //this method is implemented by each concrete class
    html += renderBody();
  } catch (const MyMoneyException &e) {
    kDebug(2) << "reports::ReportTable::renderHTML(): ERROR " << e.what();

    QString error = i18n("There was an error creating your report: \"%1\".\nPlease report this error to the developer's list: kmymoney-devel@kde.org", e.what());

    KMessageBox::error(widget, error, i18n("Critical Error"));

    html += "<h1>" + i18n("Unable to generate report") + "</h1><p>" + error + "</p>";
  }

  //this renders a common footer
  html += renderFooter();

  return html;
}
