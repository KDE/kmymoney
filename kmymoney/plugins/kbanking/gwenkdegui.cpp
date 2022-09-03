/*
    A gwenhywfar gui for aqbanking using KDE widgets
    SPDX-FileCopyrightText: 2014-2016 Christian David <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

// Gwenhywfar uses the deprecated QFlags constructor in a header. This will be fixed in master shortly.
#undef QT_DISABLE_DEPRECATED_BEFORE

#include "gwenkdegui.h"

#include <gwenhywfar/debug.h>
#include "gwen-gui-qt5/qt5dialogbox.hpp"


#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QApplication>

#include <QDebug>
#include <QPointer>
#include <QLineEdit>

#include "passstore.h"
#include "passwordtoggle.h"
#include "widgets/chiptandialog.h"
#include "widgets/phototandialog.h"

#define KBANKING_TANMETHOD_TEXT          0x00000001
#define KBANKING_TANMETHOD_CHIPTAN       0x00000002
#define KBANKING_TANMETHOD_CHIPTAN_OPTIC 0x00000003
#define KBANKING_TANMETHOD_CHIPTAN_USB   0x00000004
#define KBANKING_TANMETHOD_CHIPTAN_QR    0x00000005
#define KBANKING_TANMETHOD_PHOTOTAN      0x00000006


gwenKdeGui::gwenKdeGui()
    : QT5_Gui()
{
}

gwenKdeGui::~gwenKdeGui()
{
}

int gwenKdeGui::execDialog(GWEN_DIALOG *dlg, GWEN_UNUSED uint32_t guiid)
{
    QT5_GuiDialog qt5Dlg(this, dlg);
    QWidget *owner = qApp->activeWindow();

    /* setup widget tree for the dialog */
    if (!(qt5Dlg.setup(owner))) {
        return GWEN_ERROR_GENERIC;
    }

    PassStore* passStore(nullptr);
    QDialog* dialog = dynamic_cast<QDialog*>(qt5Dlg.getMainWindow());
    const auto lineedits = dialog->findChildren<QLineEdit*>();
    for (const auto& edit : qAsConst(lineedits)) {
        if (edit->echoMode() == QLineEdit::Password) {
            // check for available pass entry
            passStore = new PassStore(edit, QLatin1String("KMyMoney/KBanking"), passwordId);
            if (!passStore->isActionVisible()) {
                qDebug() << "No password found for KMyMoney/KBanking/" << passwordId;
            }
            new PasswordToggle(edit);
            break;
        }
    }
    auto rc = qt5Dlg.execute();

    return rc;
}

int gwenKdeGui::getPasswordText(uint32_t flags,
                                const char* token,
                                const char* title,
                                const char* text,
                                char* buffer,
                                int minLen,
                                int maxLen,
                                GWEN_GUI_PASSWORD_METHOD methodId,
                                GWEN_DB_NODE* methodParams,
                                uint32_t guiid)
{
    // Keep the current password identifier
    passwordId = QString(token);

    // convert HTML linebreaks into regular ones
    QString txt = QString::fromUtf8(text);
    txt.replace(QLatin1String("<br>"), QLatin1String("\n"));

    return QT5_Gui::getPassword(flags, token, title, txt.toUtf8(), buffer, minLen, maxLen, methodId, methodParams, guiid);
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

    // destruction of the dialog happens from within Qt
    // when the parent widget gets destroyed
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
        strncpy(buffer, tan.toUtf8().constData(), tan.length());
        buffer[tan.length()] = 0;
        return 0;
    }
    qDebug("Received Tan with incorrect length by ui.");
    return GWEN_ERROR_INTERNAL;
}



int gwenKdeGui::getPasswordPhoto(uint32_t /*flags*/,
                                 const char * /*token*/,
                                 const char * /*title*/,
                                 const char *text,
                                 char *buffer,
                                 int minLen,
                                 int maxLen,
                                 GWEN_GUI_PASSWORD_METHOD /*methodId*/,
                                 GWEN_DB_NODE *methodParams,
                                 uint32_t /*guiid*/) {
    QPixmap picture;
    QString infoText;
    const uchar * pictureData;
    unsigned int pictureDataSize;

    pictureData = (const uchar *)GWEN_DB_GetBinValue(methodParams, "imageData", 0, NULL, 0, &pictureDataSize);
    if (! (pictureData && pictureDataSize > 0)) {
        DBG_ERROR(0, "Empty optical data");
        return GWEN_ERROR_NO_DATA;
    }

    bool loadSuccessful = picture.loadFromData(pictureData, pictureDataSize);
    if (! loadSuccessful) {
        DBG_ERROR(0, "Unable to read tan picture from image data");
        return GWEN_ERROR_NO_DATA;
    }

    infoText = QString::fromUtf8(text);

    // destruction of the dialog happens from within Qt
    // when the parent widget gets destroyed
    QPointer<photoTanDialog> dialog = new photoTanDialog(getParentWidget());
    dialog->setInfoText(infoText);
    dialog->setPicture(picture);
    dialog->setTanLimits(minLen, maxLen);

    const int rv = dialog->exec();

    if (rv == photoTanDialog::Rejected)
        return GWEN_ERROR_USER_ABORTED;
    else if (rv == photoTanDialog::InternalError || dialog.isNull())
        return GWEN_ERROR_INTERNAL;

    QString tan = dialog->tan();
    if (tan.length() >= minLen && tan.length() <= maxLen) {
        strncpy(buffer, tan.toUtf8().constData(), tan.length());
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
        int tanMethodId = GWEN_DB_GetIntValue(methodParams, "tanMethodId", 0, 0);
        switch(tanMethodId) {
        case KBANKING_TANMETHOD_CHIPTAN_OPTIC:
            return getPasswordHhd(flags,
                                  token,
                                  title,
                                  text,
                                  buffer,
                                  minLen,
                                  maxLen,
                                  methodId, methodParams,
                                  guiid);
        case KBANKING_TANMETHOD_CHIPTAN_QR:
        case KBANKING_TANMETHOD_PHOTOTAN:
            return getPasswordPhoto(flags,
                                    token,
                                    title,
                                    text,
                                    buffer,
                                    minLen,
                                    maxLen,
                                    methodId, methodParams,
                                    guiid);
        default:
            DBG_ERROR(0, "Unknown tan method ID %i", tanMethodId);
            return GWEN_ERROR_NO_DATA;
        }

        /* intentionally omit "default:" here to be informed when new password methods are added to
         * gwen which have not been implemented. */
    }

    DBG_ERROR(0, "Unhandled password method id %08x", methodId);
    return GWEN_ERROR_INVALID;
}
