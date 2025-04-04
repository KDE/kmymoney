/*

    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UNAVAILABLETASK_H
#define UNAVAILABLETASK_H

#include "onlinetasks/interfaces/tasks/onlinetask.h"

/**
 * @brief Task which can be used if original task is unavailable
 *
 * This task simply stores the XML data given to it and can write it back.
 *
 * The XML storage backend needs to load all tasks into memory. To prevent
 * data corruption if the original task cannot be loaded, this task can be used.
 */
class unavailableTask : public onlineTask
{
public:
    ONLINETASK_META(unavailableTask, "org.kmymoney.onlineTask.unavailableTask");
    bool isValid() const override;
    QString jobTypeName() const override;

    void writeXML(QXmlStreamWriter* writer) const override;

protected:
    QString responsibleAccount() const override;
    QString purpose() const override;
    unavailableTask* createFromXml(QXmlStreamReader* reader) const override;

    bool hasReferenceTo(const QString& id) const override;

    /**
     * @copydoc MyMoneyObject::referencedObjects
     */
    KMMStringSet referencedObjects() const override;

    unavailableTask* clone() const override;

private:
    explicit unavailableTask(const QString& xmlData);

    /**
     * The data received by createFromXml(). Written back by writeXML().
     */
    QString m_data;
};

#endif // UNAVAILABLETASK_H
