/*
    SPDX-FileCopyrightText: 2015-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "journaldelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>
#include <QDate>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "icons.h"
#include "investtransactioneditor.h"
#include "journalmodel.h"
#include "ledgerview.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"
#include "newtransactioneditor.h"
#include "payeesmodel.h"
#include "schedulesjournalmodel.h"

QColor JournalDelegate::m_erroneousColor = QColor(Qt::red);

class JournalDelegate::Private
{
public:
    Private()
        : m_editor(nullptr)
        , m_view(nullptr)
        , m_editorRow(-1)
        , m_singleLineRole(eMyMoney::Model::SplitPayeeRole)
        , m_lineHeight(-1)
        , m_margin(2)

    {}

    ~Private()
    {
    }

    QStringList displayString(const QModelIndex& index, const QStyleOptionViewItem& opt)
    {
        QStringList lines;
        if(index.column() == JournalModel::Column::Detail) {
            if (index.data(eMyMoney::Model::TransactionIsInvestmentRole).toBool()) {
                if(opt.state & QStyle::State_Selected) {
                    lines << index.data(eMyMoney::Model::SplitActivityRole).toString();
                    lines << index.data(eMyMoney::Model::TransactionBrokerageAccountRole).toString();
                    lines << index.data(eMyMoney::Model::TransactionInterestCategoryRole).toString();
                    lines << index.data(eMyMoney::Model::TransactionFeesCategoryRole).toString();
                    lines << index.data(eMyMoney::Model::SplitSingleLineMemoRole).toString();
                } else {
                    lines << index.data(eMyMoney::Model::SplitActivityRole).toString();
                }

            } else {
                lines << index.data(m_singleLineRole).toString();
                if(opt.state & QStyle::State_Selected) {
                    lines.clear();
                    lines << index.data(eMyMoney::Model::Roles::SplitPayeeRole).toString();
                    lines << index.data(eMyMoney::Model::Roles::TransactionCounterAccountRole).toString();
                    lines << index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();

                } else {
                    if(lines.at(0).isEmpty()) {
                        lines.clear();
                        lines << index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();
                    }
                    if(lines.at(0).isEmpty()) {
                        lines << index.data(eMyMoney::Model::Roles::TransactionCounterAccountRole).toString();
                    }
                }
            }
            lines.removeAll(QString());

        } else if(index.column() == JournalModel::Column::Quantity) {
            if (index.data(eMyMoney::Model::TransactionIsInvestmentRole).toBool()) {
                lines << opt.text;
                if(opt.state & QStyle::State_Selected) {
                    // we have to pay attention here as later on empty items will be removed
                    // from the lines all together. Since we use the column detail as label
                    // we have to make that we are not off. Therefor, if the detail column
                    // is filled, we add a simple blank here instead of an empty line.
                    // The first line is always present, so we make sure it is not empty in this column.
                    if (lines[0].isEmpty())
                        lines[0] = QStringLiteral(" ");
                    lines << (index.data(eMyMoney::Model::TransactionBrokerageAccountRole).toString().isEmpty() ? QString() : QStringLiteral(" "));

                    MyMoneySecurity currency = MyMoneyFile::instance()->currency(index.data(eMyMoney::Model::TransactionCommodityRole).toString());

                    if (index.data(eMyMoney::Model::TransactionInterestSplitPresentRole).toBool()) {
                        const auto value = index.data(eMyMoney::Model::TransactionInterestValueRole).value<MyMoneyMoney>();
                        lines << (index.data(eMyMoney::Model::TransactionInterestCategoryRole).toString().isEmpty() ? QString() : MyMoneyUtils::formatMoney(value.abs(), currency));
                    }

                    if (index.data(eMyMoney::Model::TransactionFeeSplitPresentRole).toBool()) {
                        const auto value = index.data(eMyMoney::Model::TransactionFeesValueRole).value<MyMoneyMoney>();
                        lines << (index.data(eMyMoney::Model::TransactionFeesCategoryRole).toString().isEmpty() ? QString() : MyMoneyUtils::formatMoney(value.abs(), currency));
                    }
                } else {
                    lines << opt.text;
                }
            }
            lines.removeAll(QString());

        } else {
            lines << opt.text;
        }
        return lines;
    }

    TransactionEditorBase*        m_editor;
    LedgerView*                   m_view;
    int                           m_editorRow;
    eMyMoney::Model::Roles        m_singleLineRole;
    int                           m_lineHeight;
    int                           m_margin;
    int                           m_editorWidthOfs;
};


JournalDelegate::JournalDelegate(LedgerView* parent)
    : KMMStyledItemDelegate(parent)
    , d(new Private)
{
    d->m_view = parent;
}

JournalDelegate::~JournalDelegate()
{
    delete d;
}

void JournalDelegate::setErroneousColor(const QColor& color)
{
    m_erroneousColor = color;
}

void JournalDelegate::setSingleLineRole(eMyMoney::Model::Roles role)
{
    d->m_singleLineRole = role;
}

QWidget* JournalDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);

    if(index.isValid()) {
        if(d->m_view->selectionModel()->selectedRows().count() > 1) {
            qDebug() << "Editing multiple transactions at once is not yet supported";

            /**
             * @todo replace the following three lines with the creation of a special
             * editor that can handle multiple transactions at once
             */
            d->m_editor = nullptr;
            JournalDelegate* that = const_cast<JournalDelegate*>(this);
            emit that->closeEditor(d->m_editor, NoHint);

        } else {
            auto accountId = index.data(eMyMoney::Model::SplitAccountIdRole).toString();
            if (accountId.isEmpty() || (accountId == MyMoneyFile::instance()->journalModel()->fakeId())) {
                accountId = d->m_view->accountId();
            }
            if (!accountId.isEmpty()) {
                // now determine which editor to use. In case we have no transaction (yet)
                // we use the account type
                if (index.data(eMyMoney::Model::JournalTransactionIdRole).toString().isEmpty()) {
                    const auto acc = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
                    if (acc.accountType() == eMyMoney::Account::Type::Investment) {
                        d->m_editor = new InvestTransactionEditor(parent, accountId);
                    } else {
                        d->m_editor = new NewTransactionEditor(parent, accountId);
                    }
                } else {
                    if (index.data(eMyMoney::Model::TransactionIsInvestmentRole).toBool()) {
                        // in case of an investment transaction we need to use
                        // the parent account of the security account and pass
                        // it to the editor.
                        accountId = index.data(eMyMoney::Model::TransactionInvestmentAccountIdRole).toString();
                        d->m_editor = new InvestTransactionEditor(parent, accountId);
                    } else {
                        d->m_editor = new NewTransactionEditor(parent, accountId);
                    }
                }
                d->m_editorWidthOfs = 8;
                if(d->m_view) {
                    if(d->m_view->verticalScrollBar()->isVisible()) {
                        d->m_editorWidthOfs += d->m_view->verticalScrollBar()->width();
                    }
                }

            } else {
                qDebug() << "Unable to determine account for editing";

                d->m_editor = nullptr;
                JournalDelegate* that = const_cast<JournalDelegate*>(this);
                emit that->closeEditor(d->m_editor, NoHint);
            }
        }

        if(d->m_editor) {
            d->m_editorRow = index.row();
            connect(d->m_editor, &TransactionEditorBase::done, this, &JournalDelegate::endEdit);
            JournalDelegate* that = const_cast<JournalDelegate*>(this);
            emit that->sizeHintChanged(index);
        }

    } else {
        qFatal("JournalDelegate::createEditor(): we should never end up here");
    }
    return d->m_editor;
}

