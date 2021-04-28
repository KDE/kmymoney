/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
        JournalEntry,
        Schedule,
        Tag,
        Budget,
        OnlineJob,
        Report,
    } Object_t;
    SelectedObjects();

    void addSelections( Object_t type, const QStringList& ids);
    void addSelection( Object_t type, const QString& id);
    void setSelection( Object_t type, const QStringList& ids);
    void setSelection( Object_t type, const QString& id);

    void clearSelections( Object_t type);
    void clearSelections();
    QStringList selection( Object_t type) const;

    /**
     * This method returns the first id of the corresponding
     * selection set for @a type or an empty QString in
     * case the selection is empty.
     */
    QString firstSelection(Object_t type) const;

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
