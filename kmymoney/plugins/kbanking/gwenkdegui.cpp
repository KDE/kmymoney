/*
 * A gwenhywfar gui for aqbanking using KDE widgets
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

#include "gwenkdegui.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>

#include <QDebug>
#include <QEventLoop>
#include <QPointer>
#include <KUrl>

#include "widgets/chiptandialog.h"

gwenKdeGui::gwenKdeGui()
    : QT4_Gui()
{

}

gwenKdeGui::~gwenKdeGui()
{

}

int gwenKdeGui::getPassword(uint32_t flags, const char* token, const char* title, const char* text, char* buffer, int minLen, int maxLen, uint32_t guiid)
{
  if ((flags & GWEN_GUI_INPUT_FLAGS_OPTICAL) && text && *text) {
    // Optical Tan (chipTan)

    // Extract text to display and hhd code
    QString infoText = QString::fromUtf8(text);
    QRegExp hhdRegExp = QRegExp("^(.*)\\$OBEGIN\\$(.*)\\$OEND\\$(.*)$", Qt::CaseInsensitive);
    hhdRegExp.setMinimal(true);
    hhdRegExp.indexIn(infoText);
    QStringList captured = hhdRegExp.capturedTexts();
    QString hhdCode = captured.at(2);
    infoText = captured.at(1) + captured.at(3);

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

  return QT4_Gui::getPassword(flags, token, title, text, buffer, minLen, maxLen, guiid);
}