int JournalDelegate::editorRow() const
{
    return d->m_editorRow;
}

void JournalDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // never change the background of the cell the mouse is hovering over
    opt.state &= ~QStyle::State_MouseOver;

    // show the focus only on the detail column
    opt.state &= ~QStyle::State_HasFocus;

    // if selected, always show as active, so that the
    // background does not change when the editor is shown
    if (opt.state & QStyle::State_Selected) {
        opt.state |= QStyle::State_Active;
    }

    painter->save();

    QAbstractItemView* view = qobject_cast< QAbstractItemView* >(parent());
    const bool editWidgetIsVisible = d->m_view && d->m_view->indexWidget(index);

    // Background
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    const int lineHeight = opt.fontMetrics.lineSpacing() + 2;

    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    QPalette::ColorGroup cg;

    // Do not paint text if the edit widget is shown
    if (!editWidgetIsVisible) {
        if(view && (index.column() == JournalModel::Column::Detail)) {
            if(view->currentIndex().row() == index.row()) {
                opt.state |= QStyle::State_HasFocus;
            }
        }
        const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);
        const bool selected = opt.state & QStyle::State_Selected;

        QStringList lines = d->displayString(index, opt);

        const bool erroneous = index.data(eMyMoney::Model::Roles::TransactionErroneousRole).toBool();

        // draw the text items
        if(!opt.text.isEmpty() || !lines.isEmpty()) {

            // check if it is a scheduled transaction and display it as inactive
            if (MyMoneyFile::baseModel()->baseModel(index) == MyMoneyFile::instance()->schedulesJournalModel()) {
                opt.state &= ~QStyle::State_Enabled;
            }
            cg = (opt.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;

            if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
                cg = QPalette::Inactive;
            }
            if (selected) {
                // always use the normal palette since the background is also in normal
                painter->setPen(opt.palette.color(QPalette::ColorGroup(QPalette::Normal), QPalette::HighlightedText));

            } else if (erroneous) {
                painter->setPen(m_erroneousColor);

            } else {
                painter->setPen(opt.palette.color(cg, QPalette::Text));
            }

            if (opt.state & QStyle::State_Editing) {
                painter->setPen(opt.palette.color(cg, QPalette::Text));
                painter->drawRect(textArea.adjusted(0, 0, -1, -1));
            }

            // collect data for the various columns
            for(int i = 0; i < lines.count(); ++i) {
                painter->drawText(textArea.adjusted(0, lineHeight * i, 0, 0), opt.displayAlignment, lines[i]);
            }
        }

        // draw the focus rect
        if(opt.state & QStyle::State_HasFocus) {
            QStyleOptionFocusRect o;
            o.QStyleOption::operator=(opt);
            o.rect = style->proxy()->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt, opt.widget);
            o.state |= QStyle::State_KeyboardFocusChange;
            o.state |= QStyle::State_Item;

            cg = (opt.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
            o.backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                                  ? QPalette::Highlight : QPalette::Window);
            style->proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, opt.widget);
        }

        // take care of icons on the transaction
        if (index.column() == JournalModel::Column::Detail) {
            QRect iconArea = QRect(opt.rect.x() + margin, opt.rect.y(), opt.rect.width() - 2 * margin, opt.rect.height());
            const auto iconHeight = d->m_lineHeight + 2 * d->m_margin;

            // draw the icons
            const auto statusRoles = d->m_view->statusRoles(index);
            for (int i = 0; i < statusRoles.count(); ++i) {
                QIcon icon;
                switch (statusRoles[i]) {
                case eMyMoney::Model::TransactionErroneousRole:
                case eMyMoney::Model::ScheduleIsOverdueRole:
                    icon = style->proxy()->standardIcon(QStyle::SP_MessageBoxWarning, &option, option.widget);
                    break;
                case eMyMoney::Model::TransactionImportedRole:
                    icon = Icons::get(Icons::Icon::TransactionStateImported);
                    break;
                case eMyMoney::Model::JournalSplitIsMatchedRole:
                    icon = Icons::get(Icons::Icon::Link);
                    break;
                default:
                    break;
                }
                if (!icon.isNull()) {
                    const auto pixmap = icon.pixmap(iconHeight, iconHeight, QIcon::Active, QIcon::On);
                    style->proxy()->drawItemPixmap(painter, iconArea, Qt::AlignRight | Qt::AlignTop, pixmap);
                    iconArea.setRight(iconArea.right() - (pixmap.width() + margin));
                }
            }
        }
    }

    painter->restore();
}

