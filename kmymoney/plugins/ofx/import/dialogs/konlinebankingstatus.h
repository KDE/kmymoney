/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KONLINEBANKINGSTATUS_H
#define KONLINEBANKINGSTATUS_H

// ----------------------------------------------------------------------------
// Library Includes

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_konlinebankingstatusdecl.h"
class MyMoneyAccount;
class OfxAppVersion;
class OfxHeaderVersion;

/**
  * @author Thomas Baumgart
  */

class KOnlineBankingStatusDecl : public QWidget, public Ui::KOnlineBankingStatusDecl
{
public:
  explicit KOnlineBankingStatusDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class KOnlineBankingStatus : public KOnlineBankingStatusDecl
{
  Q_OBJECT
public:
  explicit KOnlineBankingStatus(const MyMoneyAccount& acc, QWidget *parent = 0);
  ~KOnlineBankingStatus();
  const QString appId() const;
  QString headerVersion() const;
protected Q_SLOTS:
  void applicationSelectionChanged();

private:
  OfxAppVersion*  m_appId;
  OfxHeaderVersion* m_headerVersion;
};

#endif
