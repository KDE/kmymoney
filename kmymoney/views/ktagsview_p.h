/*
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTAGSVIEW_P_H
#define KTAGSVIEW_P_H

#include "ktagsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KListWidgetSearchLine>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ktagsview.h"

#include "kmymoneyviewbase_p.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneytag.h"
#include "mymoneytransactionfilter.h"
#include "icons.h"
#include "viewenums.h"
#include "widgetenums.h"

using namespace Icons;
namespace Ui {
class KTagsView;
}

// *** KTagListItem Implementation ***

/**
  * This class represents an item in the tags list view.
  */
class KTagListItem : public QListWidgetItem
{
public:
    /**
      * Constructor to be used to construct a tag entry object.
      *
      * @param parent pointer to the QListWidget object this entry should be
      *               added to.
      * @param tag    const reference to MyMoneyTag for which
      *               the QListWidget entry is constructed
      */
    explicit KTagListItem(QListWidget *parent, const MyMoneyTag& tag) :
        QListWidgetItem(parent, QListWidgetItem::UserType),
        m_tag(tag)
    {
        setText(tag.name());
        // allow in column rename
        setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    ~KTagListItem() {}

    MyMoneyTag tag() const
    {
        return m_tag;
    }

private:
    MyMoneyTag  m_tag;
};

class KTagsViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KTagsView)

