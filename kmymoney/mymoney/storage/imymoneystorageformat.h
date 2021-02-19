/*
    SPDX-FileCopyrightText: 2002 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMYMONEYSTORAGEFORMAT_H
#define IMYMONEYSTORAGEFORMAT_H


/**
  * @author Kevin Tambascio (ktambascio@yahoo.com)
  */

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

class QString;
class QIODevice;

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyStorageMgr;
class MyMoneyFile;

class KMM_MYMONEY_EXPORT IMyMoneyOperationsFormat
{
public:
  IMyMoneyOperationsFormat();
  virtual ~IMyMoneyOperationsFormat();

  enum fileVersionDirectionType {
    Reading = 0,          /**< version of file to be read */
    Writing = 1,          /**< version to be used when writing a file */
  };

/// @todo port to new model code
  virtual void readFile(QIODevice* qf, MyMoneyFile* file) = 0;
  // virtual void readStream(QDataStream& s, IMyMoneySerialization* storage) = 0;

  virtual void writeFile(QIODevice* qf, MyMoneyFile* file) = 0;
  //virtual void writeStream(QDataStream& s, IMyMoneySerialization* storage) = 0;

  virtual void setProgressCallback(void(*callback)(int, int, const QString&)) = 0;
  /**
    * This member is used to store the file version information
    * obtained while reading a file.
    */
  static unsigned int fileVersionRead;

  /**
    * This member is used to store the file version information
    * to be used when writing a file.
    */
  static unsigned int fileVersionWrite;
};

#endif
