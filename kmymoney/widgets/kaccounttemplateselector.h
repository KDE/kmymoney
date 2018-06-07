/*
 * Copyright 2008-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
