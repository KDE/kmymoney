//Added by qt3to4:
#include <QResizeEvent>
/***************************************************************************
                          kenterscheduledlg.h  -  description
                             -------------------
    begin                : Sat Apr  7 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KENTERSCHEDULEDLG_H
#define KENTERSCHEDULEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class TransactionEditor;

#include "../dialogs/kenterscheduledlgdecl.h"
#include <kmymoney/mymoneyscheduled.h>
#include <kmymoney/kmymoneyutils.h>

/**
  * @author Thomas Baumgart
  */
class KEnterScheduleDlg : public KEnterScheduleDlgDecl
{
  Q_OBJECT
public:
  KEnterScheduleDlg(QWidget *parent, const MyMoneySchedule& schedule);
  ~KEnterScheduleDlg();

  TransactionEditor* startEdit(void);
  MyMoneyTransaction transaction(void);

  /**
   * Show (or hide) the extended dialog keys for 'Skip' and 'Ignore'
   * depending on the value of the parameter @a visible which defaults
   * to @a true.
   */
  void showExtendedKeys(bool visible = true);

  /**
   * Return the extended result code. Usage of the returned
   * value only makes sense, once the dialog has been executed.
   * Before execution it returns @a Cancel.
   */
  KMyMoneyUtils::EnterScheduleResultCodeE resultCode(void) const;

protected:
  /// Overridden for internal reasons. No API changes.
  bool focusNextPrevChild(bool next);

  /**
    * This method returns the adjusts @a _date according to
    * the setting of the schedule's weekend option.
    */
  QDate date(const QDate& _date) const;

  void resizeEvent(QResizeEvent* ev);

public slots:
  int exec(void);

private slots:
  void slotSetupSize(void);
  void slotShowHelp(void);
  void slotIgnore(void);
  void slotSkip(void);

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

#endif
