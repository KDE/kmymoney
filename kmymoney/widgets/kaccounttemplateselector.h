/*
 * SPDX-FileCopyrightText: 2008-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KACCOUNTTEMPLATESELECTOR_H
#define KACCOUNTTEMPLATESELECTOR_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <templatesmodel.h>

/**
 * @author Thomas Baumgart
 */

class KAccountTemplateSelectorPrivate;
class KMM_BASE_WIDGETS_EXPORT KAccountTemplateSelector : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KAccountTemplateSelector)

public:
  explicit KAccountTemplateSelector(QWidget* parent = nullptr);
  ~KAccountTemplateSelector();

  void setModel(TemplatesModel* model);

  QList<MyMoneyTemplate> selectedTemplates() const;

public Q_SLOTS:
  void setupInitialSelection();

private:
  KAccountTemplateSelectorPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KAccountTemplateSelector)
};

#endif
