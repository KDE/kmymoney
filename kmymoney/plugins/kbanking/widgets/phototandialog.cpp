/*
 * A tan input dialog for optical photoTan used in online banking
 * Copyright 2019  Jürgen Diez
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

#include "phototandialog.h"
#include "ui_phototandialog.h"

// Qt Includes
#include <QPushButton>
#include <QRegExpValidator>

#include <KLocalizedString>

// Project Includes
#include "kbankingsettings.h"

photoTanDialog::photoTanDialog(QWidget* parent)
  : QDialog(parent)
  , m_accepted(true)
{
  ui.reset(new Ui::photoTanDialog);
  ui->setupUi(this);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &photoTanDialog::accept);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &photoTanDialog::reject);
  connect(ui->tanInput, &QLineEdit::textEdited, this, &photoTanDialog::tanInputChanged);

  tanInputChanged(QString());
  ui->tanInput->setFocus();
}

photoTanDialog::~photoTanDialog()
{
}

void photoTanDialog::accept()
{
  m_tan = ui->tanInput->text();
  m_accepted = true;
  done(Accepted);
}

void photoTanDialog::reject()
{
  m_accepted = false;
  done(Rejected);
}

void photoTanDialog::setInfoText(const QString& text)
{
  ui->infoText->setText(text);
}

QString photoTanDialog::infoText()
{
  return ui->infoText->toPlainText();
}

void photoTanDialog::setPicture(const QPixmap &picture)
{
  QGraphicsScene *scene = new QGraphicsScene();
  pictureItem = scene->addPixmap(picture);
  ui->graphicsView->setScene(scene);
}

QPixmap photoTanDialog::picture()
{
  return pictureItem->pixmap();
}

QString photoTanDialog::tan()
{
  return m_tan;
}

void photoTanDialog::setTanLimits(const int& minLength, const int& maxLength)
{
  ui->tanInput->setValidator(new QRegExpValidator(QRegExp(QString("\\d{%1,%2}").arg(minLength).arg(maxLength)), ui->tanInput));
}

void photoTanDialog::tanInputChanged(const QString& input)
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
