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
#include <QUrl>

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
  struct csvSplit {
    QString      m_strCategoryName;
    QString      m_strMemo;
    QString      m_amount;
  } m_csvSplit;

public:
  explicit CSVDialog();
  ~CSVDialog();

  CSVWizard*          m_wiz;

  struct qifData {
    QString number;
    QDate   date;
    QString payee;
    QString amount;
    QString memo;
    QString category;
    QString id;
  } m_trData;

  QList<MyMoneyStatement> statements;

  QStringList    m_shrsinList;
  QStringList    m_divXList;
  QStringList    m_brokerageList;
  QStringList    m_reinvdivList;
  QStringList    m_buyList;
  QStringList    m_sellList;
  QStringList    m_removeList;
  QStringList    m_columnList;
  QStringList    m_securityList;
  QStringList    m_typeOfFile;
  QStringList    m_columnTypeList;  //  holds field types - date, payee, etc.

  QString        m_csvPath;
  QString        m_inFileName;
  QString        m_lastFileName;
  QString        m_outBuffer;
  QString        m_qifBuffer;
  QString        inFileName();

  /**
   * a list of already used hashes in this file
   */
  QMap<QString, bool> m_hashMap;

  bool           m_goBack;
  bool           m_isTableTrimmed;
  bool           m_firstPass;
  bool           m_firstRead;
  bool           m_firstField;
  bool           m_amountSelected;
  bool           m_creditSelected;
  bool           m_dateSelected;
  bool           m_debitSelected;
  bool           m_memoSelected;
  bool           m_payeeSelected;
  bool           m_numberSelected;
  bool           m_categorySelected;
  bool           m_closing;

  int            m_debitFlag;
  int            m_flagCol;

  QUrl           m_url;

  void             saveSettings();

  QString          columnType(int column);
  QString          currentUI();

  void             clearPreviousColumn();
  void             setPreviousColumn(int);
  void             setCurrentUI(QString);

  int              amountColumn() const;
  void             setAmountColumn(int);
  int              debitColumn() const;
  void             setDebitColumn(int);
  int              creditColumn() const;
  void             setCreditColumn(int);
  int              dateColumn() const;
  void             setDateColumn(int);
  int              payeeColumn() const;
  void             setPayeeColumn(int);
  int              numberColumn() const;
  void             setNumberColumn(int);
  int              memoColumn() const;
  void             setMemoColumn(int);
  int              categoryColumn() const;
  void             setCategoryColumn(int);
  int              oppositeSignsCheckBox() const;
  void             setOppositeSignsCheckBox(int);
  int              validateColumn(const int& col, QString& type);

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

  int            columnNumber(const QString& msg);

  /**
  * This method feeds file buffer in banking lines parser.
  */
  void           createStatement();

  /**
  * This method is called when an input file has been selected.
  * It will enable the UI elements for column selection.
  */
  void           enableInputs();

  /**
  * This method is called when a date cannot be recognised  and the user
  * cancels the statement import. It will disable the UI elements for column
  * selection.
  */
  void           disableInputs();

  /**
  * This method is called when the user clicks 'import'.
  * It will evaluate a line and prepare it to be added to a statement,
  * and to a QIF file, if required.
  */
  int            processQifLine(QString& iBuff);

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
  void           readSettings();

  /**
  * This method is called to reload column settings from the UI.
  */
  void           reloadUISettings();

  void           showStage();

  void           clearColumnTypeList();

  int            endColumn();
  bool           importNow();

signals:
  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);
  bool           isImportable();
  void           namesEdited();
  void           valueChanged(int);

public slots:
  void           slotBackButtonClicked();

  /**
  * This method is called when the user clicks 'Open File', and opens
  * a file selector dialog.
  */
  void           slotFileDialogClicked();

  /**
  * This method is called when the user clicks 'import'. It performs further
  * validity checks on the user's choices, then redraws the required rows.
  * Finally, it rereads the file, which this time will result in the import
  * actually taking place.
  */
  void           slotImportClicked();

  /**
  * This method is called when the user clicks 'Save as QIF'. A file selector
  * dialog is shown, where the user may select the save location.
  */
  void           slotSaveAsQIF();

private:
  /**
  * Clear an invalid debit or credit field and return
  * pointer to the valid field.
  */
  QString          clearInvalidField(QString, QString);
  QString          m_currentUI;
  QString          m_previousType;
  QString          m_firstValue;
  QString          m_secondValue;
  QString          m_firstType;
  QString          m_secondType;

  bool             m_clearAll;
  bool             m_firstIsValid;
  bool             m_secondIsValid;

  int              m_amountColumn;
  int              m_creditColumn;
  int              m_debitColumn;
  int              m_numberColumn;
  int              m_payeeColumn;
  int              m_categoryColumn;
  int              m_previousColumn;
  int              m_oppositeSigns;
  int              m_curId;
  int              m_lastId;
  int              m_lineNum;
  int              m_memoColCopy;
  int              m_lastHeight;
  int              m_round;
  int              m_minimumHeight;
  int              m_windowWidth;
  int              m_rectWidth;

  void             closeEvent(QCloseEvent *event);

   /**
  * Check that the debit and credit field combination
  * is valid.
  */
  int              ensureBothFieldsValid(int);

private slots:
  /**
  * This method is called when the user clicks 'Encoding' and selects an
  * encoding setting.  The file is re-read with the corresponding codec.
  */
  void           encodingChanged(int index);
};

#endif // CSVDIALOG_H
