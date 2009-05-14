/***************************************************************************
                          kaccounttemplateselector.h  -  description
                             -------------------
    begin                : Tue Feb 5 2008
    copyright            : (C) 2008 by Thomas Baumgart
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

#ifndef KACCOUNTTEMPLATESELECTOR_H
#define KACCOUNTTEMPLATESELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <ui_kaccounttemplateselectordecl.h>
//Added by qt3to4:
#include <Q3ValueList>
class MyMoneyTemplate;

/**
 * @author Thomas Baumgart <ipwizard@users.sourceforge.net>
 */



class KAccountTemplateSelectorDecl : public QWidget, public Ui::KAccountTemplateSelectorDecl
{
public:
  KAccountTemplateSelectorDecl( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};
class KAccountTemplateSelector : public KAccountTemplateSelectorDecl
{
  Q_OBJECT
  public:
    KAccountTemplateSelector(QWidget* parent = 0);
    ~KAccountTemplateSelector();

    Q3ValueList<MyMoneyTemplate> selectedTemplates(void) const;

  private slots:
    void slotLoadHierarchy(void);
    void slotLoadCountry(void);
    void slotLoadTemplateList(void);

  private:
    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif
