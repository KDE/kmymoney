/*
 * Copyright 2014-2015  Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
