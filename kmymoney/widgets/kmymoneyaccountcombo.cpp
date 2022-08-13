/*
    SPDX-FileCopyrightText: 2004-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneyaccountcombo.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QList>
#include <QRegularExpression>
#include <QTreeView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyenums.h"
#include "accountsmodel.h"
#include "icons.h"

class KMyMoneyAccountCombo::Private
{
public:
    Private(KMyMoneyAccountCombo* q)
        : m_q(q)
        , m_popupView(nullptr)
        , m_splitAction(nullptr)
        , m_inMakeCompletion(false)
    {
        m_q->setInsertPolicy(QComboBox::NoInsert);
        m_q->setMinimumWidth(m_q->fontMetrics().horizontalAdvance(QLatin1Char('W')) * 15);
        m_q->setMaxVisibleItems(15);
    }

    KMyMoneyAccountCombo*           m_q;
    QTreeView*                      m_popupView;
    QAction*                        m_splitAction;
    QString                         m_lastSelectedAccount;
    QModelIndex                     m_lastSelectedIndex;
    bool                            m_inMakeCompletion;

    void selectFirstMatchingItem()
    {
        if(m_popupView) {
            QSignalBlocker blocker(m_popupView);
            m_popupView->setCurrentIndex(QModelIndex());
            const auto rows = m_q->model()->rowCount();
            const auto filterModel = qobject_cast<AccountNamesFilterProxyModel*>(m_q->model());
            const auto filterExp = filterModel->filterRegularExpression();
            for (auto i = 0; i < rows; ++i) {
                QModelIndex childIndex = filterModel->index(i, 0);
                if (filterModel->hasChildren(childIndex)) {
                    // search the first match by walking down the first leaf
                    do {
                        childIndex = filterModel->index(0, 0, childIndex);
                        if (filterModel->flags(childIndex) & Qt::ItemIsSelectable) {
                            if (filterExp.match(childIndex.data(eMyMoney::Model::AccountFullHierarchyNameRole).toString()).hasMatch()) {
                                break;
                            }
                        }
                    } while (filterModel->hasChildren(childIndex));

                    // make it the current selection if it's selectable
                    if (filterModel->flags(childIndex) & Qt::ItemIsSelectable) {
                        m_popupView->setCurrentIndex(childIndex);
                    }
                    break;
                }
            }
        }
    }

    /**
     * Find which item has the @a id and return its index
     * into the model of the KMyMoneyAccountCombo. In case
     * no item is found, an invalid QModelIndex will be
     * returned. If the current selection is pointing to
     * one of the favorite accounts, the index for the
     * corresponding entry in the hierarchy will be returned.
     */
    QModelIndex findMatchingItem(const QString& id)
    {
        const auto startRow =
            m_q->model()->index(0, 0).data(eMyMoney::Model::Roles::IdRole).toString() == MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Favorite) ? 1
                                                                                                                                                           : 0;
        // Note: Without Qt::MatchWrap we might not get results for credit card
        const auto list = m_q->model()->match(m_q->model()->index(startRow, 0),
                                              eMyMoney::Model::Roles::IdRole,
                                              QVariant(id),
                                              1,
                                              Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap | Qt::MatchRecursive));
        if (!list.isEmpty()) {
            return list.first();
        }
        return {};
    }

    /**
     * Set the current index based on the QModelIndex @a idx.
     * While setting the index, the signals sent out by the
     * KMyMoneyAccountCombo are blocked.
     */
    void setCurrentIndex(const QModelIndex& idx)
    {
        QSignalBlocker blocker(m_q);
        m_q->setRootModelIndex(idx.parent());
        m_q->setCurrentIndex(idx.row());
        m_q->setRootModelIndex(QModelIndex());
    }

    void showSplitAction(bool show)
    {
        if (show && !m_splitAction) {
            m_splitAction = m_q->lineEdit()->addAction(Icons::get(Icons::Icon::Split), QLineEdit::TrailingPosition);
            // this for some reason does not work and I had to
            // add logic to the eventFilter below to catch this
            // key-sequence. Left it here, since it does not hurt either.
            m_splitAction->setShortcut(QKeySequence(Qt::Key_Control, Qt::Key_Space));
            m_q->connect(m_splitAction, &QAction::triggered, m_q, &KMyMoneyAccountCombo::splitDialogRequest);

        } else if(!show && m_splitAction) {
            m_q->lineEdit()->removeAction(m_splitAction);
            m_splitAction->deleteLater();
            m_splitAction = nullptr;
        }
    }
};