public:
    explicit KTagsViewPrivate(KTagsView *qq) :
        q_ptr(qq),
        ui(new Ui::KTagsView),
        m_needLoad(true),
        m_searchWidget(nullptr),
        m_inSelection(false),
        m_allowEditing(true),
        m_tagFilterType(0)
    {
    }

    ~KTagsViewPrivate() override
    {
        if (!m_needLoad) {
            // remember the splitter settings for startup
            KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
            grp.writeEntry("KTagsViewSplitterSize", ui->m_splitter->saveState());
            grp.sync();
        }
        delete ui;
    }

    void init()
    {
        Q_Q(KTagsView);
        m_needLoad = false;
        ui->setupUi(q);

        // create the searchline widget
        // and insert it into the existing layout
        m_searchWidget = new KListWidgetSearchLine(q, ui->m_tagsList);
        m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        ui->m_tagsList->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->m_listTopHLayout->insertWidget(0, m_searchWidget);

        //load the filter type
        ui->m_filterBox->addItem(i18nc("@item Show all tags", "All"));
        ui->m_filterBox->addItem(i18nc("@item Show only used tags", "Used"));
        ui->m_filterBox->addItem(i18nc("@item Show only unused tags", "Unused"));
        ui->m_filterBox->addItem(i18nc("@item Show only opened tags", "Opened"));
        ui->m_filterBox->addItem(i18nc("@item Show only closed tags", "Closed"));
        ui->m_filterBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        ui->m_newButton->setIcon(Icons::get(Icon::ListAddTag));
        ui->m_renameButton->setIcon(Icons::get(Icon::EditRename));
        ui->m_deleteButton->setIcon(Icons::get(Icon::ListRemoveTag));
        ui->m_updateButton->setIcon(Icons::get(Icon::DialogOK));
        ui->m_updateButton->setEnabled(false);

        ui->m_register->setupRegister(MyMoneyAccount(),
                                      QList<eWidgets::eTransaction::Column> { eWidgets::eTransaction::Column::Date,
                                              eWidgets::eTransaction::Column::Account,
                                              eWidgets::eTransaction::Column::Detail,
                                              eWidgets::eTransaction::Column::ReconcileFlag,
                                              eWidgets::eTransaction::Column::Payment,
                                              eWidgets::eTransaction::Column::Deposit
                                                                            });
        ui->m_register->setSelectionMode(QTableWidget::SingleSelection);
        ui->m_register->setDetailsColumnType(eWidgets::eRegister::DetailColumn::AccountFirst);
        ui->m_balanceLabel->hide();

        q->connect(ui->m_tagsList, &QListWidget::currentItemChanged, q, static_cast<void (KTagsView::*)(QListWidgetItem *, QListWidgetItem *)>(&KTagsView::slotSelectTag));
        q->connect(ui->m_tagsList, &QListWidget::itemSelectionChanged, q, static_cast<void (KTagsView::*)()>(&KTagsView::slotSelectTag));
        q->connect(ui->m_tagsList, &QListWidget::itemDoubleClicked, q, &KTagsView::slotStartRename);
        q->connect(ui->m_tagsList, &QListWidget::itemChanged, q, &KTagsView::slotRenameSingleTag);
        q->connect(ui->m_tagsList, &QWidget::customContextMenuRequested, q, &KTagsView::slotShowTagsMenu);

        q->connect(ui->m_newButton,    &QAbstractButton::clicked, q, &KTagsView::slotNewTag);
        q->connect(ui->m_renameButton, &QAbstractButton::clicked, q, &KTagsView::slotRenameTag);
        q->connect(ui->m_deleteButton, &QAbstractButton::clicked, q, &KTagsView::slotDeleteTag);

        q->connect(ui->m_colorbutton, &KColorButton::changed,   q, &KTagsView::slotTagDataChanged);
        q->connect(ui->m_closed,      &QCheckBox::stateChanged, q, &KTagsView::slotTagDataChanged);
        q->connect(ui->m_notes,       &QTextEdit::textChanged,  q, &KTagsView::slotTagDataChanged);

        q->connect(ui->m_updateButton, &QAbstractButton::clicked, q, &KTagsView::slotUpdateTag);
        q->connect(ui->m_helpButton, &QAbstractButton::clicked,   q, &KTagsView::slotHelp);

        q->connect(ui->m_register, &KMyMoneyRegister::Register::editTransaction, q, &KTagsView::slotSelectTransaction);

        q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KTagsView::refresh);

        q->connect(ui->m_filterBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), q, &KTagsView::slotChangeFilter);

        // use the size settings of the last run (if any)
        auto grp = KSharedConfig::openConfig()->group("Last Use Settings");
        ui->m_splitter->restoreState(grp.readEntry("KTagsViewSplitterSize", QByteArray()));
        ui->m_splitter->setChildrenCollapsible(false);

        // At start we haven't any tag selected
        ui->m_tabWidget->setEnabled(false); // disable tab widget
        ui->m_deleteButton->setEnabled(false); // disable delete and rename button
        ui->m_renameButton->setEnabled(false);
        m_tag = MyMoneyTag(); // make sure we don't access an undefined tag
        q->clearItemData();
    }

    /**
      * Check if a list contains a tag with a given id
      *
      * @param list const reference to value list
      * @param id const reference to id
      *
      * @retval true object has been found
      * @retval false object is not in list
      */
    bool tagInList(const QList<MyMoneyTag>& list, const QString& id) const
    {
        bool rc = false;
        QList<MyMoneyTag>::const_iterator it_p = list.begin();
        while (it_p != list.end()) {
            if ((*it_p).id() == id) {
                rc = true;
                break;
            }
            ++it_p;
        }
        return rc;
    }

    KTagsView       *q_ptr;
    Ui::KTagsView   *ui;

    MyMoneyTag   m_tag;
    QString      m_newName;

    /**
        * This member holds a list of all transactions
        */
    QList<QPair<MyMoneyTransaction, MyMoneySplit> > m_transactionList;

    /**
        * This member holds the load state of page
        */
    bool m_needLoad;

    /**
        * Search widget for the list
        */
    KListWidgetSearchLine*  m_searchWidget;

    /**
       * Semaphore to suppress loading during selection
       */
    bool m_inSelection;

    /**
       * This signals whether a tag can be edited
       **/
    bool m_allowEditing;

    /**
        * This holds the filter type
        */
    int m_tagFilterType;

    QList<MyMoneyTag> m_selectedTags;
};


#endif
