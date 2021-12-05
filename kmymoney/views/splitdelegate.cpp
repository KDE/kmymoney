/*
    SPDX-FileCopyrightText: 2016-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splitdelegate.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QScrollBar>
#include <QPainter>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "splitview.h"
#include "accountsmodel.h"
#include "splitmodel.h"
#include "newspliteditor.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyfile.h"

QColor SplitDelegate::m_erroneousColor = QColor(Qt::red);
QColor SplitDelegate::m_importedColor = QColor(Qt::yellow);

class SplitDelegate::Private
{
public:
    Private()
        : m_editor(nullptr)
        , m_editorRow(-1)
        , m_showValuesInverted(false)
        , m_readOnly(false)
    {}

    NewSplitEditor* m_editor;
    int m_editorRow;
    bool m_showValuesInverted;
    bool m_readOnly;
    MyMoneySecurity m_commodity;
    QString m_transactionPayeeId;
};


SplitDelegate::SplitDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
    , d(new Private)
{
}

SplitDelegate::~SplitDelegate()
{
    delete d;
}

void SplitDelegate::setErroneousColor(const QColor& color)
{
    m_erroneousColor = color;
}

void SplitDelegate::setCommodity(const MyMoneySecurity& commodity)
{
    d->m_commodity = commodity;
}

void SplitDelegate::setTransactionPayeeId(const QString& id)
{
    d->m_transactionPayeeId = id;
}

QWidget* SplitDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);

    if(index.isValid()) {
        Q_ASSERT(parent);
        auto view = qobject_cast<SplitView*>(parent->parentWidget());
        Q_ASSERT(view != 0);

        if(view->selectionModel()->selectedRows().count() > 1) {
            qDebug() << "Editing multiple splits at once is not yet supported";

            /**
             * @todo replace the following three lines with the creation of a special
             * editor that can handle multiple splits at once or show a message to the user
             * that this is not possible
             */
            d->m_editor = 0;
            SplitDelegate* that = const_cast<SplitDelegate*>(this);
            emit that->closeEditor(d->m_editor, NoHint);

        } else {
            QString accountId = index.data(eMyMoney::Model::SplitAccountIdRole).toString();
            d->m_editor = new NewSplitEditor(parent, d->m_commodity, accountId);
        }

        if(d->m_editor) {
            d->m_editorRow = index.row();
            connect(d->m_editor, &NewSplitEditor::done, this, &SplitDelegate::endEdit);
            emit sizeHintChanged(index);

            d->m_editor->setAmountPlaceHolderText(index.model());

            // propagate read-only mode
            d->m_editor->setReadOnly(d->m_readOnly);
        }

    } else {
        qFatal("SplitDelegate::createEditor(): we should never end up here");
    }
    return d->m_editor;
}

int SplitDelegate::editorRow() const
{
    return d->m_editorRow;
}

void SplitDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // never change the background of the cell the mouse is hovering over
    opt.state &= ~QStyle::State_MouseOver;

    // show the focus only on the detail column
    opt.state &= ~QStyle::State_HasFocus;
    if(index.column() == SplitModel::Column::Memo) {
        QAbstractItemView* view = qobject_cast< QAbstractItemView* >(parent());
        if(view) {
            if(view->currentIndex().row() == index.row()) {
                opt.state |= QStyle::State_HasFocus;
            }
        }
    }

    painter->save();

    // Background
    auto bgOpt = opt;
    // if selected, always show as active, so that the
    // background does not change when the editor is shown
    if (opt.state & QStyle::State_Selected) {
        bgOpt.state |= QStyle::State_Active;
    }
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &bgOpt, painter, opt.widget);

    // Do not paint text if the edit widget is shown
    const auto view = qobject_cast<const SplitView *>(opt.widget);
    if (view && view->indexWidget(index)) {
        painter->restore();
        return;
    }

    const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);

    QStringList lines;
    if(index.column() == SplitModel::Column::Memo) {
        const auto payeeId = index.data(eMyMoney::Model::SplitPayeeIdRole).toString();
        if (!payeeId.isEmpty()) {
            if (payeeId != d->m_transactionPayeeId) {
                lines << index.data(eMyMoney::Model::SplitPayeeRole).toString();
            }
        }
        lines << index.data(eMyMoney::Model::SplitSingleLineMemoRole).toString();
        lines.removeAll(QString());
    }

    // draw the text items
    if(!opt.text.isEmpty() || !lines.isEmpty()) {

        QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                                  ? QPalette::Normal : QPalette::Disabled;

        if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
            cg = QPalette::Inactive;
        }
        if (opt.state & QStyle::State_Selected) {
            painter->setPen(opt.palette.color(cg, QPalette::HighlightedText));
        } else {
            painter->setPen(opt.palette.color(cg, QPalette::Text));
        }
        if (opt.state & QStyle::State_Editing) {
            painter->setPen(opt.palette.color(cg, QPalette::Text));
            painter->drawRect(textArea.adjusted(0, 0, -1, -1));
        }

        // collect data for the various columns
        if(index.column() == SplitModel::Column::Memo) {
            for(int i = 0; i < lines.count(); ++i) {
                painter->drawText(textArea.adjusted(0, (opt.fontMetrics.lineSpacing() + 5) * i, 0, 0), opt.displayAlignment, lines[i]);
            }

        } else {
            painter->drawText(textArea, opt.displayAlignment, opt.text);
        }
    }

    // draw the focus rect
    if(opt.state & QStyle::State_HasFocus) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(opt);
        o.rect = style->proxy()->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt, opt.widget);
        o.state |= QStyle::State_KeyboardFocusChange;
        o.state |= QStyle::State_Item;

        QPalette::ColorGroup cg = (opt.state & QStyle::State_Enabled)
                                  ? QPalette::Normal : QPalette::Disabled;
        o.backgroundColor = opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                              ? QPalette::Highlight : QPalette::Window);
        style->proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, opt.widget);
    }

    painter->restore();
}

