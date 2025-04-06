/*
    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYFACTORY_H
#define MYMONEYFACTORY_H

#include "kmm_mymoney_export.h"

#include <typeinfo>

#include <QMetaType>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QWidget>

/**
 * This class represents the MyMoney Factory interface
 * It contains the client implementation and allows to
 * be overwritten with the actual factory code.
 *
 * @author Thomas Baumgart
 */
class KMM_MYMONEY_EXPORT MyMoneyFactory
{
public:
    explicit MyMoneyFactory(QObject* owner);
    virtual ~MyMoneyFactory() = default;

    template<class T>
    T* create()
    {
        return qobject_cast<T*>(createFactoryObject(m_parent, className(typeid(T).name())));
    };

    template<class T, class U>
    void registerCreator(T* (*creator)(U*))
    {
        registerFactoryObject(className(typeid(T).name()), reinterpret_cast<void* (*)(U*)>(creator));
    };

    template<class T>
    void unregisterCreator()
    {
        unregisterFactoryObject(className(typeid(T).name()));
    };

protected:
    virtual QObject* createFactoryObject(QObject* parent, const QString& objectName);
    void registerFactoryObject(const QString& objectName, void* (*creator)(QObject*));
    void registerFactoryObject(const QString& objectName, void* (*creator)(QWidget*));
    void unregisterFactoryObject(const QString& objectName);
    QObject* createObject(QObject* parent, const QString& objectName);

private:
    const char* className(const char* objectName);

private:
    QObject* m_parent;
    QMap<QString, void* (*)(QObject*)> m_objectMap;
    QMap<QString, void* (*)(QWidget*)> m_widgetMap;
};

#endif // MYMONEYFACTORY_H
