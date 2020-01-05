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
#include <QtCore/QMetaObject>
#include <QtDeclarative/QDeclarativeView>
#include <QGraphicsObject>
#include <QPushButton>

// KDE Includes
#include <KStandardDirs>

// Prject Includes
#include "kbankingsettings.h"

chipTanDialog::chipTanDialog(QWidget* parent)
    : QDialog(parent)
    , m_accepted(true)
{
  ui = new Ui::chipTanDialog;
  ui->setupUi(this);

  connect(ui->dialogButtonBox, SIGNAL(accepted()), SLOT(accept()));
  connect(ui->dialogButtonBox, SIGNAL(rejected()), SLOT(reject()));
  connect(ui->tanInput, SIGNAL(textChanged(QString)), SLOT(tanInputChanged(QString)));

  ui->declarativeView->setSource(KGlobal::dirs()->findResource("data", QLatin1String("kmm_kbanking/qml/chipTan/ChipTan.qml")));

  setFlickerFieldWidth(KBankingSettings::width());
  setFlickerFieldClockSetting(KBankingSettings::clocksetting());

  connect(ui->decelerateButton, SIGNAL(clicked(bool)), ui->declarativeView->rootObject(), SLOT(decelerateTransmission()));
  connect(ui->accelerateButton, SIGNAL(clicked(bool)), ui->declarativeView->rootObject(), SLOT(accelerateTransmission()));
  connect(ui->enlargeButton, SIGNAL(clicked(bool)), ui->declarativeView->rootObject(), SLOT(enlargeFlickerField()));
  connect(ui->reduceButton, SIGNAL(clicked(bool)), ui->declarativeView->rootObject(), SLOT(reduceFlickerField()));

  connect(ui->declarativeView->rootObject(), SIGNAL(flickerFieldWidthChanged(int)), SLOT(flickerFieldWidthChanged(int)));
  connect(ui->declarativeView->rootObject(), SIGNAL(flickerFieldClockSettingChanged(int)), SLOT(flickerFieldClockSettingChanged(int)));

  if (ui->declarativeView->status() == QDeclarativeView::Error)
    done(InternalError);

  tanInputChanged(QString());
}

chipTanDialog::~chipTanDialog()
{
  delete ui;
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
  QGraphicsObject* rootObject = ui->declarativeView->rootObject();
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
  QGraphicsObject* rootObject = ui->declarativeView->rootObject();
  if (rootObject)
    QMetaObject::invokeMethod(rootObject, "setFlickerFieldWidth", Q_ARG(QVariant, QVariant(width)));
}

int chipTanDialog::flickerFieldWidth()
{
  QGraphicsObject* rootObject = ui->declarativeView->rootObject();
  QVariant width;
  if (rootObject)
    QMetaObject::invokeMethod(rootObject, "flickerFieldWidth", Qt::DirectConnection, Q_RETURN_ARG(QVariant, width));

  return width.toInt();
}

void chipTanDialog::setFlickerFieldClockSetting(const int& width)
{
  QGraphicsObject* rootObject = ui->declarativeView->rootObject();
  if (rootObject)
    QMetaObject::invokeMethod(rootObject, "setFlickerClockSetting", Q_ARG(QVariant, QVariant(width)));
}

void chipTanDialog::flickerFieldClockSettingChanged(const int& takt)
{
  KBankingSettings::setClocksetting(takt);
  KBankingSettings::self()->writeConfig();
}

void chipTanDialog::flickerFieldWidthChanged(const int& width)
{
  KBankingSettings::setWidth(width);
  KBankingSettings::self()->writeConfig();
}

void chipTanDialog::tanInputChanged(const QString& input)
{
  QPushButton *button = ui->dialogButtonBox->button(QDialogButtonBox::Ok);
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
  QGraphicsObject* rootObject = ui->declarativeView->rootObject();
  if (rootObject)
    rootObject->setProperty(property, value);
}
