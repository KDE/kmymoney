/***************************************************************************
                          kloadtemplatedlg.h
                             -------------------
    copyright            : (C) 2008 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
