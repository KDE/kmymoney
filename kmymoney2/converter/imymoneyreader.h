  /***************************************************************************
                          imymoneyreader.h  -  description
                             -------------------
    begin                : Wed Feb 25 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IMYMONEYREADER_H
#define IMYMONEYREADER_H

// ----------------------------------------------------------------------------
// QT Headers

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <ktemporaryfile.h>
#include <k3process.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "../mymoney/mymoneyaccount.h"

/**
  * @author Kevin Tambascio
  */

class IMyMoneyReader : public QObject
{
public:
  IMyMoneyReader() {}
  virtual ~IMyMoneyReader() {}
  
  Q_OBJECT
	
	/**
    * This method is used to store the filename into the object.
    * The file should exist. If it does and an external filter
    * program is specified with the current selected profile,
    * the file is send through this filter and the result
    * is stored in the m_tempFile file.
    *
    * @param name path and name of the file to be imported
    */
  virtual void setFilename(const QString& name)=0;

  /**
    * This method is used to store the name of the profile into the object.
    * The selected profile will be loaded if it exists. If an external
    * filter program is specified with the current selected profile,
    * the file is send through this filter and the result
    * is stored in the m_tempFile file.
    *
    * @param name QString reference to the name of the profile
    */
  virtual void setProfile(const QString& name)=0;

  /**
    * This method actually starts the import of data from the selected file
    * into the MyMoney engine.
    *
    * This method also starts the user defined import filter program
    * defined in the QIF profile(when a QIF file is selected). If none is
    * defined, the file is read as is (actually the UNIX command
    * 'cat -' is used as the filter).
    *
    * If data from the filter program is available, the slot
    * slotReceivedDataFromFilter() will be called.
    *
    * Make sure to connect the signal importFinished() to detect when
    * the import actually ended. Call the method finishImport() to clean
    * things up and get the overall result of the import.
    *
    * @retval true the import was started successfully
    * @retval false the import could not be started.
    */
  virtual const bool startImport(void)=0;

  /**
    * This method must be called once the signal importFinished() has
    * been emitted. It will clean up the reader state and determines
    * the actual return code of the import.
    *
    * @retval true Import was successful.
    * @retval false Import failed because the filter program terminated
    *               abnormally or the user aborted the import process.
    */
  virtual const bool finishImport(void)=0;

  /**
    * This method is used to modify the auto payee creation flag.
    * If this flag is set, records for payees that are not currently
    * found in the engine will be automatically created with no
    * further user interaction required. If this flag is no set,
    * the user will be asked if the payee should be created or not.
    * If the MyMoneyQifReader object is created auto payee creation
    * is turned off.
    *
    * @param create flag if this feature should be turned on (@p true)
    *               or turned off (@p false)
    */
  virtual void setAutoCreatePayee(const bool create)=0;
  virtual void setAskPayeeCategory(const bool ask)=0;
  
  virtual const MyMoneyAccount& account() const { return m_account; };
  virtual void setProgressCallback(void(*callback)(int, int, const QString&)) { m_progressCallback = callback; }
	
private:
	MyMoneyAccount          m_account;
	void (*m_progressCallback)(int, int, const QString&);
	QString                 m_filename;
		
};

#endif
