/*
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
