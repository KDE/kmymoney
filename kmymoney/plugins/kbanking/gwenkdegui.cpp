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

#include "gwenkdegui.h"

#include <gwenhywfar/debug.h>


#include <QString>
#include <QStringList>
#include <QRegularExpression>

#include <QDebug>
#include <QPointer>

#include "widgets/chiptandialog.h"

gwenKdeGui::gwenKdeGui()
    : QT5_Gui()
{

}

gwenKdeGui::~gwenKdeGui()
{

}



int gwenKdeGui::getPasswordText(uint32_t flags,
                                const char *token,
                                const char *title,
                                const char *text,
                                char *buffer,
                                int minLen,
                                int maxLen,
                                GWEN_GUI_PASSWORD_METHOD methodId,
                                GWEN_DB_NODE *methodParams,
                                uint32_t guiid) {
  return QT5_Gui::getPassword(flags, token, title, text, buffer, minLen, maxLen, methodId, methodParams, guiid);
}



int gwenKdeGui::getPasswordHhd(uint32_t /*flags*/,
                               const char * /*token*/,
                               const char * /*title*/,
                               const char *text,
                               char *buffer,
                               int minLen,
                               int maxLen,
                               GWEN_GUI_PASSWORD_METHOD /*methodId*/,
                               GWEN_DB_NODE *methodParams,
                               uint32_t /*guiid*/) {
  QString hhdCode;
  QString infoText;
  const char *sChallenge;

  sChallenge=GWEN_DB_GetCharValue(methodParams, "challenge", 0, NULL);
  if (! (sChallenge && *sChallenge)) {
    DBG_ERROR(0, "Empty optical data");
    return GWEN_ERROR_NO_DATA;
  }

  hhdCode = QString::fromUtf8(sChallenge);
  infoText = QString::fromUtf8(text);

  //! @todo: Memory leak?
  QPointer<chipTanDialog> dialog = new chipTanDialog(getParentWidget());
  dialog->setInfoText(infoText);
  dialog->setHhdCode(hhdCode);
  dialog->setTanLimits(minLen, maxLen);

  const int rv = dialog->exec();

  if (rv == chipTanDialog::Rejected)
    return GWEN_ERROR_USER_ABORTED;
  else if (rv == chipTanDialog::InternalError || dialog.isNull())
    return GWEN_ERROR_INTERNAL;

  QString tan = dialog->tan();
  if (tan.length() >= minLen && tan.length() <= maxLen) {
    strncpy(buffer, tan.toUtf8().constData() , tan.length());
    buffer[tan.length()] = 0;
    return 0;
  }
  qDebug("Received Tan with incorrect length by ui.");
  return GWEN_ERROR_INTERNAL;
}




int gwenKdeGui::getPassword(uint32_t flags, const char* token, const char* title, const char* text, char* buffer,
                            int minLen, int maxLen,
                            GWEN_GUI_PASSWORD_METHOD methodId,
                            GWEN_DB_NODE *methodParams,
                            uint32_t guiid)
{


  switch( (methodId & GWEN_Gui_PasswordMethod_Mask)) {
  case GWEN_Gui_PasswordMethod_Unknown:
  case GWEN_Gui_PasswordMethod_Mask:
    DBG_ERROR(0, "Invalid password method id %08x", methodId);
    return GWEN_ERROR_INVALID;

  case GWEN_Gui_PasswordMethod_Text:
    return getPasswordText(flags,
                           token,
                           title,
                           text,
                           buffer,
                           minLen,
                           maxLen,
                           methodId, methodParams,
                           guiid);

  case GWEN_Gui_PasswordMethod_OpticalHHD:
    return getPasswordHhd(flags,
                          token,
                          title,
                          text,
                          buffer,
                          minLen,
                          maxLen,
                          methodId, methodParams,
                          guiid);

    /* intentionally omit "default:" here to be informed when new password methods are added to
     * gwen which have not been implemented. */
  }

  DBG_ERROR(0, "Unhandled password method id %08x", methodId);
  return GWEN_ERROR_INVALID;
}
