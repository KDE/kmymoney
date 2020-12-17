/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
 * (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "weboobinterface.h"

//Python uses slots var that is QT macro
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")

// ----------------------------------------------------------------------------
// QT Includes

#include <QMutexLocker>
#include <QStandardPaths>
#include <QFileInfo>
#include <QLibrary>
#include <QVariant>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../weboobexc.h"

WeboobInterface::WeboobInterface() :
  m_weboobInterface(nullptr)
{
  const auto fileInfo = QFileInfo(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kmymoney/weboob/kmymoneyweboob.py"));

  // begin of a hack (read more on https://bugs.kde.org/show_bug.cgi?id=335965)
  auto pythonLib = new QLibrary();
  pythonLib->setLoadHints(QLibrary::ExportExternalSymbolsHint);
  pythonLib->setFileName(LIB_PYTHON);
  pythonLib->load();
  // end of a hack

  Py_Initialize();

  auto script = QString::fromLatin1("import sys\n"
                                    "sys.path.append('%1')\n").arg(fileInfo.absolutePath());
  auto ba = script.toLocal8Bit();
  const char *cscript = ba.data();
  PyRun_SimpleString(cscript);
  ba = fileInfo.baseName().toLocal8Bit();
  const char *sBaseName = ba.data();
  auto pyName = PyUnicode_FromString(sBaseName);
  m_weboobInterface = PyImport_Import(pyName);  // this will be nullptr if no hack
  Py_DECREF(pyName);

}

WeboobInterface::~WeboobInterface()
{
  if (m_weboobInterface)
    Py_DECREF(m_weboobInterface);

  if (Py_IsInitialized())
    Py_Finalize();
}

PyObject* WeboobInterface::execute(QString method, QVariantList args)
{
  QMutex mutex;
  QMutexLocker locker(&mutex);

  PyObject* retVal = nullptr;
  auto ba = method.toLocal8Bit();
  const char *cmethod = ba.data();
  auto pyFunc = PyObject_GetAttrString(m_weboobInterface, cmethod);
  if (pyFunc && PyCallable_Check(pyFunc)) {
    PyObject* pArgs = nullptr;
    if (!args.isEmpty()) {
      pArgs = PyTuple_New(args.size());
      for (auto i = 0; i < args.size(); ++i) {
        ba = args.at(i).toString().toLocal8Bit();
        const char *carg = ba.data();
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
            throw WeboobException(ExceptionCode::BrowserIncorrectPassword);
          Py_DECREF(pyRepr);
          Py_DECREF(pyValue);
        }
      }
    }
  }
  Py_XDECREF(pyFunc);
  return retVal;
}

QList<WeboobInterface::Backend> WeboobInterface::getBackends()
{
  QList<WeboobInterface::Backend> backendsList;
  if(!m_weboobInterface)
    return backendsList;

  auto pValue = execute("get_backends", QVariantList());
  if (pValue) {
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(pValue, &pos, &key, &value)) {
      WeboobInterface::Backend backend;
      backend.name = PyUnicode_AsUTF8(key);
      backend.module = extractDictStringValue(value, "module");
      backendsList.append(backend);
    }
    Py_DECREF(pValue);
  }

  return backendsList;
}


QList<WeboobInterface::Account> WeboobInterface::getAccounts(QString backend)
{
  QList<WeboobInterface::Account> accountsList;
  if(!m_weboobInterface)
    return accountsList;

  auto pValue = execute("get_accounts", QVariantList{backend});
  if (pValue) {
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(pValue, &pos, &key, &value)) {

      WeboobInterface::Account account;
      account.id = PyUnicode_AsUTF8(key);
      account.name = extractDictStringValue(value, "name");
      account.balance = MyMoneyMoney(extractDictLongValue(value, "balance"), 100);
      account.type = (WeboobInterface::Account::type_t)extractDictLongValue(value, "type");

      accountsList.append(account);
    }
    Py_DECREF(pValue);
  }

  return accountsList;
}

WeboobInterface::Account WeboobInterface::getAccount(QString backend, QString accid, QString max)
{
  WeboobInterface::Account acc;
  if(!m_weboobInterface)
    return acc;

  auto retVal = execute("get_transactions", QVariantList{backend, accid, max});
  if (retVal) {
    acc.id = extractDictStringValue(retVal, "id");
    acc.name = extractDictStringValue(retVal, "name");
    acc.balance = MyMoneyMoney(extractDictLongValue(retVal, "balance"), 100);
    acc.type = (WeboobInterface::Account::type_t)extractDictLongValue(retVal, "type");

    auto key = PyUnicode_FromString("transactions");
    auto val = PyDict_GetItem(retVal, key);
    if (val) {
      auto sizeVal = PyList_Size(val);
      for (auto i = 0 ; i < sizeVal; ++i) {
        auto val2 = PyList_GetItem(val, i);
        if (val2) {
          PyObject *key3, *val3;
          Py_ssize_t pos3 = 0;
          while (PyDict_Next(val2, &pos3, &key3, &val3)) {
            WeboobInterface::Transaction tr;
            tr.id = extractDictStringValue(val3, "id");
            tr.date = QDate::fromString(extractDictStringValue(val3, "date"), "yyyy-MM-dd");
            tr.rdate = QDate::fromString(extractDictStringValue(val3, "rdate"), "yyyy-MM-dd");
            tr.type = (WeboobInterface::Transaction::type_t)extractDictLongValue(val3, "type");;
            tr.raw = extractDictStringValue(val3, "raw");
            tr.category = extractDictStringValue(val3, "category");
            tr.label = extractDictStringValue(val3, "label");
            tr.amount = MyMoneyMoney(extractDictLongValue(val3, "amount"), 100);

            acc.transactions.append(tr);
          }
        }

      }
    }
    Py_DECREF(key);
    Py_DECREF(retVal);
  }
  return acc;
}

QString WeboobInterface::extractDictStringValue(PyObject* pyContainer, const char* szKey)
{
  QString sVal;
  auto pyKey = PyUnicode_FromString(szKey);
  auto pyVal = PyDict_GetItem(pyContainer, pyKey);
  if (pyVal)
    sVal = PyUnicode_AsUTF8(pyVal);
  Py_DECREF(pyKey);
  return sVal;
}

long WeboobInterface::extractDictLongValue(PyObject* pyContainer, const char* szKey)
{
  long sVal = 0;
  auto pyKey = PyUnicode_FromString(szKey);
  auto pyVal = PyDict_GetItem(pyContainer, pyKey);
  if (pyVal)
    sVal = PyLong_AsLong(pyVal);
  Py_DECREF(pyKey);
  return sVal;
}
