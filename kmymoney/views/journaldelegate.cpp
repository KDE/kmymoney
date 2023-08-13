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

#include <KColorScheme>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "icons.h"
#include "investtransactioneditor.h"
#include "journalmodel.h"
#include "ledgerview.h"
#include "ledgerviewsettings.h"
#include "multitransactioneditor.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"
#include "newtransactioneditor.h"
#include "schedulesjournalmodel.h"

QColor JournalDelegate::m_erroneousColor = QColor(Qt::red);

struct displayProperties {
    int italicStartLine;
    QStringList lines;
};

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
        , m_showPayeeInDetailColumn(true)
        , m_accountType(eMyMoney::Account::Type::Unknown)
    {}

    ~Private()
    {
    }

    bool isInvestmentView()
    {
        if (m_accountType == eMyMoney::Account::Type::Unknown) {
            const auto accountId = m_view->accountId();
            const auto acc = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
            if (!acc.id().isEmpty()) {
                m_accountType = acc.accountType();
            }
        }
        return (m_accountType == eMyMoney::Account::Type::Investment);
    }

    displayProperties displayMatchedString(const QModelIndex& index, const QStyleOptionViewItem& opt)
    {
        Q_UNUSED(opt)
        displayProperties rc;
        rc.italicStartLine = -1;
        if (index.data(eMyMoney::Model::JournalSplitIsMatchedRole).toBool()) {
            if (index.column() == JournalModel::Column::Detail) {
                const auto memo = index.data(eMyMoney::Model::MatchedSplitMemoRole).toString();
                rc.lines << (memo.isEmpty() ? i18nc("@info placeholder for memo of matched transaction if empty", "Empty memo") : memo);
                return rc;
            }
        }
        return rc;
    }

    displayProperties displayString(const QModelIndex& index, const QStyleOptionViewItem& opt)
    {
        displayProperties rc;
        rc.italicStartLine = -1;

        const auto showAllSplits = LedgerViewSettings::instance()->showAllSplits();

        if(index.column() == JournalModel::Column::Detail) {
            const auto showDetails = LedgerViewSettings::instance()->showTransactionDetails();
            const auto showLedgerLens = LedgerViewSettings::instance()->showLedgerLens();
            const auto havePayeeColumn = !m_view->isColumnHidden(JournalModel::Payee);

            if (index.data(eMyMoney::Model::TransactionIsInvestmentRole).toBool() && isInvestmentView()) {
                if (((opt.state & QStyle::State_Selected) && (showLedgerLens)) || showDetails || showAllSplits) {
                    rc.lines << index.data(eMyMoney::Model::SplitActivityRole).toString();
                    rc.lines << index.data(eMyMoney::Model::TransactionBrokerageAccountRole).toString();
                    rc.lines << index.data(eMyMoney::Model::TransactionInterestCategoryRole).toString();
                    rc.lines << index.data(eMyMoney::Model::TransactionFeesCategoryRole).toString();
                    rc.lines << index.data(eMyMoney::Model::SplitSingleLineMemoRole).toString();
                } else {
                    rc.lines << index.data(eMyMoney::Model::SplitActivityRole).toString();
                }

            } else {
                rc.italicStartLine = 1;
                // make sure to not duplicate the payee information in the detail column
                if (havePayeeColumn && (m_singleLineRole == eMyMoney::Model::SplitPayeeRole)) {
                    rc.lines << index.data(eMyMoney::Model::TransactionCounterAccountRole).toString();
                } else {
                    rc.lines << index.data(m_singleLineRole).toString();
                }
                if (showAllSplits && isMultiSplitDisplay(index)) {
                    rc.italicStartLine = 0;
                    rc.lines.clear();
                    if (!havePayeeColumn) {
                        const auto payee = index.data(eMyMoney::Model::SplitPayeeRole).toString();
                        if (!payee.isEmpty()) {
                            rc.lines << payee;
                            ++rc.italicStartLine;
                        }
                    }
                    const auto memo = index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();
                    if (!memo.isEmpty()) {
                        rc.lines << memo;
                        ++rc.italicStartLine;
                    }
                    // make sure to show at least one line even if we have no payee or memo in the split
                    if (rc.italicStartLine == 0) {
                        rc.lines << QStringLiteral(" ");
                    }

                    const auto rowIndeces =
                        MyMoneyFile::instance()->journalModel()->indexesByTransactionId(index.data(eMyMoney::Model::JournalTransactionIdRole).toString());
                    const auto rowCount = rowIndeces.count();
                    const auto splitId = index.data(eMyMoney::Model::IdRole).toString();
                    for (int row = 0; row < rowCount; ++row) {
                        const auto rowIndex = rowIndeces[row];
                        if (rowIndex.data(eMyMoney::Model::IdRole) != splitId) {
                            // don't include the split if the value is zero
                            if (!rowIndex.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().isZero()) {
                                const auto accountId = rowIndex.data(eMyMoney::Model::Roles::JournalSplitAccountIdRole).toString();
                                const auto accountIdx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
                                const auto account = accountIdx.data(eMyMoney::Model::Roles::AccountFullNameRole).toString();
                                const auto splitMemo = rowIndex.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();
                                QString txt;
                                QString sep;
                                if (!account.isEmpty()) {
                                    txt = account;
                                    sep = QStringLiteral(", ");
                                }
                                if (!splitMemo.isEmpty()) {
                                    txt += sep + splitMemo;
                                }
                                rc.lines << txt;
                            }
                        }
                    }
                } else if (((opt.state & QStyle::State_Selected) && (showLedgerLens)) || showDetails || showAllSplits) {
                    rc.lines.clear();
                    if (!havePayeeColumn && m_showPayeeInDetailColumn) {
                        rc.lines << index.data(eMyMoney::Model::Roles::SplitPayeeRole).toString();
                    }
                    rc.lines << index.data(eMyMoney::Model::Roles::TransactionCounterAccountRole).toString();
                    rc.lines << index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();

                } else {
                    if (rc.lines.at(0).isEmpty()) {
                        rc.lines.clear();
                        rc.lines << index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString();
                    }
                    if (rc.lines.at(0).isEmpty()) {
                        rc.lines << index.data(eMyMoney::Model::Roles::TransactionCounterAccountRole).toString();
                    }
                }
            }
            rc.lines.removeAll(QString());

        } else if(index.column() == JournalModel::Column::Quantity) {
            if (index.data(eMyMoney::Model::TransactionIsInvestmentRole).toBool()) {
                const auto showDetails = LedgerViewSettings::instance()->showTransactionDetails();
                const auto showLedgerLens = LedgerViewSettings::instance()->showLedgerLens();
                rc.lines << opt.text;
                if (((opt.state & QStyle::State_Selected) && (showLedgerLens)) || showDetails) {
                    // we have to pay attention here as later on empty items will be removed
                    // from the lines all together. Since we use the detail column as label
                    // we have to make sure that we are not off. Therefore, if the detail column
                    // is filled, we add a simple blank here instead of an empty line.
                    // The first line is always present, so we make sure it is not empty in this column.
                    if (rc.lines[0].isEmpty())
                        rc.lines[0] = QStringLiteral(" ");
                    rc.lines << (index.data(eMyMoney::Model::TransactionBrokerageAccountRole).toString().isEmpty() ? QString() : QStringLiteral(" "));

                    MyMoneySecurity currency = MyMoneyFile::instance()->currency(index.data(eMyMoney::Model::TransactionCommodityRole).toString());

                    if (index.data(eMyMoney::Model::TransactionInterestSplitPresentRole).toBool()) {
                        const auto value = index.data(eMyMoney::Model::TransactionInterestValueRole).value<MyMoneyMoney>();
                        rc.lines << (index.data(eMyMoney::Model::TransactionInterestCategoryRole).toString().isEmpty()
                                         ? QString()
                                         : MyMoneyUtils::formatMoney(-value, currency));
                    }

                    if (index.data(eMyMoney::Model::TransactionFeeSplitPresentRole).toBool()) {
                        const auto value = index.data(eMyMoney::Model::TransactionFeesValueRole).value<MyMoneyMoney>();
                        rc.lines << (index.data(eMyMoney::Model::TransactionFeesCategoryRole).toString().isEmpty()
                                         ? QString()
                                         : MyMoneyUtils::formatMoney(-value, currency));
                    }
                } else {
                    rc.lines << opt.text;
                }
            }
            rc.lines.removeAll(QString());

        } else if (index.column() == JournalModel::Column::Deposit) {
            const auto havePayeeColumn = !m_view->isColumnHidden(JournalModel::Payee);
            rc.lines << opt.text;
            if (showAllSplits && isMultiSplitDisplay(index)) {
                const auto payee = index.data(eMyMoney::Model::SplitPayeeRole).toString();
                if (!havePayeeColumn && !payee.isEmpty() && !index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString().isEmpty()) {
                    rc.lines << QStringLiteral(" ");
                }
                rc.italicStartLine = 1;
                rc.lines << displaySplitValues(index, JournalModel::Column::Payment);
            }

        } else if (index.column() == JournalModel::Column::Payment) {
            const auto havePayeeColumn = !m_view->isColumnHidden(JournalModel::Payee);
            rc.lines << opt.text;
            if (showAllSplits && isMultiSplitDisplay(index)) {
                const auto payee = index.data(eMyMoney::Model::SplitPayeeRole).toString();
                if (!havePayeeColumn && !payee.isEmpty() && !index.data(eMyMoney::Model::Roles::SplitSingleLineMemoRole).toString().isEmpty()) {
                    rc.lines << QStringLiteral(" ");
                }
                rc.italicStartLine = 1;
                rc.lines << displaySplitValues(index, JournalModel::Column::Deposit);
            }

        } else {
            rc.lines << opt.text;
        }
        return rc;
    }

    QStringList displaySplitValues(const QModelIndex& index, JournalModel::Column column)
    {
        QStringList lines;
        const auto rowIndeces =
            MyMoneyFile::instance()->journalModel()->indexesByTransactionId(index.data(eMyMoney::Model::Roles::JournalTransactionIdRole).toString());
        const auto rowCount = rowIndeces.count();
        const auto splitId = index.data(eMyMoney::Model::IdRole).toString();
        for (int row = 0; row < rowCount; ++row) {
            const auto rowIndex = rowIndeces[row];
            if (rowIndex.data(eMyMoney::Model::IdRole) != splitId) {
                // don't include the split if the value is zero
                if (!rowIndex.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().isZero()) {
                    const auto columnIndex = rowIndex.model()->index(rowIndex.row(), column, rowIndex.parent());
                    const auto txt = columnIndex.data().toString();
                    lines << (!txt.isEmpty() ? txt : QStringLiteral(" "));
                }
            }
        }
        return lines;
    }

    inline bool isMultiSplitDisplay(const QModelIndex& index)
    {
        return index.data(eMyMoney::Model::TransactionValuableSplitCountRole).toInt() > 2;
    }

    TransactionEditorBase* m_editor;
    LedgerView* m_view;
    int m_editorRow;
    eMyMoney::Model::Roles m_singleLineRole;
    int m_lineHeight;
    int m_margin;
    int m_editorWidthOfs;
    bool m_showPayeeInDetailColumn;
    eMyMoney::Account::Type m_accountType;
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

