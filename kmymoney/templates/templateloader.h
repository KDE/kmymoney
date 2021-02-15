/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEMPLATELOADER_H
#define TEMPLATELOADER_H

#include "kmm_templates_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class TemplatesModel;
class MyMoneyTemplate;

/**
 * @author Thomas Baumgart
 */

class TemplateLoaderPrivate;
class KMM_TEMPLATES_EXPORT TemplateLoader : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(TemplateLoader)
  Q_DECLARE_PRIVATE(TemplateLoader)

public:
  explicit TemplateLoader(QWidget* parent = nullptr);
  ~TemplateLoader();

  void load(TemplatesModel* model);

  bool importTemplate(const MyMoneyTemplate& tmpl);

private Q_SLOTS:
  void slotLoadCountry();

Q_SIGNALS:
  void loadingFinished();

private:
  TemplateLoaderPrivate * const d_ptr;
};

#endif
