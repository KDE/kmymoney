/*
 * A tan input dialog for optical chipTan used in online banking
 * Copyright 2014  Christian David <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHIPTANDIALOG_H
#define CHIPTANDIALOG_H

#include <memory>

#include <QDialog>

namespace Ui
{
class chipTanDialog;
}

class chipTanDialog : public QDialog
{
  Q_OBJECT
  Q_PROPERTY(QString infoText READ infoText() WRITE setInfoText)
  Q_PROPERTY(QString hhdCode READ hhdCode() WRITE setHhdCode)
  Q_PROPERTY(int flickerFieldWidth READ flickerFieldWidth WRITE setFlickerFieldWidth)

public:
  explicit chipTanDialog(QWidget* parent = 0);
  ~chipTanDialog();

  enum Result { Accepted = 0, Rejected, InternalError };

  QString infoText();
  QString hhdCode();
  QString tan();
  int flickerFieldWidth();

public Q_SLOTS:
  void accept() final override;
  void reject() final override;

  void setInfoText(const QString&);
  void setHhdCode(const QString&);

  void setTanLimits(const int& minLength, const int& maxLength);
  void setFlickerFieldWidth(const int& width);
  void setFlickerFieldClockSetting(const int& width);

private Q_SLOTS:
  void tanInputChanged(const QString&);
  void flickerFieldWidthChanged(const int& width);
  void flickerFieldClockSettingChanged(const int& takt);

private:
  std::unique_ptr<Ui::chipTanDialog> ui;
  QString m_tan;
  bool m_accepted;

  void setRootObjectProperty(const char* property, const QVariant& value);
};

#endif // CHIPTANDIALOG_H
