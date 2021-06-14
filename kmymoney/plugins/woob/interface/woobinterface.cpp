/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDebug>
#include <QTemporaryFile>

#include "woobinterface.h"

// Python uses slots var that is QT macro
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QFileInfo>
#include <QLibrary>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QVariant>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../woobexc.h"

WoobInterface::WoobInterface()
    : m_woobInterface(nullptr)
{
    Q_INIT_RESOURCE(woobinterface);

    Py_Initialize();
    qDebug() << "Python interpreter found:" << Py_GetVersion();

    const auto scriptResourceName = ":/plugins/weboob/kmymoneyweboob.py";
    auto nativeScript = std::unique_ptr<QTemporaryFile>(QTemporaryFile::createNativeFile(scriptResourceName));

    if (!nativeScript) {
        qDebug() << "Failed to save a native copy of the embedded" << scriptResourceName << "script";
        return;
    }

    // createNativeFile() doesn't take templateName, so we need to rename the file post-creation to meet Python reqs
    nativeScript->rename(nativeScript->fileName().remove(QChar('.')).append(".py"));
    auto nativeScriptFileInfo = QFileInfo(nativeScript->fileName());
    qDebug() << "Saved a copy of the embedded" << scriptResourceName << "script as" << nativeScriptFileInfo.filePath();

    if (nativeScript->open()) {
        auto moduleName = nativeScriptFileInfo.baseName().toLocal8Bit();
        auto moduleLocation = nativeScriptFileInfo.absolutePath().toLocal8Bit();

        qDebug() << "Attempt to load the" << moduleName << "python module from" << moduleLocation;

        PyObject* sys = PyImport_ImportModule("sys");
        PyObject* path = PyObject_GetAttrString(sys, "path");
        PyObject* pyLocation = PyUnicode_FromString(moduleLocation);
        PyList_Append(path, pyLocation);

        m_woobInterface = PyImport_ImportModule(moduleName);

        if (m_woobInterface == nullptr) {
            PyErr_Print();
        } else {
            qDebug() << moduleName << "Python module loaded successfully";
        }

        Py_DECREF(sys);
        Py_DECREF(path);
        Py_DECREF(pyLocation);
    }
}

WoobInterface::~WoobInterface()
{
    if (m_woobInterface)
        Py_DECREF(m_woobInterface);

    if (Py_IsInitialized())
        Py_Finalize();
}

PyObject* WoobInterface::execute(QString method, QVariantList args)
{
    QMutex mutex;
    QMutexLocker locker(&mutex);

    PyObject* retVal = nullptr;
    auto ba = method.toLocal8Bit();
    const char* cmethod = ba.data();
    auto pyFunc = PyObject_GetAttrString(m_woobInterface, cmethod);
    if (pyFunc && PyCallable_Check(pyFunc)) {
        PyObject* pArgs = nullptr;
        if (!args.isEmpty()) {
            pArgs = PyTuple_New(args.size());
            for (auto i = 0; i < args.size(); ++i) {
                ba = args.at(i).toString().toLocal8Bit();
                const char* carg = ba.data();
                auto argValue = PyUnicode_FromString(carg);
                if (!argValue) {
                    Py_DECREF(pArgs);
                    fprintf(stderr, "Cannot convert argument: %s\n", carg);
                    return retVal;
                }
                PyTuple_SetItem(pArgs, i, argValue);
            }
        }
        retVal = PyObject_CallObject(pyFunc, pArgs);
        Py_XDECREF(pArgs);

        if (!retVal) {
            if (PyErr_Occurred()) {
                PyObject *pyType, *pyValue, *pyTraceback;
                PyErr_Fetch(&pyType, &pyValue, &pyTraceback);
                PyErr_NormalizeException(&pyType, &pyValue, &pyTraceback);

                Py_XDECREF(pyType);
                Py_XDECREF(pyTraceback);

                if (pyValue) {
                    auto pyRepr = PyObject_Repr(pyValue);
                    QString sError = PyUnicode_AsUTF8(pyRepr);
                    if (sError.contains(QLatin1String("BrowserIncorrectPassword()")))
                        throw WoobException(ExceptionCode::BrowserIncorrectPassword);
                    Py_DECREF(pyRepr);
                    Py_DECREF(pyValue);
                }
            }
        }
    }
    Py_XDECREF(pyFunc);
    return retVal;
}