KMyMoneyAccountCombo::KMyMoneyAccountCombo(QSortFilterProxyModel *model, QWidget *parent)
    : KComboBox(parent)
    , d(new Private(this))
{
    init();
    setModel(model);
}

KMyMoneyAccountCombo::KMyMoneyAccountCombo(QWidget *parent)
    : KComboBox(parent)
    , d(new Private(this))
{
    init();
}

void KMyMoneyAccountCombo::init()
{
    setMaxVisibleItems(15);
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

KMyMoneyAccountCombo::~KMyMoneyAccountCombo()
{
}

void KMyMoneyAccountCombo::setEditable(bool isEditable)
{
    KComboBox::setEditable(isEditable);
    // don't do the standard behavior
    if(lineEdit()) {
        lineEdit()->setClearButtonEnabled(true);
        connect(lineEdit(), &QLineEdit::textEdited, this, &KMyMoneyAccountCombo::makeCompletion, Qt::UniqueConnection);
        installEventFilter(this);
        d->showSplitAction(true);
    }
}

void KMyMoneyAccountCombo::setSplitActionVisible(bool show)
{
    if (lineEdit()) {
        d->showSplitAction(show);
    }
}

void KMyMoneyAccountCombo::wheelEvent(QWheelEvent *ev)
{
    Q_UNUSED(ev)
    // don't change anything with the help of the wheel, yet (due to the tree model)
}

void KMyMoneyAccountCombo::expandAll()
{
    if (d->m_popupView)
        d->m_popupView->expandAll();
}

void KMyMoneyAccountCombo::collapseAll()
{
    if (d->m_popupView)
        d->m_popupView->collapseAll();
}

void KMyMoneyAccountCombo::activated()
{
    auto accountId = view()->currentIndex().data(eMyMoney::Model::Roles::IdRole).toString();
    if (!accountId.isEmpty() && !(lineEdit() && lineEdit()->text().isEmpty())) {
        selectItem(view()->currentIndex());
    }
}

bool KMyMoneyAccountCombo::eventFilter(QObject* o, QEvent* e)
{
    if(isEditable()) {
        if (o == d->m_popupView) {
            // propagate all relevant key press events to the lineEdit widget
            if(e->type() == QEvent::KeyPress) {
                QKeyEvent* kev = static_cast<QKeyEvent*>(e);
                bool forLineEdit = (kev->text().length() > 0);
                switch(kev->key()) {
                case Qt::Key_Tab:
                case Qt::Key_Backtab: {
                    const auto idx = view()->currentIndex();
                    const auto accountId = idx.data(eMyMoney::Model::Roles::IdRole).toString();
                    if (!accountId.isEmpty()) {
                        d->m_lastSelectedAccount = accountId;
                        QSignalBlocker blocker(lineEdit());
                        lineEdit()->setText(idx.data(eMyMoney::Model::Roles::AccountFullNameRole).toString());
                        d->setCurrentIndex(d->findMatchingItem(accountId));
                    }
                    hidePopup();
                    break;
                }

                case Qt::Key_Escape:
                case Qt::Key_Up:
                case Qt::Key_Down:
                    forLineEdit = false;
                    break;
                default:
                    break;
                }
                if(forLineEdit) {
                    return lineEdit()->event(e);
                }
            } else if(e->type() == QEvent::KeyRelease) {
                QKeyEvent* kev = static_cast<QKeyEvent*>(e);
                switch(kev->key()) {
                case Qt::Key_Escape:
                    hidePopup();
                    return true;

                case Qt::Key_Enter:
                case Qt::Key_Return:
                    activated();
                    hidePopup();
                    break;
                default:
                    break;
                }

            } else if(e->type() == QEvent::FocusOut) {
                hidePopup();
            }

        } else if(o == this) {
            if(e->type() == QEvent::KeyPress) {
                const auto kev = static_cast<QKeyEvent*>(e);
                if (kev->modifiers() & Qt::ControlModifier && kev->key() == Qt::Key_Space) {
                    emit splitDialogRequest();
                    return true;
                }
            }
        }
    }
    return KComboBox::eventFilter(o, e);
}

void KMyMoneyAccountCombo::setSelected(const QString& id)
{
    if (id.isEmpty()) {
        d->m_lastSelectedAccount.clear();
        d->m_popupView->selectionModel()->clearSelection();
        d->m_popupView->setCurrentIndex(QModelIndex());
        setRootModelIndex(QModelIndex());
        setCurrentIndex(-1);
        emit accountSelected(id);
        return;
    }

    if (id == d->m_lastSelectedAccount) {
        // nothing to do
        return;
    }

    // make sure, we have all items available for search
    if(isEditable()) {
        lineEdit()->clear();
    }

    // reset the filter of the model
    auto* filterModel = qobject_cast<QSortFilterProxyModel*>(model());
    filterModel->setFilterFixedString(QString());

    const auto idx = d->findMatchingItem(id);
    if (idx.isValid()) {
        // make sure the popup is closed from here on
        hidePopup();
        d->m_lastSelectedAccount = id;
        d->setCurrentIndex(idx);
        emit accountSelected(id);
    }
}

const QString& KMyMoneyAccountCombo::getSelected() const
{
    return d->m_lastSelectedAccount;
}

void KMyMoneyAccountCombo::setModel(QSortFilterProxyModel *model)
{
    // CAUTION! Assumption is being made that AccountName column number is always 0
    if (AccountsModel::Column::AccountName != 0) {
        qFatal("AccountsModel::Column::AccountName must be 0 in accountsmodel.h");
    }

    // since we create a new popup view, we get rid of an existing one
    delete d->m_popupView;

    // call base class implementation
    KComboBox::setModel(model);

    // setup filtering criteria
    model->setFilterKeyColumn(AccountsModel::Column::AccountName);
    model->setFilterRole(eMyMoney::Model::Roles::AccountFullHierarchyNameRole);

    // create popup view, attach model and allow to select a single item
    d->m_popupView = new QTreeView(this);
    d->m_popupView->setModel(model);
    d->m_popupView->setSelectionMode(QAbstractItemView::SingleSelection);
    setView(d->m_popupView);

    // setup view parameters
    d->m_popupView->setHeaderHidden(true);
    d->m_popupView->setRootIsDecorated(true);
    d->m_popupView->setAlternatingRowColors(true);
    d->m_popupView->setAnimated(true);

    d->m_popupView->expandAll();
    connect(d->m_popupView, &QTreeView::activated, this, &KMyMoneyAccountCombo::selectItem);

    d->m_popupView->installEventFilter(this);

    // for some unknown reason, the first selection with the mouse (not with the keyboard)
    // after the qlineedit had been cleared using the clear button does not trigger the
    // activated() signal of d->m_popupView. This is a workaround to catch this scenario
    // and still get valid settings.
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
        if (index != -1) {
            const auto idx = d->m_popupView->currentIndex();
            if (idx.isValid()) {
                if (d->m_lastSelectedAccount.isEmpty() || (idx.data(eMyMoney::Model::IdRole).toString() != d->m_lastSelectedAccount)) {
                    selectItem(idx);
                }
            }
        }
    });

    if(isEditable()) {
        connect(lineEdit(), &QLineEdit::textEdited, this, &KMyMoneyAccountCombo::makeCompletion, Qt::UniqueConnection);
    } else {
        connect(this, static_cast<void (KComboBox::*)(int)>(&KMyMoneyAccountCombo::KComboBox::activated), this, &KMyMoneyAccountCombo::activated);
    }
}

