/*******************************************************************************
*                                 csvdialog.h
*                              ------------------
* begin                       : Sat Jan 01 2010
* copyright                   : (C) 2010 by Allan Anderson
* email                       : agander93@gmail.com
* copyright                   : (C) 2016 by Łukasz Wojniłowicz
* email                       : lukasz.wojnilowicz@gmail.com
********************************************************************************/

/*******************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
********************************************************************************/

#ifndef CSVDIALOG_H
#define CSVDIALOG_H

#include <QWidget>
#include <QWizard>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QComboBox>
#include <QSet>
#include <KSharedConfig>

#include <mymoneystatement.h>

class CSVWizard;

namespace Ui
{
class CSVDialog;
}

class CSVDialog : public QObject
{
  Q_OBJECT

private:

public:
  explicit CSVDialog();
  ~CSVDialog();

  CSVWizard*          m_wiz;

  typedef enum:uchar { ColumnNumber, ColumnDate, ColumnPayee, ColumnAmount,
         ColumnCredit, ColumnDebit, ColumnCategory,
         ColumnMemo, ColumnEmpty = 0xFE, ColumnInvalid = 0xFF
       } columnTypeE;

  QMap<columnTypeE, int>   m_colTypeNum;
  QMap<int ,columnTypeE>   m_colNumType;
  QMap<columnTypeE, QString> m_colTypeName;

  QStringList    m_columnList;

  QString        m_qifBuffer;

  /**
   * a list of already used hashes in this file
   */
  QSet<QString> m_hashSet;

  int            m_oppositeSigns;

  void             saveSettings();
  int              validateColumn(const int& col, columnTypeE type);

  /**
  * This method is called when the user clicks 'Clear selections', in order to
  * clear incorrect column number entries.  Also called on initialisation.
  */
  void           clearSelectedFlags();

  /**
  * This method is called when the user clicks 'Clear selections', in order to
  * clear incorrect column number entries.
  */
  void           clearColumnNumbers();

  /**
  * This method is called when the user clicks 'Clear selections'.
  * All column selections are cleared.
  */
  void           clearColumnsSelected();

  /**
  * Because the memo field allows multiple selections, it helps to be able
  * to see which columns are selected already, particularly if a column
  * selection gets deleted. This is achieved by adding a '*' character
  * after the column number of each selected column in the menu combobox.
  * This method is called to remove the '*' characters when a file is
  * reloaded, or when the user clears his selections.
  */
  void           clearComboBoxText();

  /**
  * This method is called when an input file has been selected, to clear
  * previous column selections.
  */
  void           clearColumnTypesList();

  /**
  * This method is called during processing. It ensures that processed credit/debit
  * are valid.
  */
  bool           processCreditDebit(QString& credit, QString& debit , MyMoneyMoney& amount);

  /**
  * This method feeds file buffer in banking lines parser.
  */
  bool           createStatement(MyMoneyStatement& st);

  /**
  * This method is called when the user clicks 'import'.
  * or 'Make QIF' It will evaluate a line and prepare it to be added to a statement,
  * and to a QIF file, if required.
  */
  bool           processBankLine(const QString &line, MyMoneyStatement &st);

  /**
  * This method is called after processQifLine, to add a transaction to a
  * list, ready to be imported.
  */
  void           csvImportTransaction(MyMoneyStatement& st);

  /**
  * This method is called after startup, to initialise some parameters.
  */
  void           init();

  /**
  * This method is called during file selection, to load settings from the resource file.
  */
  void           readSettings(const KSharedConfigPtr &config);

  /**
  * This method is called to reload column settings from the UI.
  */
  void           reloadUISettings();

  /**
  * This method is called when it is detected that the user has selected the
  * same column for two different fields.  The column detecting the error
  * has to reset the other column.
  */
  void           resetComboBox(columnTypeE comboBox);

  bool           validateMemoComboBox();
  bool           validateSelectedColumn(int col, columnTypeE type);


signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);
  bool           isImportable();
  void           namesEdited();

public slots:

  void           memoColumnSelected(int col);
  void           categoryColumnSelected(int col);
  void           numberColumnSelected(int col);
  void           payeeColumnSelected(int col);
  void           dateColumnSelected(int col);
  void           debitColumnSelected(int col);
  void           creditColumnSelected(int col);
  void           amountColumnSelected(int col);

  /**
  * This method is called when the user clicks 'Save as QIF'. A file selector
  * dialog is shown, where the user may select the save location.
  */
  void           slotSaveAsQIF();

private:

  void             closeEvent(QCloseEvent *event);

private slots:
  /**
  * This method is called when the user clicks 'Encoding' and selects an
  * encoding setting.  The file is re-read with the corresponding codec.
  */
  void           encodingChanged(int index);
};

#endif // CSVDIALOG_H
