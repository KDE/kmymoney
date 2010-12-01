/***************************************************************************
*                          csvprocessing.h
*                          ---------------
* begin                  : Sat Jan 01 2010
* copyright            : (C) 2010 by Allan Anderson
* email                : aganderson@ukonline.co.uk
****************************************************************************/

/***************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

#ifndef CSVPROCESSING_H
#define CSVPROCESSING_H

// ----------------------------------------------------------------------------
// QT Headers

#include <QtCore/QFile>
#include <QtCore/QDate>
// ----------------------------------------------------------------------------
// KDE Headers
#include <KUrl>
// ----------------------------------------------------------------------------
// Project Headers

#include "ui_csvimporterdlgdecl.h"
#include "csvimporterplugin.h"

class CsvImporterDlgDecl;
class MyMoneyStatement;
class ParseLine;

#define MAXCOL 14    //                 maximum no. of columns (arbitrary value)

class CsvProcessing: public QObject
{
  Q_OBJECT

public:

  CsvProcessing();
  ~CsvProcessing();

  CsvImporterDlg*   m_csvDialog;
  ParseLine*        m_parseline ;

  /**
  * This method is called after startup, to initialise some parameters.
  */
  void           init();

  /**
  * This method is called to redraw the window according to the number of
  * columns and rows to be displayed.
  */
  void           updateScreen();

  QString        csvPath();
  QString        inFileName();

  int            fieldDelimiterIndex();
  int            endColumn();

signals:

  /**
  * This signal is raised when the plugin has completed a transaction.  This
  * then needs to be processed by MyMoneyStatement.
  */
  void           statementReady(MyMoneyStatement&);

public slots:
  /**
  * This method is called when the user clicks 'Open File', and opens
  * a file selector dialog.
  */
  void           fileDialog();

  /**
  * This method is called when the user selects a new field delimiter.  The
  * input file is reread using the current delimiter.
  */
  void           fieldDelimiterChanged();

  /**
  * This method is called when the user clicks 'import'. It performs further
  * validity checks on the user's choices, then redraws the required rows.
  * Finally, it rereads the file, which this time will result in the import
  * actually taking place.
  */
  void           importClicked(bool checked);

  /**
  * This method is called when the user clicks 'Date format' and selects a
  * format, which is used by convertDate().
  */
  void           dateFormatSelected(int dF);

  /**
  * This method is called when the user clicks 'Save as QIF'. A file selector
  * dialog is shown, where the user may select the save location.
  */
  void           saveAs();

  /**
  * This method is called when the user selects the start line.  The requested
  * start line  value is saved.
  */
  void           startLineChanged();

  /**
  * This method is called when the user selects the end line.  The requested
  * end line  value is saved, to be used on import.
  */
  void           endLineChanged();

private:
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
  * This method is called on opening the plugin.
  * It will populate a list with all available codecs.
  */
  void           findCodecs();

  /**
  * This method is called initially after an input file has been selected.
  * It will call other routines to display file content and to complete the
  * statement import. It will also be called to reposition the file after row
  * deletion, or to reread following encoding or delimiter change.
  */
  void           readFile(const QString& fname, int skipLines);

  /**
  * This method is called on opening, to load settings from the resource file.
  */
  void           readSettings();

  /**
  * This method is called on opening the input file.
  * It will display a line in the UI table widget.
  */
  void           displayLine(const QString& data);

  /**
  * This method is called when the user clicks 'import'.
  * It will evaluate a line and prepare it to be added to a statement,
  * and to a QIF file, if required.
  */
  int           processQifLine(QString& iBuff);

  /**
  * This method is called on opening the plugin.
  * It will add all codec names to the encoding combobox.
  */
  void           setCodecList(const QList<QTextCodec *> &list);

  /**
  * This method is called after processQifLine, to add a transaction to a
  * list, ready to be imported.
  */
  void           csvImportTransaction(MyMoneyStatement& st);

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
  * Because the memo field allows multiple selections, it helps to be able
  * to see which columns are selected already, particularly if a column
  * selection gets deleted. This is achieved by adding a '*' character
  * after the column number of each selected column in the menu combobox.
  * This method is called to remove the '*' characters when a file is
  * reloaded, or when the user clears his selections.
  */
  void           clearComboBoxText();

  QString        m_csvPath;
  QString        m_fieldDelimiterCharacter;
  QString        m_inFileName;

  struct qifData {
    QString number;
    QDate   date;
    QString payee;
    QString amount;
    QString memo;
  } m_trData;

  QList<MyMoneyStatement> statements;
  QList<QTextCodec *>     m_codecs;

  QStringList    m_dateFormats;
  QStringList    m_columnList;

  QString        m_date;
  QString        m_filename;
  QString        m_inBuffer;
  QString        m_outBuffer;

  bool           m_importNow;

  int            m_dateFormatIndex;
  int            m_endLine;
  int            m_fieldDelimiterIndex;
  int            m_endColumn;
  int            m_maxWidth;
  int            m_row;
  int            m_startLine;
  int            m_width;

  KUrl           m_url;
  QFile*         m_inFile;

private slots:
  /**
  * This method is called when the user clicks 'Encoding' and selects an
  * encoding setting.  The file is re-read with the corresponding codec.
  */
  void           encodingChanged();

  /**
  * This method is called when the user clicks 'Clear selections'.
  * All column selections are cleared.
  */
  void           clearColumnsSelected();
};
#endif // CSVPROCESSING_H
