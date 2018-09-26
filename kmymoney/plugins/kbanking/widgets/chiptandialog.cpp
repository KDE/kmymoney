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

#include "chiptandialog.h"
#include "ui_chiptandialog.h"

// Qt Includes
#include <QMetaObject>
#include <QQuickView>
#include <QQuickItem>
#include <QPushButton>
#include <QRegExpValidator>
#include <QStandardPaths>

#include <KLocalizedString>

// Project Includes
#include "kbankingsettings.h"

chipTanDialog::chipTanDialog(QWidget* parent)
  : QDialog(parent),
    m_tan(""),
    m_accepted(true)
{
  ui.reset(new Ui::chipTanDialog);
  ui->setupUi(this);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &chipTanDialog::accept);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &chipTanDialog::reject);
  connect(ui->tanInput, &QLineEdit::textEdited, this, &chipTanDialog::tanInputChanged);

  ui->declarativeView->setSource(QUrl(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kbanking/qml/chipTan/ChipTan.qml"))));

  setFlickerFieldWidth(KBankingSettings::width());
  setFlickerFieldClockSetting(KBankingSettings::clocksetting());

  connect(ui->decelerateButton, SIGNAL(clicked()), ui->declarativeView->rootObject(), SLOT(decelerateTransmission()));
  connect(ui->accelerateButton, SIGNAL(clicked()), ui->declarativeView->rootObject(), SLOT(accelerateTransmission()));
  connect(ui->enlargeButton, SIGNAL(clicked()), ui->declarativeView->rootObject(), SLOT(enlargeFlickerField()));
  connect(ui->reduceButton, SIGNAL(clicked()), ui->declarativeView->rootObject(), SLOT(reduceFlickerField()));

  connect(ui->declarativeView->rootObject(), SIGNAL(flickerFieldWidthChanged(int)), this, SLOT(flickerFieldWidthChanged(int)));
  connect(ui->declarativeView->rootObject(), SIGNAL(flickerFieldClockSettingChanged(int)), SLOT(flickerFieldClockSettingChanged(int)));

  if (ui->declarativeView->status() == QQuickWidget::Error)
    done(InternalError);

  tanInputChanged(QString());
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
  ui->infoText->setText(text);
}

QString chipTanDialog::infoText()
{
  return ui->infoText->toPlainText();
}

void chipTanDialog::setHhdCode(const QString& code)
{
  setRootObjectProperty("transferData", code);
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
  ui->tanInput->setValidator(new QRegExpValidator(QRegExp(QString("\\d{%1,%2}").arg(minLength).arg(maxLength)), ui->tanInput));
}

void chipTanDialog::setFlickerFieldWidth(const int& width)
{
  QQuickItem* rootObject = ui->declarativeView->rootObject();
  if (rootObject)
    QMetaObject::invokeMethod(rootObject, "setFlickerFieldWidth", Q_ARG(QVariant, QVariant(width)));
}

int chipTanDialog::flickerFieldWidth()
{
  QQuickItem* rootObject = ui->declarativeView->rootObject();
  QVariant width;
  if (rootObject)
    QMetaObject::invokeMethod(rootObject, "flickerFieldWidth", Qt::DirectConnection, Q_RETURN_ARG(QVariant, width));

  return width.toInt();
}

void chipTanDialog::setFlickerFieldClockSetting(const int& width)
{
  QQuickItem *const rootObject = ui->declarativeView->rootObject();
  if (rootObject)
    QMetaObject::invokeMethod(rootObject, "setFlickerClockSetting", Q_ARG(QVariant, QVariant(width)));
}

void chipTanDialog::flickerFieldClockSettingChanged(const int& takt)
{
  KBankingSettings::setClocksetting(takt);
  KBankingSettings::self()->save();
}

void chipTanDialog::flickerFieldWidthChanged(const int& width)
{
  ui->declarativeView->setFixedWidth(width);
  KBankingSettings::setWidth(width);
  KBankingSettings::self()->save();
}

void chipTanDialog::tanInputChanged(const QString& input)
{
  QPushButton *const button = ui->buttonBox->button(QDialogButtonBox::Ok);
  Q_ASSERT(button);
  if (input.isEmpty() || !ui->tanInput->hasAcceptableInput()) {
    button->setEnabled(false);
    button->setToolTip(i18n("A valid tan is required to proceed."));
  } else {
    button->setEnabled(true);
    button->setToolTip("");
  }
}

void chipTanDialog::setRootObjectProperty(const char* property, const QVariant& value)
{
  QQuickItem *const rootObject = ui->declarativeView->rootObject();
  if (rootObject)
    rootObject->setProperty(property, value);
}
