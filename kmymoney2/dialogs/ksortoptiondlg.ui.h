/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kstdguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes


void KSortOptionDlg::init()
{
  m_okButton->setGuiItem(KStandardGuiItem::ok());
  m_cancelButton->setGuiItem(KStandardGuiItem::cancel());
  m_helpButton->setGuiItem(KStandardGuiItem::help());
}

void KSortOptionDlg::setSortOption(const QString& option, const QString& def)
{
  if(option.isEmpty()) {
    m_sortOption->setSettings(def);
    m_useDefault->setChecked(true);
  } else {
    m_sortOption->setSettings(option);
    m_useDefault->setChecked(false);
  }
}

QString KSortOptionDlg::sortOption(void) const
{
  QString rc;
  if(!m_useDefault->isChecked()) {
    rc = m_sortOption->settings();
  }
  return rc;
}

void KSortOptionDlg::hideDefaultButton(void)
{
  m_useDefault->hide();
}
 
