/*
    SPDX-FileCopyrightText: 2002 Thomas Baumgart <thb@net-bembel.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyqifprofileeditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QCheckBox>
#include <QListWidget>
#include <QTabWidget>
#include <QIcon>
#include <QInputDialog>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <kconfig.h>
#include <KConfigGroup>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <kurlrequester.h>
#include <khelpclient.h>
#include <KGuiItem>
#include <KStandardGuiItem>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes
#include <icons/icons.h>

using namespace Icons;

MyMoneyQifProfileNameValidator::MyMoneyQifProfileNameValidator(QObject *o)
    : QValidator(o)
{
}

MyMoneyQifProfileNameValidator::~MyMoneyQifProfileNameValidator()
{
}

QValidator::State MyMoneyQifProfileNameValidator::validate(QString& name, int&) const
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
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
    : QWidget(parent),
      m_inEdit(edit),
      m_isDirty(false),
      m_isAccepted(false),
      m_selectedAmountType(0)
{
    setupUi(this);
    loadWidgets();
    loadProfileListFromConfig();

    // load button icons
    KGuiItem::assign(m_resetButton, KStandardGuiItem::reset());
    KGuiItem::assign(m_deleteButton, KStandardGuiItem::del());
    KGuiItem::assign(m_helpButton, KStandardGuiItem::help());

    KGuiItem newButtenItem(i18nc("New profile", "&New"),
                           Icons::get(Icon::DocumentNew),
                           i18n("Create a new profile"),
                           i18n("Use this to create a new QIF import/export profile"));
    KGuiItem::assign(m_newButton, newButtenItem);

    connect(m_profileListBox, &QListWidget::currentTextChanged, this, &MyMoneyQifProfileEditor::slotLoadProfileFromConfig);
    connect(m_resetButton, &QAbstractButton::clicked, this, &MyMoneyQifProfileEditor::slotReset);
    connect(m_renameButton, &QAbstractButton::clicked, this, &MyMoneyQifProfileEditor::slotRename);
    connect(m_deleteButton, &QAbstractButton::clicked, this, &MyMoneyQifProfileEditor::slotDelete);
    connect(m_newButton, &QAbstractButton::clicked, this, &MyMoneyQifProfileEditor::slotNew);
    connect(m_helpButton, &QAbstractButton::clicked, this, &MyMoneyQifProfileEditor::slotHelp);

    connect(m_editDescription, &QLineEdit::textChanged, &m_profile, &MyMoneyQifProfile::setProfileDescription);
    connect(m_editType, &QLineEdit::textChanged, &m_profile, &MyMoneyQifProfile::setProfileType);
    connect(m_editOpeningBalance, &QLineEdit::textChanged, &m_profile, &MyMoneyQifProfile::setOpeningBalanceText);
    connect(m_editAccountDelimiter, &QLineEdit::textChanged, &m_profile, &MyMoneyQifProfile::setAccountDelimiter);
    connect(m_editVoidMark, &QLineEdit::textChanged, &m_profile, &MyMoneyQifProfile::setVoidMark);

    connect(m_editDateFormat, QOverload<const QString&>::of(&QComboBox::highlighted), &m_profile, &MyMoneyQifProfile::setOutputDateFormat);
    connect(m_editApostrophe, QOverload<const QString&>::of(&QComboBox::highlighted), &m_profile, &MyMoneyQifProfile::setApostropheFormat);

    connect(m_editAmounts, &QTreeWidget::itemSelectionChanged, this, &MyMoneyQifProfileEditor::slotAmountTypeSelected);
    connect(m_decimalBox, QOverload<const QString&>::of(&QComboBox::activated), this, &MyMoneyQifProfileEditor::slotDecimalChanged);
    connect(m_thousandsBox, QOverload<const QString&>::of(&QComboBox::activated), this, &MyMoneyQifProfileEditor::slotThousandsChanged);

    connect(m_editInputFilterLocation, &KUrlRequester::textChanged, &m_profile, &MyMoneyQifProfile::setFilterScriptImport);
    connect(m_editInputFilterLocation, &KUrlRequester::urlSelected, m_editInputFilterLocation, &KUrlRequester::setUrl);

    connect(m_editInputFilterFileType, &QLineEdit::textChanged, &m_profile, &MyMoneyQifProfile::setFilterFileType);

    connect(m_editOutputFilterLocation, &KUrlRequester::textChanged, &m_profile, &MyMoneyQifProfile::setFilterScriptExport);
    connect(m_editOutputFilterLocation, &KUrlRequester::urlSelected, m_editOutputFilterLocation, &KUrlRequester::setUrl);

    connect(m_attemptMatch, &QAbstractButton::toggled, &m_profile, &MyMoneyQifProfile::setAttemptMatchDuplicates);
}

MyMoneyQifProfileEditor::~MyMoneyQifProfileEditor()
{
    if (m_inEdit && m_isDirty && m_isAccepted) {
        KSharedConfigPtr config = KSharedConfig::openConfig();
        config->sync();
    } else {
        slotReset();
    }
    delete tabMoney;
    delete tabDate;
}

void MyMoneyQifProfileEditor::loadWidgets()
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

void MyMoneyQifProfileEditor::loadProfileListFromConfig()
{
    QFontMetrics fontMetrics(m_profileListBox->font());
    int w = 100;      // minimum is 100 pixels width for the list box

    if (m_profile.isDirty()) {
        m_profile.saveProfile();
        m_isDirty = true;
    }

    m_profileListBox->clear();

    QStringList list;
    KSharedConfigPtr config = KSharedConfig::openConfig();
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
        int nw = fontMetrics.horizontalAdvance(list[i]) + 10;
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

    m_profile.loadProfile(profileName);

    QList<QListWidgetItem*> lbi = m_profileListBox->findItems(profileName, Qt::MatchExactly | Qt::MatchCaseSensitive);

    if (!lbi.empty()) {
        m_profileListBox->setCurrentItem(lbi.at(0));
    }
    showProfile();
}

void MyMoneyQifProfileEditor::showProfile()
{
    m_editDescription->setText(m_profile.profileDescription());
    m_editType->setText(m_profile.profileType());
    m_editOpeningBalance->setText(m_profile.openingBalanceText());
    m_editAccountDelimiter->setText(m_profile.accountDelimiter());
    m_editVoidMark->setText(m_profile.voidMark());
    m_editInputFilterLocation->setUrl(QUrl::fromLocalFile(m_profile.filterScriptImport()));
    m_editOutputFilterLocation->setUrl(QUrl::fromLocalFile(m_profile.filterScriptExport()));
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
    KSharedConfigPtr config = KSharedConfig::openConfig();

    config->deleteGroup("Profile-" + name);

    KConfigGroup grp = config->group("Profiles");
    QStringList list = grp.readEntry("profiles", QStringList());
    list.removeAll(name);

    grp.writeEntry("profiles", list);
    m_isDirty = true;
}

void MyMoneyQifProfileEditor::addProfile(const QString& name)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp = config->group("Profiles");
    QStringList list = grp.readEntry("profiles", QStringList());

    list += name;
    list.sort();
    grp.writeEntry("profiles", list);

    m_profile.setProfileName("Profile-" + name);
    m_profile.saveProfile();

    m_isDirty = true;
}

void MyMoneyQifProfileEditor::slotReset()
{
    // first flush any changes
    m_profile.saveProfile();

    KSharedConfigPtr config = KSharedConfig::openConfig();
    config->reparseConfiguration();

    QString currentProfile = m_profile.profileName().mid(8);
    loadProfileListFromConfig();
    slotLoadProfileFromConfig(currentProfile);
    m_isDirty = false;
}

void MyMoneyQifProfileEditor::slotRename()
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

void MyMoneyQifProfileEditor::slotNew()
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
    QString rc;
    bool internalOk;

    do {
        rc = QInputDialog::getText(this, i18n("QIF Profile Editor"),
                                   i18n("Enter new profile name"),
                                   QLineEdit::Normal,
                                   rc,
                                   &internalOk);
        // if user pressed OK we check the name
        if (internalOk) {
            int pos = 0;
            if (val.validate(rc, pos) != MyMoneyQifProfileNameValidator::Acceptable) {
                QString errorMsg;
                if (rc.isEmpty()) {
                    errorMsg = i18n("The profile name cannot be empty. Please provide a name or cancel.");
                } else {
                    errorMsg = i18n("The name <b>%1</b> is already taken. Please change the name or cancel.", rc);
                }
                KMessageBox::sorry(this, errorMsg, i18n("QIF profile name problem"));
                continue;
            }
        }
        break;
    } while (true);

    ok = internalOk;
    return rc;
}

void MyMoneyQifProfileEditor::slotDelete()
{
    QString profile = m_profile.profileName().mid(8);

    if (KMessageBox::questionYesNo(this, i18n("Do you really want to delete profile '%1'?", profile)) == KMessageBox::Yes) {
        int idx = m_profileListBox->currentRow();
        m_profile.saveProfile();
        deleteProfile(profile);
        loadProfileListFromConfig();

        if (idx >= m_profileListBox->count())
            idx = m_profileListBox->count() - 1;

        m_profileListBox->setCurrentRow(idx);
        slotLoadProfileFromConfig(m_profileListBox->item(idx)->text());
    }
}

void MyMoneyQifProfileEditor::slotHelp()
{
    KHelpClient::invokeHelp("details.impexp.qifimp.profile");
}

void MyMoneyQifProfileEditor::slotAmountTypeSelected()
{
    QList<QTreeWidgetItem*> selectedItems = m_editAmounts->selectedItems();
    if (! selectedItems.empty()) {
        QTreeWidgetItem* item = selectedItems.at(0);
        m_decimalBox->setCurrentIndex(m_decimalBox->findText(item->text(2), Qt::MatchExactly));
        m_thousandsBox->setCurrentIndex(m_thousandsBox->findText(item->text(3), Qt::MatchExactly));
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