void JournalDelegate::setShowPayeeInDetailColumn(bool show)
{
    d->m_showPayeeInDetailColumn = show;
}

QWidget* JournalDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    QString errorMessage;

    if(index.isValid()) {
        d->m_editor = nullptr;
        if(d->m_view->selectionModel()->selectedRows().count() > 1) {
            auto accountId = d->m_view->accountId();
            if (!accountId.isEmpty()) {
                const auto acc = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
                if (acc.accountType() == eMyMoney::Account::Type::Investment) {
                    d->m_editor = nullptr;
                } else {
                    d->m_editor = new MultiTransactionEditor(parent, accountId);
                }
            } else {
                errorMessage =
                    i18nc("@info Editing multiple transactions", "The current implementation cannot modify multiple transactions in different accounts.");
                // Message that multiple edit is only available in ledger (within a single account)
            }

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
            }
        }

        // in case we have an editor, we check that it can perform the action
        if (d->m_editor) {
            if (d->m_editor->setSelectedJournalEntryIds(d->m_view->selectedJournalEntryIds())) {
                d->m_editor->setAmountPlaceHolderText(index.model());
                d->m_editorWidthOfs = 8;
                if(d->m_view) {
                    if(d->m_view->verticalScrollBar()->isVisible()) {
                        d->m_editorWidthOfs += d->m_view->verticalScrollBar()->width();
                    }
                }
            } else {
                // if not get error message and display it and delete the editor again
                /// @todo add error message handling here
                errorMessage = d->m_editor->errorMessage();
                delete d->m_editor;
                d->m_editor = nullptr;
            }
        }

        // if we still have an editor here,
        if(d->m_editor) {
            d->m_editorRow = index.row();
            connect(d->m_editor, &TransactionEditorBase::done, this, &JournalDelegate::endEdit);
            JournalDelegate* that = const_cast<JournalDelegate*>(this);
            Q_EMIT that->sizeHintChanged(index);

            // check if we need to open editor in read-only mode
            const auto journalEntryId = index.data(eMyMoney::Model::IdRole).toString();
            const auto warnLevel = MyMoneyUtils::transactionWarnLevel(journalEntryId);
            d->m_editor->setReadOnly(warnLevel >= OneSplitFrozen);

        } else {
            if (!errorMessage.isEmpty()) {
                KMessageBox::information(0, errorMessage);
            }
            JournalDelegate* that = const_cast<JournalDelegate*>(this);
            Q_EMIT that->closeEditor(d->m_editor, NoHint);
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

    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent());
    const auto editWidget = (d->m_view) ? d->m_view->indexWidget(index) : nullptr;

    // if selected, always show as active, so that the
    // background does not change when the editor is shown
    if (opt.state & QStyle::State_Selected && (editWidget == nullptr)) {
        opt.state |= QStyle::State_Active;
    } else {
        opt.state &= ~QStyle::State_Active;
    }

    // if the widget has a different size than what we can paint on
    // then we adjust the size of the widget so that the focus frame
    // can be painted correctly using a WidgetHintFrame
    if (editWidget) {
        if (editWidget->size() != opt.rect.size()) {
            editWidget->resize(opt.rect.size());
        }
    }
    painter->save();

    // Background
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin);
    const int lineHeight = opt.fontMetrics.lineSpacing() + 2;

    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    QPalette::ColorGroup cg;

    // Do not paint text if the edit widget is shown
    if (editWidget == nullptr) {
        if(view && (index.column() == JournalModel::Column::Detail)) {
            if(view->currentIndex().row() == index.row()) {
                opt.state |= QStyle::State_HasFocus;
            }
        }
        const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);
        const bool selected = opt.state & QStyle::State_Selected;

        const auto displayProperties = d->displayString(index, opt);
        const auto matchedDisplayProperties = d->displayMatchedString(index, opt);

        const int lineCount = displayProperties.lines.count();
        const int matchedLineCount = matchedDisplayProperties.lines.count();

        const bool erroneous = index.data(eMyMoney::Model::Roles::TransactionErroneousRole).toBool();

        // draw the text items
        if (!opt.text.isEmpty() || (lineCount > 0) || (matchedLineCount > 0)) {
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

            painter->save();
            // collect data for the various columns
            for (int i = 0; i < lineCount; ++i) {
                if (i == displayProperties.italicStartLine && LedgerViewSettings::instance()->showAllSplits() && d->isMultiSplitDisplay(index)) {
                    auto font = painter->font();
                    font.setItalic(true);
                    font.setPointSize(font.pointSize() - 2);
                    painter->setFont(font);
                }
                painter->drawText(textArea.adjusted(0, lineHeight * i, 0, 0), opt.displayAlignment, displayProperties.lines[i]);
            }
            painter->restore();

            if (matchedLineCount > 0) {
                painter->drawText(textArea.adjusted(0, lineHeight * lineCount, 0, 0), opt.displayAlignment, matchedDisplayProperties.lines[0]);
                // possibly draw horizontal line as separator
                if (lineCount > 0) {
                    const auto yOffset(lineHeight * lineCount);
                    if (yOffset < opt.rect.height()) {
                        painter->drawLine(opt.rect.x(), opt.rect.y() + yOffset, opt.rect.x() + opt.rect.width(), opt.rect.y() + yOffset);
                    }
                }
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
                case eMyMoney::Model::TransactionIsImportedRole:
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

void JournalDelegate::resetLineHeight()
{
    d->m_lineHeight = -1;
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

    const auto settings = LedgerViewSettings::instance();
    if (((option.state & QStyle::State_Selected) && (settings->showLedgerLens())) || settings->showTransactionDetails()) {
        auto rows = index.data(eMyMoney::Model::JournalSplitMaxLinesCountRole).toInt();
        if (rows == 0) {
            // Scan certain rows which may show multiple lines in a table row
            QSet<int> columns = {JournalModel::Column::Detail, JournalModel::Column::Deposit, JournalModel::Column::Payment};
            for (const auto& column : qAsConst(columns)) {
                const auto idx = index.model()->index(index.row(), column);
                const auto rowCount = d->displayString(idx, option).lines.count() + d->displayMatchedString(idx, option).lines.count();
                if (rowCount > rows) {
                    rows = rowCount;
                }
            }

            // make sure we show at least one row
            if (!rows) {
                rows = 1;
            }
            // and cache the value in the model
            auto model = const_cast<QAbstractItemModel*>(index.model());
            model->setData(index, rows, eMyMoney::Model::JournalSplitMaxLinesCountRole);
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
            Q_EMIT commitData(d->m_editor);
        }
        Q_EMIT closeEditor(d->m_editor, NoHint);
        d->m_editorRow = -1;
        delete d->m_editor;
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
        // the editor may adjust the selection in case it changes when
        // it moves the selected transaction(s) around due to a date change.
        // therefore, we reselect when we return from saving.
        const auto selection = editor->saveTransaction(d->m_view->selectedJournalEntryIds());
        QMetaObject::invokeMethod(d->m_view, "setSelectedJournalEntries", Qt::QueuedConnection, Q_ARG(QStringList, selection));
    }
}

void JournalDelegate::setAccountType(eMyMoney::Account::Type accountType)
{
    d->m_accountType = accountType;
}
