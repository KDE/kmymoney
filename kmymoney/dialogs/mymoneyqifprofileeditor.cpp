/***************************************************************************
                          kqifprofileeditor.cpp  -  description
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

#include "mymoneyqifprofileeditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QCheckBox>
#include <QListWidget>
#include <QTabWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kinputdialog.h>
#include <ktoolinvocation.h>

// ----------------------------------------------------------------------------
// Project Includes

MyMoneyQifProfileNameValidator::MyMoneyQifProfileNameValidator(QObject *o)
    : QValidator(o)
{
}

MyMoneyQifProfileNameValidator::~MyMoneyQifProfileNameValidator()
{
}

QValidator::State MyMoneyQifProfileNameValidator::validate(QString& name, int&) const
{
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Profiles");
  QStringList list = grp.readEntry("profiles", QStringList());

  // invalid character?
  if (name.contains(",") != 0)
    return QValidator::Invalid;

  // would not work in this form (empty or existing name)
  if (name.isEmpty() || list.contains(name))
    return QValidator::Intermediate;

  // is OK
  return QValidator::Acceptable;
}

MyMoneyQifProfileEditor::MyMoneyQifProfileEditor(const bool edit, QWidget *parent)
    : MyMoneyQifProfileEditorDecl(parent),
    m_inEdit(edit),
    m_isDirty(false),
    m_isAccepted(false),
    m_selectedAmountType(0)
{
  loadWidgets();
  loadProfileListFromConfig();

  // load button icons
  m_resetButton->setGuiItem(KStandardGuiItem::reset());
  m_cancelButton->setGuiItem(KStandardGuiItem::cancel());
  m_okButton->setGuiItem(KStandardGuiItem::ok());
  m_deleteButton->setGuiItem(KStandardGuiItem::del());
  m_helpButton->setGuiItem(KStandardGuiItem::help());

  KIconLoader* il = KIconLoader::global();
  KGuiItem newButtenItem(i18nc("New profile", "&New"),
                         KIcon(il->loadIcon("document-new", KIconLoader::Small, KIconLoader::SizeSmall)),
                         i18n("Create a new profile"),
                         i18n("Use this to create a new QIF import/export profile"));
  m_newButton->setGuiItem(newButtenItem);

  connect(m_profileListBox, SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotLoadProfileFromConfig(const QString&)));
  connect(m_resetButton, SIGNAL(clicked()), this, SLOT(slotReset()));
  connect(m_okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
  connect(m_renameButton, SIGNAL(clicked()), this, SLOT(slotRename()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(slotDelete()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNew()));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

  connect(m_editDescription, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setProfileDescription(const QString&)));
  connect(m_editType, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setProfileType(const QString&)));
  connect(m_editOpeningBalance, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setOpeningBalanceText(const QString&)));
  connect(m_editAccountDelimiter, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setAccountDelimiter(const QString&)));
  connect(m_editVoidMark, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setVoidMark(const QString&)));

  connect(m_editDateFormat, SIGNAL(highlighted(const QString&)), &m_profile, SLOT(setOutputDateFormat(const QString&)));
  connect(m_editApostrophe, SIGNAL(highlighted(const QString&)), &m_profile, SLOT(setApostropheFormat(const QString&)));

  connect(m_editAmounts, SIGNAL(itemSelectionChanged()), this, SLOT(slotAmountTypeSelected()));
  connect(m_decimalBox, SIGNAL(activated(const QString&)), this, SLOT(slotDecimalChanged(const QString&)));
  connect(m_thousandsBox, SIGNAL(activated(const QString&)), this, SLOT(slotThousandsChanged(const QString&)));

  connect(m_editInputFilterLocation, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setFilterScriptImport(const QString&)));
  connect(m_editInputFilterLocation, SIGNAL(urlSelected(const KUrl&)), m_editInputFilterLocation, SLOT(setUrl(const KUrl&)));

  connect(m_editInputFilterFileType, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setFilterFileType(const QString&)));

  connect(m_editOutputFilterLocation, SIGNAL(textChanged(const QString&)), &m_profile, SLOT(setFilterScriptExport(const QString&)));
  connect(m_editOutputFilterLocation, SIGNAL(urlSelected(const KUrl&)), m_editOutputFilterLocation, SLOT(setUrl(const KUrl&)));

  connect(m_attemptMatch, SIGNAL(toggled(bool)), &m_profile, SLOT(setAttemptMatchDuplicates(bool)));
}

MyMoneyQifProfileEditor::~MyMoneyQifProfileEditor()
{
  if (m_inEdit && m_isDirty && m_isAccepted) {
    KSharedConfigPtr config = KGlobal::config();
    config->sync();
  } else {
    slotReset();
  }
  delete tabMoney;
  delete tabDate;
}

void MyMoneyQifProfileEditor::loadWidgets(void)
{
  if (m_inEdit)
    setWindowTitle(i18n("QIF Profile Editor"));
  else
    setWindowTitle(i18n("QIF Profile Selector"));

  m_editDateFormat->clear();
  m_editDateFormat->addItem("%d/%m/%yy");
  m_editDateFormat->addItem("%d/%mmm/%yy");
  m_editDateFormat->addItem("%d/%m/%yyyy");
  m_editDateFormat->addItem("%d/%mmm/%yyyy");
  m_editDateFormat->addItem("%d/%m%yy");
  m_editDateFormat->addItem("%d/%mmm%yy");
  m_editDateFormat->addItem("%d.%m.%yy");
  m_editDateFormat->addItem("%d.%m.%yyyy");
  m_editDateFormat->addItem("%m.%d.%yy");
  m_editDateFormat->addItem("%m.%d.%yyyy");
  m_editDateFormat->addItem("%m/%d/%yy");
  m_editDateFormat->addItem("%mmm/%d/%yy");
  m_editDateFormat->addItem("%m/%d/%yyyy");
  m_editDateFormat->addItem("%m-%d-%yyyy");
  m_editDateFormat->addItem("%mmm/%d/%yyyy");
  m_editDateFormat->addItem("%m%d%yy");
  m_editDateFormat->addItem("%mmm/%d%yy");
  m_editDateFormat->addItem("%yyyy-%mm-%dd");
  m_editDateFormat->addItem("%m/%d'%yyyy");

  m_editApostrophe->clear();
  m_editApostrophe->addItem("1900-1949");
  m_editApostrophe->addItem("1900-1999");
  m_editApostrophe->addItem("2000-2099");

  m_editAmounts->setColumnHidden(4, true);
  m_editAmounts->sortItems(4, Qt::AscendingOrder);

  m_decimalBox->addItem(" ");
  m_decimalBox->addItem(",");
  m_decimalBox->addItem(".");

  m_thousandsBox->addItem(" ");
  m_thousandsBox->addItem(",");
  m_thousandsBox->addItem(".");

  m_editDescription->setEnabled(m_inEdit);
  m_editType->setEnabled(m_inEdit);
  m_editDateFormat->setEnabled(m_inEdit);
  m_editApostrophe->setEnabled(m_inEdit);
  m_editAmounts->setEnabled(m_inEdit);
  m_decimalBox->setEnabled(m_inEdit);
  m_thousandsBox->setEnabled(m_inEdit);
  m_editOpeningBalance->setEnabled(m_inEdit);
  m_editAccountDelimiter->setEnabled(m_inEdit);
  m_editVoidMark->setEnabled(m_inEdit);
  m_editInputFilterLocation->setEnabled(m_inEdit);
  m_editOutputFilterLocation->setEnabled(m_inEdit);
  m_editInputFilterFileType->setEnabled(m_inEdit);

  if (!m_inEdit) {
    m_renameButton->hide();
    m_deleteButton->hide();
    m_resetButton->hide();
    m_newButton->hide();
  }
}

void MyMoneyQifProfileEditor::loadProfileListFromConfig(void)
{
  QFontMetrics fontMetrics(m_profileListBox->font());
  int w = 100;      // minimum is 100 pixels width for the list box

  if (m_profile.isDirty()) {
    m_profile.saveProfile();
    m_isDirty = true;
  }

  m_profileListBox->clear();

  QStringList list;
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Profiles");
  list = grp.readEntry("profiles", QStringList());

  if (list.count() == 0) {
    m_profile.clear();
    m_profile.setProfileDescription(i18n("The default QIF profile"));
    addProfile("Default");

    grp = config->group("Profiles");
    list = grp.readEntry("profiles", QStringList());
  }

  list.sort();

  m_profileListBox->addItems(list);
  if (!list.isEmpty()) {
    m_profileListBox->item(0)->setSelected(true);
    slotLoadProfileFromConfig(list[0]);
  }
  for (int i = 0; i < list.count(); ++i) {
    int nw = fontMetrics.width(list[i]) + 10;
    w = qMax(w, nw);
  }
  w = qMin(w, 200);
  m_profileListBox->setMinimumWidth(w);
}

void MyMoneyQifProfileEditor::slotLoadProfileFromConfig(const QString& profile)
{
  QString profileName = profile;

  if (m_profile.isDirty()) {
    m_profile.saveProfile();
    m_isDirty = true;
  }

  if (m_profileListBox->findItems(profileName, Qt::MatchExactly | Qt::MatchCaseSensitive).empty()) {
    profileName = m_profileListBox->item(0)->text();
  }

  m_profile.loadProfile("Profile-" + profileName);

  QList<QListWidgetItem*> lbi = m_profileListBox->findItems(profileName, Qt::MatchExactly | Qt::MatchCaseSensitive);
  showProfile();
  if (!lbi.empty()) {
    foreach (QListWidgetItem* idx, lbi) {
      idx->setSelected(true);
    }
  }
}

void MyMoneyQifProfileEditor::showProfile(void)
{
  m_editDescription->setText(m_profile.profileDescription());
  m_editType->setText(m_profile.profileType());
  m_editOpeningBalance->setText(m_profile.openingBalanceText());
  m_editAccountDelimiter->setText(m_profile.accountDelimiter());
  m_editVoidMark->setText(m_profile.voidMark());
  m_editInputFilterLocation->setUrl(m_profile.filterScriptImport());
  m_editOutputFilterLocation->setUrl(m_profile.filterScriptExport());
  m_editInputFilterFileType->setText(m_profile.filterFileType());

  // load combo boxes
  int idx = m_editDateFormat->findText(m_profile.outputDateFormat());
  if (idx == -1)
    idx = 0;
  m_editDateFormat->setCurrentIndex(idx);

  idx = m_editApostrophe->findText(m_profile.apostropheFormat());
  if (idx == -1)
    idx = 0;
  m_editApostrophe->setCurrentIndex(idx);

  m_attemptMatch->setChecked(m_profile.attemptMatchDuplicates());

  QTreeWidgetItemIterator it(m_editAmounts);

  while (*it) {
    QChar key = (*it)->text(1)[0];
    (*it)->setText(2, m_profile.amountDecimal(key));
    (*it)->setTextAlignment(2, Qt::AlignCenter);
    (*it)->setText(3, m_profile.amountThousands(key));
    (*it)->setTextAlignment(3, Qt::AlignCenter);
    if (m_selectedAmountType == 0 && key == 'T' && m_inEdit) {
      (*it)->setSelected(true);
      slotAmountTypeSelected();
    } else if ((*it) == m_selectedAmountType) {
      (*it)->setSelected(true);
      slotAmountTypeSelected();
    }
    ++it;
  }
}

void MyMoneyQifProfileEditor::deleteProfile(const QString& name)
{
  KSharedConfigPtr config = KGlobal::config();

  config->deleteGroup("Profile-" + name);

  KConfigGroup grp = config->group("Profiles");
  QStringList list = grp.readEntry("profiles", QStringList());
  list.removeAll(name);

  grp.writeEntry("profiles", list);
  m_isDirty = true;
}

void MyMoneyQifProfileEditor::addProfile(const QString& name)
{
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Profiles");
  QStringList list = grp.readEntry("profiles", QStringList());

  list += name;
  list.sort();
  grp.writeEntry("profiles", list);

  m_profile.setProfileName("Profile-" + name);
  m_profile.saveProfile();

  m_isDirty = true;
}

void MyMoneyQifProfileEditor::slotOk(void)
{
  if (m_profile.isDirty())
    m_isDirty = true;

  m_profile.saveProfile();

  KSharedConfigPtr config = KGlobal::config();
  config->sync();

  m_isAccepted = true;
  accept();
}

void MyMoneyQifProfileEditor::slotReset(void)
{
  // first flush any changes
  m_profile.saveProfile();

  KSharedConfigPtr config = KGlobal::config();
  config->reparseConfiguration();

  QString currentProfile = m_profile.profileName().mid(8);
  loadProfileListFromConfig();
  slotLoadProfileFromConfig(currentProfile);
  m_isDirty = false;
}

void MyMoneyQifProfileEditor::slotRename(void)
{
  bool ok;
  QString newName = enterName(ok);

  if (ok == true) {
    deleteProfile(m_profile.profileName().mid(8));
    addProfile(newName);
    loadProfileListFromConfig();
    slotLoadProfileFromConfig(newName);
  }
}

void MyMoneyQifProfileEditor::slotNew(void)
{
  bool ok;
  QString newName = enterName(ok);

  if (ok == true) {
    m_profile.clear();
    addProfile(newName);
    loadProfileListFromConfig();
    slotLoadProfileFromConfig(newName);
  }
}

const QString MyMoneyQifProfileEditor::enterName(bool& ok)
{
  MyMoneyQifProfileNameValidator val(this);
#if KDE_IS_VERSION(3,2,0)
  return KInputDialog::getText(i18n("QIF Profile Editor"),
                               i18n("Enter new profile name"),
                               QString(),
                               &ok,
                               this,

                               &val,
                               0);
#else
  QString rc;

  // the blank in the next line as the value for the edit box is
  // there on purpose, so that with the following call to validateAndSet
  // the state is changed and the OK-Button is grayed
  QPointer<KLineEditDlg> dlg = new KLineEditDlg(i18n("Enter new profile name"), " ", this);
  dlg->lineEdit()->setValidator(&val);
  dlg->lineEdit()->validateAndSet("", 0, 0, 0);

  ok = false;
  if (dlg->exec()) {
    ok = true;
  }
  rc = dlg->lineEdit()->text();
  delete dlg;

  return rc;
#endif
}

void MyMoneyQifProfileEditor::slotDelete(void)
{
  QString profile = m_profile.profileName().mid(8);

  if (KMessageBox::questionYesNo(this, i18n("Do you really want to delete profile '%1'?", profile)) == KMessageBox::Yes) {
    QListWidgetItem* idx = m_profileListBox->currentItem();
    m_profile.saveProfile();
    deleteProfile(profile);
    loadProfileListFromConfig();
    if (!idx)
      idx = m_profileListBox->item(m_profileListBox->count() - 1);

    slotLoadProfileFromConfig(idx->text());
  }
}

void MyMoneyQifProfileEditor::slotHelp(void)
{
  KToolInvocation::invokeHelp("details.impexp.qifimp.profile");
}

void MyMoneyQifProfileEditor::slotAmountTypeSelected()
{
  QList<QTreeWidgetItem*> selectedItems = m_editAmounts->selectedItems();
  if (! selectedItems.empty()) {
    QTreeWidgetItem* item = selectedItems.at(0);
    m_decimalBox->setItemText(m_decimalBox->currentIndex(), item->text(2));
    m_thousandsBox->setItemText(m_thousandsBox->currentIndex(), item->text(3));
    m_selectedAmountType = item;
  }
}

void MyMoneyQifProfileEditor::slotDecimalChanged(const QString& val)
{
  if (m_selectedAmountType != 0) {
    QChar key = m_selectedAmountType->text(1)[0];
    m_profile.setAmountDecimal(key, val[0]);
    m_selectedAmountType->setText(2, val);
  }
}

void MyMoneyQifProfileEditor::slotThousandsChanged(const QString& val)
{
  if (m_selectedAmountType != 0) {
    QChar key = m_selectedAmountType->text(1)[0];
    m_profile.setAmountThousands(key, val[0]);
    m_selectedAmountType->setText(3, val);
  }
}

const QString MyMoneyQifProfileEditor::selectedProfile() const
{
  return m_profileListBox->currentItem()->text();
}

#include "mymoneyqifprofileeditor.moc"
