/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Christian David <c.david@christian-david.de>
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

namespace Ui
{
class ibanBicItemEdit;
}

class ibanBicItemEdit : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString iban READ iban WRITE setIban NOTIFY ibanChanged STORED true DESIGNABLE true)
  Q_PROPERTY(QString bic READ bic WRITE setBic NOTIFY bicChanged STORED true DESIGNABLE true)

public:
  ibanBicItemEdit(QWidget* parent = 0);

  QString iban() const;
  QString bic() const;

public slots:
  void setIban( QString );
  void setBic( QString );

signals:
  void ibanChanged( QString );
  void bicChanged( QString );

private:
  Ui::ibanBicItemEdit* ui;
};

#endif // IBANBICITEMEDIT_H
