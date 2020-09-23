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

#ifndef TEMPLATEWRITER_H
#define TEMPLATEWRITER_H

#include "kmm_templates_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyTemplate;

/**
 * @author Thomas Baumgart
 */

class TemplateWriterPrivate;
class KMM_TEMPLATES_EXPORT TemplateWriter : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(TemplateWriter)
  Q_DECLARE_PRIVATE(TemplateWriter)

public:
  explicit TemplateWriter(QWidget* parent = nullptr);
  ~TemplateWriter();

  bool exportTemplate(const MyMoneyTemplate& tmpl, const QUrl &url);
  QString errorMessage() const;

private:
  TemplateWriterPrivate * const d_ptr;
};

#endif
