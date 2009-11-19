/***************************************************************************
                          kqifprofileeditor.h  -  description
                             -------------------
    begin                : Tue Dec 24 2002
    copyright            : (C) 2002 by Thomas Baumgart
    email                : thb@net-bembel.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYQIFPROFILEEDITOR_H
#define MYMONEYQIFPROFILEEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <QValidator>
class Q3ListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_mymoneyqifprofileeditordecl.h"
#include "mymoneyqifprofile.h"

/**
  * @author Thomas Baumgart
  */

class MyMoneyQifProfileNameValidator : public QValidator
{
  Q_OBJECT

public:
  MyMoneyQifProfileNameValidator(QObject* o);
  virtual ~MyMoneyQifProfileNameValidator();

  QValidator::State validate(QString&, int&) const;
};


class MyMoneyQifProfileEditorDecl : public QDialog, public Ui::MyMoneyQifProfileEditorDecl
{
public:
  MyMoneyQifProfileEditorDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};

class MyMoneyQifProfileEditor : public MyMoneyQifProfileEditorDecl
{
  Q_OBJECT

public:
  explicit MyMoneyQifProfileEditor(const bool edit = false, QWidget *parent=0);
  virtual ~MyMoneyQifProfileEditor();

  /**
    * This method returns the currently selected profile in the list box.
    */
  const QString selectedProfile() const;

public slots:
  void slotOk(void);

protected slots:
  void slotLoadProfileFromConfig(const QString& name);
  void slotReset(void);
  void slotRename(void);
  void slotDelete(void);
  void slotNew(void);
  void slotAmountTypeSelected(Q3ListViewItem*);
  void slotDecimalChanged(const QString& val);
  void slotThousandsChanged(const QString& val);
  void slotHelp(void);

private:
  void loadProfileListFromConfig(void);
  void loadWidgets(void);
  void showProfile(void);
  void addProfile(const QString& name);
  void deleteProfile(const QString& name);
  const QString enterName(bool& ok);

private:
  bool                m_inEdit;
  MyMoneyQifProfile   m_profile;
  bool                m_isDirty;
  bool                m_isAccepted;
  Q3ListViewItem*      m_selectedAmountType;
};

#endif
