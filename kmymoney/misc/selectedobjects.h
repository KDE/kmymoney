/*
 * Copyright (C) 2020      Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SELECTEDOBJECTS_H
#define SELECTEDOBJECTS_H

#include <kmm_selections_export.h>

// ----------------------------------------------------------------------------
// QT Headers

#include <QHash>
#include <QString>
#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Headers


class KMM_SELECTIONS_EXPORT SelectedObjects
{
public:
    typedef enum {
        Account,
        Institution,
        Payee,
        Transaction,
        Schedule,
        Split,
        Tag,
        Budget,
        OnlineJob
    } Object_t;
    SelectedObjects();

    void addSelections( Object_t type, const QStringList& ids);
    void addSelection( Object_t type, const QString& id);
    void setSelection( Object_t type, const QStringList& ids);
    void setSelection( Object_t type, const QString& id);

    void clearSelections( Object_t type);
    void clearSelections();
    QStringList selection( Object_t type) const;

    int count( Object_t type) const;
    /**
     * Returns @c true if the selected @a type has no
     * selected objects, @c false otherwise.
     */
    bool isEmpty( Object_t type) const;

    /**
     * Returns @c true if no object is selected  @c false otherwise.
     */
    bool isEmpty() const;

private:
    QHash<Object_t, QStringList>    m_selections;
};

typedef QHash<SelectedObjects::Object_t, QStringList> ObjectSelections;
typedef const ObjectSelections& (*selectedObjectsFunction)();

#endif
