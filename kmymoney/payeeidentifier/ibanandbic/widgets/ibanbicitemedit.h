/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IBANBICITEMEDIT_H
#define IBANBICITEMEDIT_H

#include <QWidget>
#include <payeeidentifier/payeeidentifier.h>

namespace Ui
{
class ibanBicItemEdit;
}

class ibanBicItemEdit : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(payeeIdentifier identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged STORED true)
  Q_PROPERTY(QString iban READ iban WRITE setIban NOTIFY ibanChanged STORED false DESIGNABLE true)
  Q_PROPERTY(QString bic READ bic WRITE setBic NOTIFY bicChanged STORED false DESIGNABLE true)

public:
  explicit ibanBicItemEdit(QWidget* parent = 0);

  payeeIdentifier identifier() const;
  QString iban() const;
  QString bic() const;

public Q_SLOTS:
  void setIdentifier(const payeeIdentifier&);
  void setIban(const QString&);
  void setBic(const QString&);

Q_SIGNALS:
  void commitData(QWidget*);
  void closeEditor(QWidget* editor);
  void identifierChanged(payeeIdentifier);
  void ibanChanged(QString);
  void bicChanged(QString);

private Q_SLOTS:
  void updateIdentifier();

  /** @brief emits commitData(this) and closeEditor(this) */
  void editFinished();

private:
  struct Private;
  Private* d;
};

#endif // IBANBICITEMEDIT_H
