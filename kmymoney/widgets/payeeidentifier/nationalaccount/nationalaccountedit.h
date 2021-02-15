/*
 * SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef NATIONALACCOUNTEDIT_H
#define NATIONALACCOUNTEDIT_H

#include <QWidget>
#include "payeeidentifier/payeeidentifier.h"

namespace Ui
{
class nationalAccountEdit;
}

class nationalAccountEdit : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(payeeIdentifier identifier READ identifier WRITE setIdentifier STORED true)
  Q_PROPERTY(QString accountNumber READ accountNumber WRITE setAccountNumber NOTIFY accountNumberChannged STORED false DESIGNABLE true)
  Q_PROPERTY(QString institutionCode READ institutionCode WRITE setInstitutionCode NOTIFY institutionCodeChanged STORED false DESIGNABLE true)

public:
  explicit nationalAccountEdit(QWidget* parent = 0);

  payeeIdentifier identifier() const;
  QString accountNumber() const;
  QString institutionCode() const;

public Q_SLOTS:
  void setIdentifier(const payeeIdentifier&);
  void setAccountNumber(const QString&);
  void setInstitutionCode(const QString&);

Q_SIGNALS:
  void institutionCodeChanged(QString);
  void accountNumberChannged(QString);
  void commitData(QWidget*);
  void closeEditor(QWidget*);

private Q_SLOTS:
  void editFinished();

private:
  struct Private;
  Private* d;
};

#endif // NATIONALACCOUNTEDIT_H
