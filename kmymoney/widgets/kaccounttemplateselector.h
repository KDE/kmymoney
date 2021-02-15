/*
 * SPDX-FileCopyrightText: 2008-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KACCOUNTTEMPLATESELECTOR_H
#define KACCOUNTTEMPLATESELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyTemplate;

/**
 * @author Thomas Baumgart <ipwizard@users.sourceforge.net>
 */

class KAccountTemplateSelectorPrivate;
class KAccountTemplateSelector : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KAccountTemplateSelector)

public:
  explicit KAccountTemplateSelector(QWidget* parent = nullptr);
  ~KAccountTemplateSelector();

  QList<MyMoneyTemplate> selectedTemplates() const;

private Q_SLOTS:
  void slotLoadHierarchy();
  void slotLoadCountry();
  void slotLoadTemplateList();

private:
  KAccountTemplateSelectorPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KAccountTemplateSelector)
};

#endif