void KMyMoneyAccountCombo::selectItem(const QModelIndex& index)
{
    if (d->m_inMakeCompletion)
        return;

    if (!index.isValid())
        return;

    if (index.model() != model()) {
        qDebug() << "KMyMoneyAccountCombo::selectItem called with wrong model" << index;
    }
    if (index.model()->flags(index) & Qt::ItemIsSelectable) {
        // delay the call until the next time in the event loop
        QMetaObject::invokeMethod(this, "setSelected", Qt::QueuedConnection, Q_ARG(QString, index.data(eMyMoney::Model::Roles::IdRole).toString()));
    }
}

void KMyMoneyAccountCombo::makeCompletion(const QString& txt)
{
    if(!d->m_inMakeCompletion) {
        d->m_inMakeCompletion = true;
        if (txt.isEmpty()) {
            setSelected(QString());
        } else {
            AccountNamesFilterProxyModel* filterModel = qobject_cast<AccountNamesFilterProxyModel*>(model());

            if(filterModel) {
                const auto completionStr = QStringLiteral(".*");
                // for some reason it helps to avoid internal errors if we
                // clear the filter before setting it to a new value
                filterModel->setFilterFixedString(QString());
                if (txt.contains(MyMoneyFile::AccountSeparator) == 0) {
                    const auto filterString = QString::fromLatin1("%1%2%3").arg(completionStr).arg(QRegularExpression::escape(txt)).arg(completionStr);
                    filterModel->setFilterRegularExpression(QRegularExpression(filterString, QRegularExpression::CaseInsensitiveOption));
                } else {
                    QStringList parts = txt.split(MyMoneyFile::AccountSeparator /*, Qt::SkipEmptyParts */);
                    QString pattern;
                    QStringList::iterator it;
                    for (it = parts.begin(); it != parts.end(); ++it) {
                        if (pattern.length() > 1)
                            pattern += MyMoneyFile::AccountSeparator;
                        pattern += QRegularExpression::escape(QString(*it).trimmed()) + completionStr;
                    }
                    filterModel->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
                    // if we don't have a match, we try it again, but this time
                    // we add a wildcard for the top level
                    if (filterModel->visibleItems() == 0) {
                        // for some reason it helps to avoid internal errors if we
                        // clear the filter before setting it to a new value
                        filterModel->setFilterFixedString(QString());
                        pattern = pattern.prepend(completionStr + MyMoneyFile::AccountSeparator);
                        filterModel->setFilterRegularExpression(QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption));
                    }
                }

                // if nothing is shown, we might as well close the popup
                switch(filterModel->visibleItems()) {
                case 0:
                    hidePopup();
                    break;
                default:
                    expandAll();
                    showPopup();
                    d->selectFirstMatchingItem();
                    break;
                }

                // we don't have a selection since the user edits the name
                d->m_lastSelectedAccount.clear();

                // keep current text in edit widget no matter what
                QSignalBlocker blocker(lineEdit());
                lineEdit()->setText(txt);
            }
        }
        d->m_inMakeCompletion = false;
    }
}

