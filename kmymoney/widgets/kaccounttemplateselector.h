/***************************************************************************
                          kaccounttemplateselector.h  -  description
                             -------------------
    begin                : Tue Feb 5 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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
