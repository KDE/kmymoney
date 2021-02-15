/*
 * A gwenhywfar gui for aqbanking using KDE widgets
 * SPDX-FileCopyrightText: 2014-2016 Christian David <christian-david@web.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#ifndef GWENKDEGUI_H
#define GWENKDEGUI_H

#include <QObject>

#include "gwen-gui-qt5/qt5_gui.hpp"
#include "gwen-gui-qt5/qt5_gui_dialog.hpp"

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

  int getPassword(uint32_t flags,
                          const char *token,
                          const char *title,
                          const char *text,
                          char *buffer,
                          int minLen,
                          int maxLen,
                          GWEN_GUI_PASSWORD_METHOD methodId,
                          GWEN_DB_NODE *methodParams,
                          uint32_t guiid) final override;

  int execDialog(GWEN_DIALOG *dlg, GWEN_UNUSED uint32_t guiid) final override;

private:
  int getPasswordText(uint32_t flags,
                      const char *token,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen,
                      GWEN_GUI_PASSWORD_METHOD methodId,
                      GWEN_DB_NODE *methodParams,
                      uint32_t guiid);
  int getPasswordHhd(uint32_t flags,
                     const char *token,
                     const char *title,
                     const char *text,
                     char *buffer,
                     int minLen,
                     int maxLen,
                     GWEN_GUI_PASSWORD_METHOD methodId,
                     GWEN_DB_NODE *methodParams,
                     uint32_t guiid);
   int getPasswordPhoto(uint32_t flags,
                      const char *token,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen,
                      GWEN_GUI_PASSWORD_METHOD methodId,
                      GWEN_DB_NODE *methodParams,
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

  ~gwenKdeGuiTanResult() {}

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
