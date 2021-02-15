/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KLOADTEMPLATEDLG_H
#define KLOADTEMPLATEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

template <typename T> class QList;

namespace Ui { class KLoadTemplateDlg; }

class MyMoneyTemplate;

/// This dialog lets the user load more account templates
class KLoadTemplateDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KLoadTemplateDlg)

public:
  explicit KLoadTemplateDlg(QWidget *parent = nullptr);
  ~KLoadTemplateDlg();

  QList<MyMoneyTemplate> templates() const;

private Q_SLOTS:
  void slotHelp();

private:
  Ui::KLoadTemplateDlg *ui;
};

#endif