QList<WoobInterface::Backend> WoobInterface::getBackends()
{
    QList<WoobInterface::Backend> backendsList;
    if (!m_woobInterface)
        return backendsList;

    auto pValue = execute("get_backends", QVariantList());
    if (pValue) {
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(pValue, &pos, &key, &value)) {
            WoobInterface::Backend backend;
            backend.name = PyUnicode_AsUTF8(key);
            backend.module = extractDictStringValue(value, "module");
            backendsList.append(backend);
        }
        Py_DECREF(pValue);
    }

    return backendsList;
}

QList<WoobInterface::Account> WoobInterface::getAccounts(QString backend)
{
    QList<WoobInterface::Account> accountsList;
    if (!m_woobInterface)
        return accountsList;

    auto pValue = execute("get_accounts", QVariantList{backend});
    if (pValue) {
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(pValue, &pos, &key, &value)) {
            WoobInterface::Account account;
            account.id = PyUnicode_AsUTF8(key);
            account.name = extractDictStringValue(value, "name");
            account.balance = MyMoneyMoney(extractDictLongValue(value, "balance"), 100);
            account.type = (WoobInterface::Account::type_t)extractDictLongValue(value, "type");

            accountsList.append(account);
        }
        Py_DECREF(pValue);
    }

    return accountsList;
}

WoobInterface::Account WoobInterface::getAccount(QString backend, QString accid, QString max)
{
    WoobInterface::Account acc;
    if (!m_woobInterface)
        return acc;

    auto retVal = execute("get_transactions", QVariantList{backend, accid, max});
    if (retVal) {
        acc.id = extractDictStringValue(retVal, "id");
        acc.name = extractDictStringValue(retVal, "name");
        acc.balance = MyMoneyMoney(extractDictLongValue(retVal, "balance"), 100);
        acc.type = (WoobInterface::Account::type_t)extractDictLongValue(retVal, "type");

        auto key = PyUnicode_FromString("transactions");
        auto val = PyDict_GetItem(retVal, key);
        if (val) {
            auto sizeVal = PyList_Size(val);
            for (auto i = 0; i < sizeVal; ++i) {
                auto val2 = PyList_GetItem(val, i);
                if (val2) {
                    WoobInterface::Transaction tr;
                    tr.id = extractDictStringValue(val2, "id");
                    tr.date = QDate::fromString(extractDictStringValue(val2, "date"), "yyyy-MM-dd");
                    tr.rdate = QDate::fromString(extractDictStringValue(val2, "rdate"), "yyyy-MM-dd");
                    tr.type = (WoobInterface::Transaction::type_t)extractDictLongValue(val2, "type");
                    tr.raw = extractDictStringValue(val2, "raw");
                    tr.category = extractDictStringValue(val2, "category");
                    tr.label = extractDictStringValue(val2, "label");
                    tr.amount = MyMoneyMoney(extractDictLongValue(val2, "amount"), 100);

                    acc.transactions.append(tr);
                }
            }
        }
        Py_DECREF(key);
        Py_DECREF(retVal);
    }
    return acc;
}

QString WoobInterface::extractDictStringValue(PyObject* pyContainer, const char* szKey)
{
    QString sVal;
    auto pyKey = PyUnicode_FromString(szKey);
    auto pyVal = PyDict_GetItem(pyContainer, pyKey);
    if (pyVal)
        sVal = PyUnicode_AsUTF8(pyVal);
    Py_DECREF(pyKey);
    return sVal;
}

long WoobInterface::extractDictLongValue(PyObject* pyContainer, const char* szKey)
{
    long sVal = 0;
    auto pyKey = PyUnicode_FromString(szKey);
    auto pyVal = PyDict_GetItem(pyContainer, pyKey);
    if (pyVal)
        sVal = PyLong_AsLong(pyVal);
    Py_DECREF(pyKey);
    return sVal;
}
