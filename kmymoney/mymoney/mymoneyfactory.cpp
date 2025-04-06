/*
    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyfactory.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>

// ----------------------------------------------------------------------------
// Project Includes

MyMoneyFactory::MyMoneyFactory(QObject* parent)
    : m_parent(parent)
{
}

QObject* MyMoneyFactory::createFactoryObject(QObject* parent, const QString& objectName)
{
    for (auto p = parent; p != nullptr; p = p->parent()) {
        if (p->metaObject()->indexOfMethod("createFactoryObject(QObject*,QString)") != -1) {
            QObject* createdObject(nullptr);
            p->metaObject()->invokeMethod(p,
                                          "createFactoryObject",
                                          Qt::DirectConnection,
                                          Q_RETURN_ARG(QObject*, createdObject),
                                          Q_ARG(QObject*, parent),
                                          Q_ARG(const QString&, objectName));
            return createdObject;
        }
    }
    return nullptr;
}

void MyMoneyFactory::registerFactoryObject(const QString& objectName, void* (*creator)(QObject*))
{
    m_objectMap.insert(objectName, creator);
}

void MyMoneyFactory::registerFactoryObject(const QString& objectName, void* (*creator)(QWidget*))
{
    m_widgetMap.insert(objectName, creator);
}

void MyMoneyFactory::unregisterFactoryObject(const QString& objectName)
{
    m_objectMap.remove(objectName);
    m_widgetMap.remove(objectName);
}

QObject* MyMoneyFactory::createObject(QObject* parent, const QString& objectName)
{
    auto objectCreator = m_objectMap.find(objectName);
    if (objectCreator != m_objectMap.end()) {
        return reinterpret_cast<QObject*>((*objectCreator)(parent));
    }
    auto widgetCreator = m_widgetMap.find(objectName);
    if (widgetCreator != m_widgetMap.end()) {
        return reinterpret_cast<QWidget*>((*widgetCreator)(reinterpret_cast<QWidget*>(parent)));
    }
    qDebug() << "Request to create" << objectName << "which is not registered. Aborting";
    exit(1);
}

const char* MyMoneyFactory::className(const char* objectName)
{
    while (*objectName >= '0' && *objectName <= '9') {
        ++objectName;
    }
    return objectName;
}