QSize SplitDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    bool fullDisplay = false;
    auto view = qobject_cast<SplitView*>(parent());
    if(view) {
        const auto currentIndex = view->currentIndex();
        if(currentIndex.isValid()) {
            const auto currentId = currentIndex.model()->data(currentIndex, eMyMoney::Model::IdRole).toString();
            const auto myId = index.model()->data(index, eMyMoney::Model::IdRole).toString();
            fullDisplay = (currentId == myId);
        }
    }

    QSize size;
    QStyleOptionViewItem opt = option;
    if(index.isValid()) {
        // check if we are showing the edit widget
        const auto* viewWidget = qobject_cast<const QAbstractItemView*>(opt.widget);
        if (viewWidget) {
            const auto editIndex = viewWidget->model()->index(index.row(), 0);
            if(editIndex.isValid()) {
                QWidget* editor = viewWidget->indexWidget(editIndex);
                if(editor) {
                    size = editor->minimumSizeHint();
                    return size;
                }
            }
        }
    }

    int rows = 1;
    if(fullDisplay) {
        initStyleOption(&opt, index);
        rows = 0;
        const auto payeeId = index.data(eMyMoney::Model::SplitPayeeIdRole).toString();
        if (!payeeId.isEmpty() && (payeeId != d->m_transactionPayeeId)) {
            ++rows;
        }
        if (!index.data(eMyMoney::Model::SplitMemoRole).toString().isEmpty()) {
            ++rows;
        }

        // make sure we show at least one row
        if(!rows) {
            rows = 1;
        }
    }

    // leave a 5 pixel margin for each row
    size = QSize(100, (opt.fontMetrics.lineSpacing() + 5) * rows);
    return size;
}

void SplitDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    QStyleOptionViewItem opt = option;
    const auto view = qobject_cast<const SplitView*>(opt.widget);

    QRect r(opt.rect);
    if(view && view->verticalScrollBar()->isVisible()) {
        r.setWidth(opt.widget->width() - view->verticalScrollBar()->width());
    }

    editor->setGeometry(r);
    editor->update();
}

void SplitDelegate::endEdit()
{
    if(d->m_editor) {
        if(d->m_editor->accepted()) {
            emit commitData(d->m_editor);
        }
        emit closeEditor(d->m_editor, NoHint);
        d->m_editorRow = -1;
    }
}

void SplitDelegate::setEditorData(QWidget* editWidget, const QModelIndex& index) const
{
    const SplitModel* model = qobject_cast<const SplitModel*>(index.model());
    NewSplitEditor* editor = qobject_cast<NewSplitEditor*>(editWidget);

    if(model && editor) {
        editor->startLoadingSplit();
        editor->setShowValuesInverted(d->m_showValuesInverted);
        editor->setMemo(index.data(eMyMoney::Model::SplitMemoRole).toString());
        editor->setAccountId(index.data(eMyMoney::Model::SplitAccountIdRole).toString());
        editor->setShares(-(index.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>()));
        editor->setValue(-(index.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>()));
        editor->setCostCenterId(index.data(eMyMoney::Model::SplitCostCenterIdRole).toString());
        editor->setNumber(index.data(eMyMoney::Model::SplitNumberRole).toString());
        editor->setPayeeId(index.data(eMyMoney::Model::SplitPayeeIdRole).toString());
        editor->finishLoadingSplit();
    }
}

void SplitDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    NewSplitEditor* splitEditor = qobject_cast< NewSplitEditor* >(editor);
    if(splitEditor) {
        // prevent update signals
        QSignalBlocker block(model);
        model->setData(index, splitEditor->number(), eMyMoney::Model::SplitNumberRole);
        model->setData(index, splitEditor->memo(), eMyMoney::Model::SplitMemoRole);
        model->setData(index, splitEditor->accountId(), eMyMoney::Model::SplitAccountIdRole);
        model->setData(index, splitEditor->costCenterId(), eMyMoney::Model::SplitCostCenterIdRole);
        model->setData(index, splitEditor->payeeId(), eMyMoney::Model::SplitPayeeIdRole);
        model->setData(index, QVariant::fromValue<MyMoneyMoney>(-splitEditor->shares()), eMyMoney::Model::SplitSharesRole);
        // send out the dataChanged signal with the next (last) setData()
        block.unblock();
        model->setData(index, QVariant::fromValue<MyMoneyMoney>(-splitEditor->value()), eMyMoney::Model::SplitValueRole);

        // in case this was a new split, we need to create a new empty one
        SplitModel* splitModel = qobject_cast<SplitModel*>(model);
        if(splitModel) {
            splitModel->appendEmptySplit();
        }
    }
}

/**
 * This eventfilter seems to do nothing but it prevents that selecting a
 * different row with the mouse closes the editor
 */
bool SplitDelegate::eventFilter(QObject* o, QEvent* event)
{
    return QAbstractItemDelegate::eventFilter(o, event);
}

void SplitDelegate::setShowValuesInverted(bool inverse)
{
    d->m_showValuesInverted = inverse;
}

bool SplitDelegate::showValuesInverted()
{
    return d->m_showValuesInverted;
}

void SplitDelegate::setReadOnlyMode(bool readOnly)
{
    d->m_readOnly = readOnly;
}
