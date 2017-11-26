/*
 * A gwenhywfar gui for aqbanking using KDE widgets
 * Copyright 2014 - 2016 Christian David <christian-david@web.de>
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

#ifndef GWENKDEGUI_H
#define GWENKDEGUI_H

#include <QObject>

#include "gwen-gui-qt5/qt5_gui.hpp"

/**
 * @brief Gwenhywfar Gui by KDE
 *
 *
 * @author Christian David
 */
class gwenKdeGui : public QT5_Gui
{
public:
  gwenKdeGui();
  ~gwenKdeGui();

  virtual int getPassword(uint32_t flags,
                          const char *token,
                          const char *title,
                          const char *text,
                          char *buffer,
                          int minLen,
                          int maxLen,
                          uint32_t guiid);
};

/**
 * @brief Helper class which is receiver for several signals
 */
class gwenKdeGuiTanResult : public QObject
{
  Q_OBJECT

public:
  explicit gwenKdeGuiTanResult(QObject* parent = nullptr)
      : QObject(parent),
      m_tan(QString()),
      m_aborted(false)
      {}

  virtual ~gwenKdeGuiTanResult() {}

  QString tan() {
    return m_tan;
  }

  bool aborted() {
    return m_aborted;
  }

public Q_SLOTS:
  void abort() {
    m_aborted = true;
  }

  void acceptTan(QString tan) {
    m_tan = tan;
    m_aborted = false;
  }

private:
  QString m_tan;
  bool m_aborted;
};

#endif // GWENKDEGUI_H