void KMyMoneyAccountCombo::showPopup()
{
    if(d->m_popupView) {
        d->m_popupView->show();
    }
    KComboBox::showPopup();
}

void KMyMoneyAccountCombo::hidePopup()
{
    if(d->m_popupView) {
        d->m_popupView->hide();
    }
    KComboBox::hidePopup();
}

QTreeView* KMyMoneyAccountCombo::popup() const
{
    return d->m_popupView;
}

void KMyMoneyAccountCombo::clearSelection()
{
    d->m_lastSelectedAccount.clear();
    setCurrentIndex(-1);
    clearEditText();
}

class KMyMoneyAccountComboSplitHelperPrivate
{
    Q_DISABLE_COPY(KMyMoneyAccountComboSplitHelperPrivate)
    Q_DECLARE_PUBLIC(KMyMoneyAccountComboSplitHelper)

public:
    KMyMoneyAccountComboSplitHelperPrivate(KMyMoneyAccountComboSplitHelper* qq)
        : q_ptr(qq)
        , m_accountCombo(nullptr)
        , m_splitModel(nullptr)
        , m_norecursive(false)
    {
    }

    KMyMoneyAccountComboSplitHelper*  q_ptr;
    QComboBox*                        m_accountCombo;
    QAbstractItemModel*               m_splitModel;
    bool                              m_norecursive;
};


KMyMoneyAccountComboSplitHelper::KMyMoneyAccountComboSplitHelper(QComboBox* accountCombo, QAbstractItemModel* model)
    : QObject(accountCombo)
    , d_ptr(new KMyMoneyAccountComboSplitHelperPrivate(this))
{
    Q_D(KMyMoneyAccountComboSplitHelper);
    d->m_accountCombo = accountCombo;
    d->m_splitModel = model;

    connect(model, &QAbstractItemModel::dataChanged, this, &KMyMoneyAccountComboSplitHelper::updateWidget);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &KMyMoneyAccountComboSplitHelper::updateWidget, Qt::QueuedConnection);
    connect(model, &QAbstractItemModel::modelReset, this, &KMyMoneyAccountComboSplitHelper::updateWidget, Qt::QueuedConnection);
    connect(model, &QAbstractItemModel::destroyed, this, &KMyMoneyAccountComboSplitHelper::modelDestroyed);

    accountCombo->installEventFilter(this);
    if (accountCombo->lineEdit()) {
        accountCombo->lineEdit()->installEventFilter(this);
    }
    updateWidget();
}

KMyMoneyAccountComboSplitHelper::~KMyMoneyAccountComboSplitHelper()
{
}

