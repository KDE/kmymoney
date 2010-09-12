/***************************************************************************
                            redefine.h
                        -------------------
begin                 : Sat Jan 01 2010
copyright             : (C) 2010 by Allan Anderson
email                 : aganderson@ukonline.co.uk
****************************************************************************/

/***************************************************************************

*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

#ifndef REDEFINEDLG_H
#define REDEFINEDLG_H

#include <KDialog>

#include "ui_redefinedlgdecl.h"

#define defMAXCOL 14

class InvestmentDlg;

class RedefineDlgDecl : public QWidget, public Ui::RedefineDlgDecl
{
public:
  RedefineDlgDecl() {
    setupUi(this);
  }
};

class RedefineDlg : public KDialog
{
  Q_OBJECT

public:
  RedefineDlg();
  ~RedefineDlg();

  int              m_amountColumn;
  int              m_columnTotalWidth;
  int              m_mainHeight;
  int              m_mainWidth;
  int              m_priceColumn;
  int              m_quantityColumn;
  int              m_typeColumn;

  QString          m_accountName;
  QString          m_inBuffer;

  QStringList      m_columnList;

  /**
  * This method validates the column numbers entered by the user.  It then
  * checks the values in those columns for compatibility with the input
  * investment activity type.  If an error is detected, suspectType() is called.
  */
  int              checkValid(const QString& type, QString info);

  /**
  * This method calls buildOkTypeList() to identify the likely valid type,
  * based on the input values.  It then displays the transaction, allowing
  * the user to select the proper activity type.  This selection also is validated.
  */
  int              suspectType(QString info);

signals:
  /**
  * This method calls buildOkTypeList() to help identify the likely valid type,
  * based on the input values.  It then displays the transaction, allowing
  * the user to select the proper activity type.  This selection also is
  * validated, or, rather, controlled.
  */
  void             changedType(const QString&);

private:
  RedefineDlgDecl* m_widget;

  QPixmap          m_iconYes;
  QPixmap          m_iconNo;

  QString          m_newType;
  QStringList      m_okTypeList;

  int              m_ret;

  float            m_price;
  float            m_quantity;
  float            m_amount;

  /**
  * This method displays the transaction, highlighting the column containing the
  * dubious invewstment transaction type.  It displays a combobox, allowing the
  * user to select the proper type, based on m_okTypeList.
  */
  void             displayLine(const QString& info);

  /**
  * This method displays a dialog box, requiring the user to enter a checking/brokerage
  * account name for transfer of funds for buy, sell or dividend transactions.
  */
  QString          getParameter(QString aName);

  void             resizeEvent(QResizeEvent * event);

  /**
  * This method is called to redraw the window following input, or resizing.
  */
  void             updateWindow();

private slots:
  /**
  * This method is called when the OK button is clicked.  The new investment type
  * is added to the transaction, and signal changedType() is raised to inform
  * InvestmentDlg of the new type.
  */
  void             slotAccepted();

  /**
  * This method is called when the user selects a new investment type, which replaces the
  * original one.  The OK button is then enabled.
  */
  void             slotNewActionSelected(QString type);

  /**
  * This method is called if the user cancels the dialog, and returns KMessageBox::Cancel
  * (This return is used for compatibility with other InvestProcessing routine returns.)
  */
  void             slotRejected();

  /**
  * This method is called to identify suitable investment types, based on the combination of
  * investment parameters in the transaction.
  */
  void             buildOkTypeList();

  /**
  * This method extracts the transaction values from the columns selected by the user.
  */
  void             getValues();
};

#endif // REDEFINEDLG_H
