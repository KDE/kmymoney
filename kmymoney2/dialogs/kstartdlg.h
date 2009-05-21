
/***************************************************************************
                          kstartdlg.h  -  description
                             -------------------
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSTARTDLG_H
#define KSTARTDLG_H

#include "config-kmymoney.h"


#include <klocale.h>
#include <kfontdialog.h>
#include <kurlrequester.h>
#include <k3iconview.h>

#include <qstring.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <Q3Frame>
#include <KPageDialog>

/**KMyMoney 2 start dialog
  */

class KStartDlg : public KPageDialog  {
   Q_OBJECT
public:
	KStartDlg( QWidget *parent=0, const char *name=0, bool modal=true );
	virtual ~KStartDlg();
  bool isNewFile(void)  const        { return isnewfile;           }
  bool isOpenFile(void) const        { return !kurlrequest->url().isEmpty();          }
  const QString getURL(void) const { return kurlrequest->url().path(); }
  QString getTemplateName(void) const { return templatename;    }

private: // Private methods
  QString m_filename;
	bool fileExists(KUrl url);

  void setPage_Template();
  void setPage_Documents();
  /** misc widgets */
  /** Write config window */
  void writeConfig();
  /** Read config window */
  void readConfig();
  K3IconView *view_wizard;
  K3IconView *view_recent;
  KUrlRequester *kurlrequest;
  /** misc variables */
  bool isnewfile;
  bool isopenfile;
  QString templatename;
  Q3VBox *templateMainFrame;
  Q3Frame *recentMainFrame;

protected slots:
  /** No descriptions */
  void slotOk();
private slots:
  void slotTemplateClicked(Q3IconViewItem *item);
  /** slot to recent view */
  void slotRecentClicked(Q3IconViewItem *item);

  /** Handle selections */
  void slotTemplateSelectionChanged(Q3IconViewItem* item);
  void slotRecentSelectionChanged(Q3IconViewItem* item);
  void slotAboutToShowPage(QWidget* page);
};

#endif