bool KMyMoneyAccountComboSplitHelper::eventFilter(QObject* watched, QEvent* event)
{
    Q_D(KMyMoneyAccountComboSplitHelper);
    if (d->m_splitModel && (d->m_splitModel->rowCount() > 1)) {
        const auto type = event->type();
        if (watched == d->m_accountCombo) {
            if (type == QEvent::FocusIn) {
                // select the complete text (which is readonly)
                // to signal focus in the lineedit widget to the user
                const auto lineEdit = d->m_accountCombo->lineEdit();
                if (lineEdit) {
                    lineEdit->end(false);
                    lineEdit->home(true);
                }
            }
        }
        if ((type == QEvent::MouseButtonPress)
                || (type == QEvent::MouseButtonRelease)
                || (type == QEvent::MouseButtonDblClick)) {
            // suppress opening the combo box
            // or selecting text in the lineedit
            return true;
        }
        if (type == QEvent::KeyPress) {
            auto kev = static_cast<QKeyEvent*>(event);
            // swallow all keypress except Ctrl+Space, Return,
            // Enter, Tab, BackTab and Esc
            switch(kev->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                return false;

            case Qt::Key_Space:
                return !(kev->modifiers() & Qt::ControlModifier);

            default:
                break;
            }
            return true;
        }

    }
    return QObject::eventFilter(watched, event);
}

void KMyMoneyAccountComboSplitHelper::modelDestroyed()
{
    Q_D(KMyMoneyAccountComboSplitHelper);
    d->m_splitModel = nullptr;
}

void KMyMoneyAccountComboSplitHelper::updateWidget()
{
    Q_D(KMyMoneyAccountComboSplitHelper);
    // sanity check
    if (!d->m_accountCombo || !d->m_splitModel || d->m_norecursive)
        return;

    d->m_norecursive = true;

    QModelIndexList indexes;
    bool disabled = false;

    const auto rows = d->m_splitModel->rowCount();
    const auto accountCombo = qobject_cast<KMyMoneyAccountCombo*>(d->m_accountCombo);
    switch (rows) {
    case 0:
        d->m_accountCombo->setCurrentIndex(-1);
        d->m_accountCombo->setCurrentText(QString());
        break;
    case 1:
        indexes = d->m_accountCombo->model()->match(d->m_accountCombo->model()->index(0,0),
                  eMyMoney::Model::IdRole,
                  d->m_splitModel->index(0, 0).data(eMyMoney::Model::SplitAccountIdRole).toString(),
                  1,
                  Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap | Qt::MatchRecursive));
        if (indexes.isEmpty()) {
            if (accountCombo) {
                accountCombo->clearSelection();
            } else {
                d->m_accountCombo->setCurrentIndex(-1);
                d->m_accountCombo->setCurrentText(QString());
            }
        } else {
            const auto idx = indexes.first();
            if (accountCombo) {
                const auto accountId = idx.data(eMyMoney::Model::IdRole).toString();
                accountCombo->setSelected(accountId);
            } else {
                QSignalBlocker comboBoxBlocker(d->m_accountCombo);
                d->m_accountCombo->setRootModelIndex(idx.parent());
                d->m_accountCombo->setCurrentIndex(idx.row());
                d->m_accountCombo->setRootModelIndex(QModelIndex());
            }
        }
        break;
    default:
    {
        QSignalBlocker lineEditBlocker(d->m_accountCombo->lineEdit());
        QString txt, sep;
        for (int row = 0; row < rows; ++row) {
            const auto idx = d->m_splitModel->index(row, 0);
            indexes = d->m_accountCombo->model()->match(d->m_accountCombo->model()->index(0,0),
                      eMyMoney::Model::IdRole,
                      idx.data(eMyMoney::Model::SplitAccountIdRole).toString(),
                      1,
                      Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap | Qt::MatchRecursive));
            if (!indexes.isEmpty()) {
                txt += sep + indexes.first().data(eMyMoney::Model::AccountNameRole).toString();
                sep = QStringLiteral(", ");
            }
        }
        d->m_accountCombo->lineEdit()->setText(txt);
        d->m_accountCombo->lineEdit()->home(false);
        disabled = true;
    }
    break;
    }
    d->m_accountCombo->hidePopup();
    d->m_accountCombo->lineEdit()->setReadOnly(disabled);

    emit accountComboEnabled(!disabled);
    emit accountComboDisabled(disabled);

    d->m_norecursive = false;
}