QSize JournalDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // get parameters only once per update to speed things up
    if (d->m_lineHeight == -1) {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        d->m_margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
        d->m_lineHeight = opt.fontMetrics.lineSpacing();
    }
    int rows = 1;

    if(index.isValid()) {
        // check if we are showing the edit widget
        // const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>(opt.widget);
        if (d->m_view) {
            QModelIndex editIndex = d->m_view->model()->index(index.row(), 0);
            if(editIndex.isValid()) {
                QWidget* editor = d->m_view->indexWidget(editIndex);
                if(editor) {
                    return editor->minimumSizeHint();
                }
            }
        }
    }

    QSize size(10, d->m_lineHeight + 2 * d->m_margin);

    if(option.state & QStyle::State_Selected) {
        rows = d->displayString(index, option).count();

        // make sure we show at least one row
        if(!rows) {
            rows = 1;
        }
        // leave a few pixels as margin for each space between rows
        size.setHeight((size.height() * rows) - (d->m_margin * (rows - 1)));
    }
    return size;
}

void JournalDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    QRect r(option.rect);
    // respect the vertical scrollbar if visible
    if (option.widget
            && d->m_view
            && d->m_view->verticalScrollBar()->isVisible() ) {
        r.setWidth(option.widget->width() - d->m_view->verticalScrollBar()->width());
    }
    editor->setGeometry(r);
    editor->update();
}

void JournalDelegate::endEdit()
{
    if(d->m_editor) {
        if(d->m_editor->accepted()) {
            emit commitData(d->m_editor);
        }
        emit closeEditor(d->m_editor, NoHint);
        d->m_editorRow = -1;
        d->m_editor = nullptr;
    }
}

/**
 * This eventfilter seems to do nothing but it prevents that selecting a
 * different row with the mouse closes the editor
 */
bool JournalDelegate::eventFilter(QObject* o, QEvent* event)
{
    return QAbstractItemDelegate::eventFilter(o, event);
}

void JournalDelegate::setEditorData(QWidget* editWidget, const QModelIndex& index) const
{
    auto* editor = qobject_cast<TransactionEditorBase*>(editWidget);
    if(editor) {
        editor->loadTransaction(index);
    }
}

void JournalDelegate::setModelData(QWidget* editWidget, QAbstractItemModel* model, const QModelIndex& index) const
{
    Q_UNUSED(model)
    Q_UNUSED(index)

    auto* editor = qobject_cast<TransactionEditorBase*>(editWidget);
    if(editor) {
        // saving the transaction may move the selected transaction(s) around
        // we keep the transaction IDs here and take care of them when we return
        const auto selection = d->m_view->selectedJournalEntries();

        editor->saveTransaction();

        d->m_view->setSelectedJournalEntries(selection);
    }
}
