/*
    A tan input dialog for optical photoTan used in online banking
    SPDX-FileCopyrightText: 2019 Jürgen Diez
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

#ifndef PHOTOTANDIALOG_H
#define PHOTOTANDIALOG_H

#include <memory>

#include <QDialog>
#include <QGraphicsPixmapItem>

namespace Ui
{
class photoTanDialog;
}

class photoTanDialog : public QDialog
{
  Q_OBJECT
  Q_PROPERTY(QString infoText READ infoText() WRITE setInfoText)
  Q_PROPERTY(QPixmap picture READ picture() WRITE setPicture)

public:
  explicit photoTanDialog(QWidget* parent = 0);
  ~photoTanDialog();

  enum Result { Accepted = 0, Rejected, InternalError };

  QString infoText();
  QString tan();
  QPixmap picture();

public Q_SLOTS:
  void accept() final override;
  void reject() final override;

  void setInfoText(const QString&);
  void setPicture(const QPixmap&);

  void setTanLimits(const int& minLength, const int& maxLength);

private Q_SLOTS:
  void tanInputChanged(const QString&);

private:
  std::unique_ptr<Ui::photoTanDialog> ui;
  QGraphicsPixmapItem *pictureItem;
  QString m_tan;
  bool m_accepted;
};

#endif // PHOTOTANDIALOG_H
