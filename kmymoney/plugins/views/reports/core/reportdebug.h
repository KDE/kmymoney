/*
    SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REPORTDEBUG_H
#define REPORTDEBUG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace reports
{

// define to enable massive debug logging to stderr
#undef DEBUG_REPORTS
// #define DEBUG_REPORTS

#define DEBUG_ENABLED_BY_DEFAULT false

#ifdef DEBUG_REPORTS

// define to filter out account names & transaction amounts
// DO NOT check into CVS with this defined!! It breaks all
// unit tests.
#undef DEBUG_HIDE_SENSITIVE

#define DEBUG_ENTER(x) Debug ___DEBUG(x)
#define DEBUG_OUTPUT(x) ___DEBUG.output(x)
#define DEBUG_OUTPUT_IF(x,y) { if (x) ___DEBUG.output(y); }
#define DEBUG_ENABLE(x) Debug::enable(x)
#define DEBUG_ENABLE_KEY(x) Debug::setEnableKey(x)
#ifdef DEBUG_HIDE_SENSITIVE
#define DEBUG_SENSITIVE(x) QString("hidden")
#else
#define DEBUG_SENSITIVE(x) (x)
#endif

#else

#define DEBUG_ENTER(x)
#define DEBUG_OUTPUT(x)
#define DEBUG_OUTPUT_IF(x,y)
#define DEBUG_ENABLE(x)
#define DEBUG_SENSITIVE(x)
#endif

class Debug
{
  QString m_methodName;
  static QString m_sTabs;
  static bool m_sEnabled;
  bool m_enabled;
  static QString m_sEnableKey;
public:
  explicit Debug(const QString& _name);
  ~Debug();
  void output(const QString& _text);
  static void enable(bool _e) {
    m_sEnabled = _e;
  }
  static void setEnableKey(const QString& _s) {
    m_sEnableKey = _s;
  }
};

} // end namespace reports

#endif // REPORTDEBUG_H
