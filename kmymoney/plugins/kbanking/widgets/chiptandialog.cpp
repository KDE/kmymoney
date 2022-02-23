/*
    A tan input dialog for optical chipTan used in online banking
    SPDX-FileCopyrightText: 2014 Christian David <christian-david@web.de>
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

#include "chiptandialog.h"
#include "ui_chiptandialog.h"

// Qt Includes
#include <QMetaObject>
#include <QPushButton>
#include <QQuickItem>
#include <QQuickView>
#include <QRegularExpressionValidator>
#include <QStandardPaths>

// KDE Includes
#include <KLocalizedString>

// Project Includes
#include "kbankingsettings.h"

chipTanDialog::chipTanDialog(QWidget* parent)
    : QDialog(parent)
    , m_accepted(true)
{
    ui.reset(new Ui::chipTanDialog);
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &chipTanDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &chipTanDialog::reject);
    connect(ui->tanInput, &QLineEdit::textEdited, this, &chipTanDialog::setTanInput);

    ui->declarativeView->setSource(QUrl("qrc:/plugins/kbanking/chipTan/ChipTan.qml"));

    setFlickerFieldWidth(KBankingSettings::width());
    setFlickerFieldClockSetting(KBankingSettings::clocksetting());

    connect(ui->decelerateButton, SIGNAL(clicked()), ui->declarativeView->rootObject(), SLOT(decelerateTransmission()));
    connect(ui->accelerateButton, SIGNAL(clicked()), ui->declarativeView->rootObject(), SLOT(accelerateTransmission()));
    connect(ui->enlargeButton, SIGNAL(clicked()), ui->declarativeView->rootObject(), SLOT(enlargeFlickerField()));
    connect(ui->reduceButton, SIGNAL(clicked()), ui->declarativeView->rootObject(), SLOT(reduceFlickerField()));

    connect(ui->declarativeView->rootObject(), SIGNAL(flickerFieldWidthChanged(int)), this, SLOT(setFlickerFieldWidth(int)));
    connect(ui->declarativeView->rootObject(), SIGNAL(flickerFieldClockSettingChanged(int)), SLOT(setFlickerFieldClockSetting(int)));

    if (ui->declarativeView->status() == QQuickWidget::Error)
        done(InternalError);

    setTanInput(QString());
    ui->tanInput->setFocus();
}

chipTanDialog::~chipTanDialog()
{
}

void chipTanDialog::accept()
{
    m_tan = ui->tanInput->text();
    m_accepted = true;
    done(Accepted);
}

void chipTanDialog::reject()
{
    m_accepted = false;
    done(Rejected);
}

void chipTanDialog::setInfoText(const QString& text)
{
    if (text != infoText()) {
        ui->infoText->setText(text);
        emit infoTextChanged(text);
    }
}

QString chipTanDialog::infoText()
{
    return ui->infoText->toPlainText();
}

void chipTanDialog::setHhdCode(const QString& code)
{
    if (hhdCode() != code) {
        setRootObjectProperty("transferData", code);
        emit hhdCodeChanged(code);
    }
}

QString chipTanDialog::hhdCode()
{
    QQuickItem* rootObject = ui->declarativeView->rootObject();
    if (rootObject)
        return rootObject->property("transferData").toString();
    return QString();
}

QString chipTanDialog::tan()
{
    return m_tan;
}

void chipTanDialog::setTanLimits(const int& minLength, const int& maxLength)
{
    ui->tanInput->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("\\d{%1,%2}").arg(minLength).arg(maxLength)), ui->tanInput));
}

int chipTanDialog::flickerFieldWidth()
{
    QQuickItem* rootObject = ui->declarativeView->rootObject();
    QVariant width;
    if (rootObject)
        QMetaObject::invokeMethod(rootObject, "flickerFieldWidth", Qt::DirectConnection, Q_RETURN_ARG(QVariant, width));

    return width.toInt();
}

void chipTanDialog::setFlickerFieldClockSetting(const int& clock)
{
    QQuickItem *const rootObject = ui->declarativeView->rootObject();
    if (rootObject) {
        QMetaObject::invokeMethod(rootObject, "setFlickerClockSetting", Q_ARG(QVariant, QVariant(clock)));
    }
    if (clock != KBankingSettings::clocksetting()) {
        KBankingSettings::setClocksetting(clock);
        KBankingSettings::self()->save();
    }
}

void chipTanDialog::setFlickerFieldWidth(const int& width)
{
    QQuickItem* rootObject = ui->declarativeView->rootObject();
    if (rootObject) {
        QMetaObject::invokeMethod(rootObject, "setFlickerFieldWidth", Q_ARG(QVariant, QVariant(width)));
        ui->declarativeView->setFixedWidth(width);
        if (width != KBankingSettings::width()) {
            KBankingSettings::setWidth(width);
            KBankingSettings::self()->save();
            emit flickerFieldWidthChanged(width);
        }
    }
}

void chipTanDialog::setTanInput(const QString& input)
{
    QPushButton *const button = ui->buttonBox->button(QDialogButtonBox::Ok);
    Q_ASSERT(button);
    if (input.isEmpty() || !ui->tanInput->hasAcceptableInput()) {
        button->setEnabled(false);
        button->setToolTip(i18n("A valid tan is required to proceed."));
    } else {
        button->setEnabled(true);
        button->setToolTip(QString());
    }
}

void chipTanDialog::setRootObjectProperty(const char* property, const QVariant& value)
{
    QQuickItem *const rootObject = ui->declarativeView->rootObject();
    if (rootObject)
        rootObject->setProperty(property, value);
}
