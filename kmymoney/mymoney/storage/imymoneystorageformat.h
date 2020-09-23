/*
 * Copyright 2002       Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2002       Kevin Tambascio <ktambascio@users.sourceforge.net>
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
    Writing = 1           /**< version to be used when writing a file */
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
