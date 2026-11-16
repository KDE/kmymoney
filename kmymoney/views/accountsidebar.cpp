/*
    SPDX-FileCopyrightText: 2026 Dada Krpasundarananda
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "accountsidebar.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QDrag>
#include <QDate>
#include <QSet>
#include <algorithm>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "kmymoneysettings.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"

// Draws each row of the sidebar itself, so all rows look exactly alike:
// - the selection is one bar across the whole row (no seam between the columns)
// - the text keeps a fixed 6 px padding left and right
// - names that do not fit are shortened with "…", balances are right aligned
// - a row is 1.5 times as high as the text
class RowDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);
        const bool selected = opt.state & QStyle::State_Selected;
        painter->save();
        if (selected)
            painter->fillRect(opt.rect, opt.palette.brush(QPalette::Highlight));
        QColor color = opt.palette.color(selected ? QPalette::HighlightedText : QPalette::Text);
        if (!selected && index.data(Qt::ForegroundRole).isValid())
            color = index.data(Qt::ForegroundRole).value<QBrush>().color(); // grey (hidden) or red (negative)
        painter->setPen(color);
        auto font = opt.font;
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        if (index.column() == 1)
            font.setFeature(QFont::Tag("tnum"), 1); // digits of equal width, so amounts line up
#endif
        painter->setFont(font);
        const auto rect = opt.rect.adjusted(6, 0, -6, 0);
        const auto align = (index.column() == 0 ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter;
        painter->drawText(rect, align, QFontMetrics(font).elidedText(opt.text, Qt::ElideRight, rect.width()));
        painter->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        auto size = QStyledItemDelegate::sizeHint(option, index);
        size.setWidth(size.width() + 12); // room for the padding
        size.setHeight(option.fontMetrics.height() * 3 / 2);
        return size;
    }
};

KConfigGroup AccountSidebar::settings() const
{
    return KSharedConfig::openConfig()->group(QStringLiteral("AccountSidebar"));
}

QString AccountSidebar::accountVisibility(const MyMoneyAccount& acc)
{
    const auto value = acc.value(QLatin1String(visibilityKey), QString::fromLatin1(visibilityNormal));
    if (value == QLatin1String(visibilityHidden) || value == QLatin1String(visibilityAlwaysShown))
        return value;
    return QLatin1String(visibilityNormal);
}

void AccountSidebar::setAccountVisibility(const QString& accountId, const QString& value)
{
    auto file = MyMoneyFile::instance();
    try {
        auto acc = file->account(accountId);
        if (accountVisibility(acc) == value)
            return;
        acc.setValue(QLatin1String(visibilityKey), value, QString::fromLatin1(visibilityNormal));
        MyMoneyFileTransaction ft;
        file->modifyAccount(acc);
        ft.commit();
    } catch (const MyMoneyException& e) {
        qDebug() << "Unable to store sidebar visibility for account" << accountId << e.what();
    }
}

QStringList AccountSidebar::accountOrder()
{
    return MyMoneyFile::instance()->value(QString::fromLatin1(orderKey)).split(QLatin1Char(','), Qt::SkipEmptyParts);
}

void AccountSidebar::setAccountOrder(const QStringList& ids)
{
    auto file = MyMoneyFile::instance();
    const auto encoded = ids.join(QLatin1Char(','));
    if (file->value(QString::fromLatin1(orderKey)) == encoded)
        return;
    MyMoneyFileTransaction ft;
    try {
        file->setValue(QString::fromLatin1(orderKey), encoded);
        ft.commit();
    } catch (const MyMoneyException& e) {
        qDebug() << "Unable to store sidebar order" << e.what();
    }
}

QStringList AccountSidebar::tabsBeforeHide()
{
    return MyMoneyFile::instance()->value(QString::fromLatin1(tabsKey)).split(QLatin1Char(','), Qt::SkipEmptyParts);
}

void AccountSidebar::setTabsBeforeHide(const QStringList& ids)
{
    auto file = MyMoneyFile::instance();
    const auto encoded = ids.join(QLatin1Char(','));
    if (file->value(QString::fromLatin1(tabsKey)) == encoded)
        return;
    MyMoneyFileTransaction ft;
    try {
        file->setValue(QString::fromLatin1(tabsKey), encoded, QString());
        ft.commit();
    } catch (const MyMoneyException& e) {
        qDebug() << "Unable to store the tabs open before hiding the tab bar" << e.what();
    }
}

AccountSidebar::AccountSidebar(QWidget* parent)
    : QTreeWidget(parent)
{
    // two columns: account name and balance
    setColumnCount(2);
    setHeaderHidden(true);
    // all rows must line up the same: no tree indentation, no icons
    setRootIsDecorated(false);
    setIndentation(0);
    setIconSize(QSize(0, 0));
    setUniformRowHeights(true);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);
    // the user can drag accounts into their own order
    // (dragging only "copies", so Qt never removes a row by itself;
    // dropEvent puts the account in its new place)
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::CopyAction);
    // instead of a drop line we show an empty row (see dragMoveEvent)
    setDropIndicatorShown(false);
    // names that do not fit end in "…" and stay left aligned
    setTextElideMode(Qt::ElideRight);
    setItemDelegate(new RowDelegate(this));
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAllColumnsShowFocus(true);
    setViewportMargins(0, 4, 0, 0); // a little room above the first row

    // clicking an entry tells the ledger view which account to open
    connect(this, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item) {
        Q_EMIT accountSelected(item->data(0, Qt::UserRole).toString());
    });
    // rebuild whenever anything in the file changes (new transaction, renamed account, ...)
    // Queued so a write that emits dataChanged cannot rebuild the list mid-drag
    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &AccountSidebar::refresh, Qt::QueuedConnection);
}

void AccountSidebar::refresh()
{
    if (m_blockRefresh || m_refreshing)
        return;
    m_refreshing = true;
    // one-time copy of the old global account lists into this file, if needed

    m_gap = nullptr; // clear() deletes it
    clear();
    const auto file = MyMoneyFile::instance();
    const auto grp = settings();
    const auto order = accountOrder();
    const bool showHidden = grp.readEntry("ShowHidden", false);
    const bool hideZero = grp.readEntry("HideZero", false);

    QList<MyMoneyAccount> accounts;
    file->accountList(accounts);
    const auto today = QDate::currentDate();

    // first alphabetically, then move the accounts the user ordered to the front
    // (in their order); accounts not in the saved order (e.g. new ones) stay at the end
    std::sort(accounts.begin(), accounts.end(), [&](const MyMoneyAccount& a, const MyMoneyAccount& b) {
        const auto ia = order.indexOf(a.id()) < 0 ? order.size() : order.indexOf(a.id());
        const auto ib = order.indexOf(b.id()) < 0 ? order.size() : order.indexOf(b.id());
        if (ia != ib)
            return ia < ib;
        return a.name().compare(b.name(), Qt::CaseInsensitive) < 0;
    });

    for (const auto& acc : std::as_const(accounts)) {
        // only open asset and liability accounts
        if (!acc.isAssetLiability() || acc.isClosed())
            continue;
        // the balance as of today: future dated entries are not counted
        const auto balance = file->balance(acc.id(), today);
        const auto vis = accountVisibility(acc);
        const bool isHidden = (vis == QLatin1String(visibilityHidden));
        if (isHidden && !showHidden)
            continue;
        if (hideZero && balance.isZero() && vis != QLatin1String(visibilityAlwaysShown) && !isHidden)
            continue;

        const auto currency = file->security(acc.currencyId());
        auto item = new QTreeWidgetItem(this, {acc.name(), MyMoneyUtils::formatMoney(balance, acc, currency)});
        item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
        item->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
        item->setData(0, Qt::UserRole, acc.id());
        // negative balances in KMyMoney's colour for negative amounts
        if (balance.isNegative())
            item->setForeground(1, KMyMoneySettings::schemeColor(SchemeColor::Negative));
        // items cannot be dropped onto each other, only between them
        item->setFlags(item->flags() & ~Qt::ItemIsDropEnabled);
        if (isHidden) {
            // hidden accounts are only shown grey and italic so they can be unhidden
            auto font = item->font(0);
            font.setItalic(true);
            for (int col = 0; col < 2; ++col) {
                item->setFont(col, font);
                item->setForeground(col, palette().brush(QPalette::Disabled, QPalette::Text));
            }
        }
    }
    // keep the account of the current ledger highlighted
    selectAccount(m_selectedId);
    m_refreshing = false;
}

void AccountSidebar::fileClosed()
{
    m_selectedId.clear();
    m_gap = nullptr;
    clear();
}

Qt::DropActions AccountSidebar::supportedDropActions() const
{
    return Qt::CopyAction;
}

void AccountSidebar::startDrag(Qt::DropActions supportedActions)
{
    // our own drag picture: the row, half transparent, placed to the
    // lower right of the mouse pointer so it does not hide the gap
    const auto item = currentItem();
    if (!item)
        return;
    const auto rect = visualItemRect(item);
    QPixmap picture(rect.size());
    picture.fill(Qt::transparent);
    QPainter painter(&picture);
    painter.setOpacity(0.6);
    painter.drawPixmap(0, 0, viewport()->grab(rect));
    painter.setOpacity(1.0);
    painter.setPen(palette().color(QPalette::Mid)); // thin outline around the picture
    painter.drawRect(picture.rect().adjusted(0, 0, -1, -1));
    painter.end();

    auto drag = new QDrag(this);
    drag->setMimeData(mimeData(selectedItems()));
    drag->setPixmap(picture);
    drag->setHotSpot(QPoint(-8, -8));
    // while dragging, the account is "in the hand": its row leaves the list
    item->setHidden(true);
    drag->exec(supportedActions, Qt::CopyAction);
    item->setHidden(false); // back in place if the drag was cancelled
    removeGap();
    // the account that was picked up is the one the user wants to see
    Q_EMIT accountSelected(item->data(0, Qt::UserRole).toString());
}

void AccountSidebar::removeGap()
{
    if (m_gap) {
        delete takeTopLevelItem(indexOfTopLevelItem(m_gap));
        m_gap = nullptr;
    }
}

void AccountSidebar::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->source() != this)
        return event->ignore(); // only accept our own accounts
    event->acceptProposedAction();

    // While dragging, an empty row (the "gap") shows where the account will land.
    // Above the middle of a row means before it, below the middle means after it,
    // below all rows means last. Over the gap itself nothing changes.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const auto pos = event->position().toPoint();
#else
    const auto pos = event->pos();
#endif
    const auto item = itemAt(pos);
    if (item && item == m_gap)
        return;
    removeGap();
    int row = topLevelItemCount();
    if (item) {
        row = indexOfTopLevelItem(item);
        if (pos.y() >= visualItemRect(item).center().y())
            ++row;
    }
    m_gap = new QTreeWidgetItem;
    m_gap->setFlags(Qt::NoItemFlags);
    insertTopLevelItem(row, m_gap);
}

void AccountSidebar::dragLeaveEvent(QDragLeaveEvent* event)
{
    QTreeWidget::dragLeaveEvent(event);
    removeGap();
}

void AccountSidebar::dropEvent(QDropEvent* event)
{
    // the account goes where the gap is: we change the saved order
    // (list of account ids, gap replaced by the dragged one) and rebuild the list
    const auto item = currentItem();
    if (item && m_gap) {
        const auto id = item->data(0, Qt::UserRole).toString();
        QStringList ids;
        for (int i = 0; i < topLevelItemCount(); ++i) {
            const auto row = topLevelItem(i);
            if (row == m_gap)
                ids.append(id);
            else if (row != item)
                ids.append(row->data(0, Qt::UserRole).toString());
        }
        auto order = accountOrder();
        for (const auto& i : std::as_const(ids))
            order.removeAll(i);
        // do not rebuild the list until startDrag() has finished using the items
        m_blockRefresh = true;
        setAccountOrder(ids + order);
        m_blockRefresh = false;
    }
    removeGap();
    event->setDropAction(Qt::CopyAction);
    event->accept();
    // rebuild once Qt has finished the drag
    QTimer::singleShot(0, this, &AccountSidebar::refresh);
}

void AccountSidebar::paintEvent(QPaintEvent* event)
{
    QTreeWidget::paintEvent(event);
    if (!m_gap)
        return;
    // draw the gap as a placeholder: a rounded box the size of a row,
    // lightly tinted with the highlight colour and with a thin border
    auto rect = visualItemRect(m_gap);
    rect.setLeft(0);
    rect.setRight(viewport()->width() - 1);
    auto fill = palette().color(QPalette::Highlight);
    fill.setAlpha(60);
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(palette().color(QPalette::Highlight), 1));
    painter.setBrush(fill);
    painter.drawRoundedRect(QRectF(rect).adjusted(2.5, 0.5, -2.5, -0.5), 4, 4);
}

void AccountSidebar::setOnLeft(bool left)
{
    // the sidebar sits in a splitter together with the ledger tabs:
    // position 0 is left of them, position 1 is right of them
    if (auto splitter = qobject_cast<QSplitter*>(parentWidget())) {
        splitter->insertWidget(left ? 0 : 1, this);
        splitter->setStretchFactor(left ? 1 : 0, 1);
        splitter->setStretchFactor(left ? 0 : 1, 0);
        // the sidebar cannot be squeezed away to nothing
        splitter->setCollapsible(splitter->indexOf(this), false);
        // remember the width whenever the user moves the divider
        connect(splitter, &QSplitter::splitterMoved, this, &AccountSidebar::saveWidth, Qt::UniqueConnection);
        applyWidth();
    }
    settings().writeEntry("Position", left ? "left" : "right");
}

void AccountSidebar::setTabWidget(QTabWidget* tabs)
{
    m_tabs = tabs;
    applyTabBarSetting();
}

void AccountSidebar::selectAccount(const QString& accountId)
{
    m_selectedId = accountId;
    clearSelection();
    setCurrentItem(nullptr);
    for (int i = 0; i < topLevelItemCount(); ++i) {
        if (topLevelItem(i)->data(0, Qt::UserRole).toString() == accountId) {
            setCurrentItem(topLevelItem(i));
            scrollToItem(topLevelItem(i));
        }
    }
}

bool AccountSidebar::tabBarHidden() const
{
    return settings().readEntry("HideTabBar", false);
}

void AccountSidebar::applyTabBarSetting()
{
    // only the bar with the tab titles is hidden, the ledgers underneath keep working.
    // While the sidebar itself is not visible, the bar is always shown,
    // so there is always a way to switch between ledgers.
    if (m_tabs)
        m_tabs->tabBar()->setVisible(!isVisible() || !tabBarHidden());
}

void AccountSidebar::applyWidth()
{
    // give the sidebar its saved width, the ledger gets the rest
    auto splitter = qobject_cast<QSplitter*>(parentWidget());
    if (!splitter || splitter->width() <= 0)
        return;
    const int width = settings().readEntry("Width", defaultWidth());
    const int rest = splitter->width() - width - splitter->handleWidth();
    splitter->setSizes(splitter->indexOf(this) == 0 ? QList<int>{width, rest} : QList<int>{rest, width});
}

int AccountSidebar::defaultWidth() const
{
    // wide enough for the longest name plus the widest balance plus padding
    // and a scrollbar, but between 180 px and 350 px (at most a quarter of the window)
    int name = 0, balance = 0;
    for (int i = 0; i < topLevelItemCount(); ++i) {
        name = qMax(name, fontMetrics().horizontalAdvance(topLevelItem(i)->text(0)));
        balance = qMax(balance, fontMetrics().horizontalAdvance(topLevelItem(i)->text(1)));
    }
    const int wanted = name + balance + 4 * 6 + 12 + style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    const int maximum = qMax(180, qMin(350, window()->width() / 4));
    return qBound(180, wanted, maximum);
}

void AccountSidebar::saveWidth()
{
    settings().writeEntry("Width", width());
}

void AccountSidebar::showEvent(QShowEvent* event)
{
    QTreeWidget::showEvent(event);
    if (!m_widthApplied) {
        m_widthApplied = true;
        applyWidth();
    }
    applyTabBarSetting();
}

void AccountSidebar::hideEvent(QHideEvent* event)
{
    QTreeWidget::hideEvent(event);
    applyTabBarSetting();
}

void AccountSidebar::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    auto grp = settings();

    // state of the account under the mouse: Normal, Hide or Always show
    if (auto item = itemAt(event->pos())) {
        const auto id = item->data(0, Qt::UserRole).toString();
        menu.addSection(item->text(0));
        menu.addAction(i18nc("@action:inmenu Account behavior in sidebar", "Normal"), this, [id]() {
            setAccountVisibility(id, QLatin1String(visibilityNormal));
        });
        menu.addAction(i18nc("@action:inmenu Account behavior in sidebar", "Hide"), this, [id]() {
            setAccountVisibility(id, QLatin1String(visibilityHidden));
        });
        menu.addAction(i18nc("@action:inmenu Account behavior in sidebar", "Always show"), this, [id]() {
            setAccountVisibility(id, QLatin1String(visibilityAlwaysShown));
        });
        menu.addSeparator();
    }

    // options for the whole list
    auto showHidden = menu.addAction(i18nc("@action:inmenu Account sidebar", "Show hidden accounts"), this, [this](bool on) {
        settings().writeEntry("ShowHidden", on);
        refresh();
    });
    showHidden->setCheckable(true);
    showHidden->setChecked(grp.readEntry("ShowHidden", false));
    auto hideZero = menu.addAction(i18nc("@action:inmenu Account sidebar", "Hide zero balances"), this, [this](bool on) {
        settings().writeEntry("HideZero", on);
        refresh();
    });
    hideZero->setCheckable(true);
    hideZero->setChecked(grp.readEntry("HideZero", false));
    auto hideTabBar = menu.addAction(i18nc("@action:inmenu Account sidebar", "Hide ledger tab bar"), this, [this](bool on) {
        settings().writeEntry("HideTabBar", on);
        applyTabBarSetting();
        Q_EMIT tabBarHiddenChanged(on);
    });
    hideTabBar->setCheckable(true);
    hideTabBar->setChecked(grp.readEntry("HideTabBar", false));
    menu.addSeparator();
    menu.addAction(i18nc("@action:inmenu Account sidebar", "Hide sidebar"), this, &AccountSidebar::hideRequested);
    menu.addSeparator();
    menu.addAction(i18nc("@action:inmenu Account sidebar", "Show on left"), this, [this]() { setOnLeft(true); });
    menu.addAction(i18nc("@action:inmenu Account sidebar", "Show on right"), this, [this]() { setOnLeft(false); });
    menu.exec(event->globalPos());
}
