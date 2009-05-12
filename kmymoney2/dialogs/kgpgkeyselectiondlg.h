/***************************************************************************
                          kgpgkeyselectiondlg.h
                             -------------------
    copyright            : (C) 2008 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KGPGKEYSELECTIONDLG_H
#define KGPGKEYSELECTIONDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdialog.h>
class KEditListBox;
class KLed;

// ----------------------------------------------------------------------------
// Project Includes

/**
  * @author Thomas Baumgart
  */
class KGpgKeySelectionDlg : public KDialog
{
  Q_OBJECT
public:

  KGpgKeySelectionDlg(QWidget *parent=0, const char *name=0);
  virtual ~KGpgKeySelectionDlg() {}

  /**
   * preset the key list with the given key ids in @a list
   */
  void setKeys(const QStringList& list);

  /**
   * Returns the list of keys currently listed in the KEditListBox
   */
  const QStringList keys(void) const { return m_listBox->items(); }

protected slots:
  void slotIdChanged(void);
  void slotKeyListChanged(void);

private:
  KEditListBox*   m_listBox;
  KLed*           m_keyLed;
  bool            m_needCheckList;
  bool            m_listOk;
  int             m_checkCount;
};

#endif
