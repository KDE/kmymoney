/*
 * Copyright 2014-2016  Christian Dávid <christian-david@web.de>
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

#ifndef PAYEEIDENTIFIERLOADER_H
#define PAYEEIDENTIFIERLOADER_H

#include "kmm_payeeidentifier_loader_export.h"

class QAbstractItemDelegate;
class QString;
class QObject;
class QStringList;

/**
 *
 * @todo Load delegates dynamically
 */
class KMM_PAYEEIDENTIFIER_LOADER_EXPORT payeeIdentifierLoader
{
public:
  payeeIdentifierLoader();
  ~payeeIdentifierLoader();

  /**
   * @brief Create a delegate to show/edit
   *
   * The payeeIdentifier to edit is identified by payeeIdentifierId. parent is set as parent of the created
   * Delegate.
   *
   * @return a pointer to a delegate or null_ptr. Caller takes ownership.
   */
  QAbstractItemDelegate* createItemDelegate(const QString& payeeIdentifierId, QObject* parent = 0);

  /**
   * @brief Test if a delegate for editing is available
   */
  bool hasItemEditDelegate(const QString& payeeIdentifierId);

  /**
   * @brief List availableDelegates delegates
   *
   * @return a list of payeeIdentifierIds for which a delegate exists.
   * @see createItemDelegate()
   */
  QStringList availableDelegates();

  static payeeIdentifierLoader* instance() {
    return &m_self;
  }

private:
  static payeeIdentifierLoader m_self;
};

#endif // PAYEEIDENTIFIERLOADER_H
