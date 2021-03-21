/*

    SPDX-FileCopyrightText: 2013 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DUMMYTASK_H
#define DUMMYTASK_H

#include "onlinetasks/interfaces/tasks/onlinetask.h"

class dummyTask : public onlineTask
{
public:
    ONLINETASK_META(dummyTask, "org.kmymoney.onlinetasks.dummy");
    dummyTask()
        : m_testNumber(0) {

    }

    dummyTask(const dummyTask& other)
        : onlineTask(other),
          m_testNumber(other.m_testNumber) {
    }

    /**
     * @brief Checks if the task is ready for sending
     */
    bool isValid() const final override {
        return true;
    }

    /**
     * @brief Human readable type-name
     */
    QString jobTypeName() const final override {
        return QLatin1String("Dummy task");
    }

    void setTestNumber(const int& number) {
        m_testNumber = number;
    }
    int testNumber() {
        return m_testNumber;
    }

    void writeXML(QDomDocument&, QDomElement&) const final override {}

protected:

    dummyTask* clone() const final override {
        return (new dummyTask(*this));
    }
    bool hasReferenceTo(const QString &id) const final override {
        Q_UNUSED(id);
        return false;
    }
    QSet<QString> referencedObjects() const final override {
        return {};
    }
    dummyTask* createFromXml(const QDomElement&) const final override {
        return (new dummyTask);
    }

    QString responsibleAccount() const final override {
        return QString();
    }

    int m_testNumber;
};

#endif // DUMMYTASK_H
