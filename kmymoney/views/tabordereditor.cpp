/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabordereditor.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QEventLoop>
#include <QMenu>
#include <QMetaObject>
#include <QPaintEvent>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QStringList>
#include <QVariant>
#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_tabordereditor.h"

class TabOrderDialogPrivate
{
    Q_DECLARE_PUBLIC(TabOrderDialog)

public:
    TabOrderDialogPrivate(TabOrderDialog* qq)
        : q_ptr(qq)
        , m_editor(nullptr)
        , m_currentIndex(0)
    {
    }

    void setTabOrder(const QStringList& widgetNames)
    {
        m_widgetList.clear();
        for (const auto& widgetName : widgetNames) {
            auto w = ui.m_targetWidget->findChild<QWidget*>(widgetName);
            if (w->property("kmm_taborder").toBool() && w->isVisible()) {
                m_widgetList.append(w);
            } else {
                qDebug() << "Skip invisible" << widgetName;
            }
        }
        m_currentIndex = 0;
    }

    QStringList tabOrder() const
    {
        QStringList widgetNames;
        for (const auto& w : m_widgetList) {
            widgetNames += w->objectName();
        }
        return widgetNames;
    }

    QRect indicatorRectangle(const QWidget* w, const QSize& size) const
    {
        constexpr int horizMargin = 1;
        constexpr int vertMargin = 2;

        if (w == nullptr) {
            return {};
        }

        const QPoint tl = m_editor->mapFromGlobal(w->mapToGlobal(w->rect().topLeft()));
        QRect rect(tl - QPoint(size.width() / 2, size.height() / 8), size);
        rect = QRect(rect.left() - horizMargin, rect.top() - vertMargin, rect.width() + horizMargin * 2, rect.height() + vertMargin * 2);

        return rect;
    }

    void updateIndicatorRegion()
    {
        const auto widgetCount = m_widgetList.count();
        for (int idx = 0; idx < widgetCount; ++idx) {
            const auto txt = QString::number(idx + 1);
            auto w = m_widgetList[idx];
            m_indicatorRegion |= indicatorRectangle(w, m_editor->indicatorFontMetrics().size(Qt::TextSingleLine, txt));
        }
    }

    TabOrderDialog* q_ptr;
    Ui::TabOrderEditor ui;

    TabOrderEditor* m_editor;
    QWidgetList bla;
    QVector<QWidget*> m_widgetList;
    QRegion m_indicatorRegion;
    QStringList m_defaultOrder;
    QStringList m_currentOrder;
    int m_currentIndex;
};

TabOrderDialog::TabOrderDialog(QWidget* parent)
    : QDialog(parent)
    , d_ptr(new TabOrderDialogPrivate(this))
{
    Q_D(TabOrderDialog);
    d->ui.setupUi(this);
}

TabOrderDialog::~TabOrderDialog()
{
    Q_D(TabOrderDialog);
    delete d;
}

void TabOrderDialog::setTarget(TabOrderEditorInterface* targetWidget)
{
    Q_D(TabOrderDialog);

    if (d->ui.m_targetWidget) {
        auto list = d->ui.m_targetWidget->findChildren<QObject*>();
        qDeleteAll(list);
    }
    targetWidget->setupUi(d->ui.m_targetWidget);

    // force the focus on our own button box (in case the targetWidget
    // contains another one which will be found prior to ours when
    // scanning over the widget tree
    d->ui.buttonBox->button(QDialogButtonBox::Ok)->setFocus();
    d->m_defaultOrder.clear();
    d->m_widgetList.clear();
    d->m_indicatorRegion = QRegion();
}

void TabOrderDialog::setDefaultTabOrder(const QStringList& widgetNames)
{
    Q_D(TabOrderDialog);
    d->m_defaultOrder = widgetNames;
}

void TabOrderDialog::setTabOrder(const QStringList& widgetNames)
{
    Q_D(TabOrderDialog);
    d->m_currentOrder = widgetNames;
}

QStringList TabOrderDialog::tabOrder() const
{
    Q_D(const TabOrderDialog);
    return d->tabOrder();
}

int TabOrderDialog::exec()
{
    Q_D(TabOrderDialog);

    // make everything visible before we
    // set the tab order. If we don't do that
    // the widgets will not be found (since
    // not visible)
    show();
    d->setTabOrder(d->m_currentOrder);
    d->m_editor = new TabOrderEditor(this);

    connect(d->m_editor, &TabOrderEditor::geometryUpdated, this, [&]() {
        Q_D(TabOrderDialog);
        d->updateIndicatorRegion();
    });

    d->updateIndicatorRegion();

    return QDialog::exec();
}

class TabOrderEditorPrivate
{
    Q_DECLARE_PUBLIC(TabOrderEditor)

public:
    TabOrderEditorPrivate(TabOrderEditor* qq)
        : q_ptr(qq)
        , m_indicatorFontMetrics(QFont())
        , m_noWidgetsSelected(true)
    {
    }

    void updateGeometry(QPoint pos, QSize size)
    {
        Q_Q(TabOrderEditor);
        // we add 20px at the top so that the indicators
        // in the top row won't get cut off
        size.setHeight(size.height() + 20);
        pos.setY(pos.y() - 20);
        q->move(pos);
        q->resize(size);
        emit q->geometryUpdated();
    }

    int widgetIndexAt(const QPoint& pos) const
    {
        const auto dlg_d = m_dialog->d_func();
        int target_index = -1;
        for (int i = 0; i < dlg_d->m_widgetList.size(); ++i) {
            if (indicatorRectangle(i).contains(pos)) {
                target_index = i;
                break;
            }
        }

        return target_index;
    }

    QRect indicatorRectangle(int idx) const
    {
        const auto dlg_d = m_dialog->d_func();

        const auto txt = QString::number(idx + 1);
        auto w = dlg_d->m_widgetList.at(idx);
        return dlg_d->indicatorRectangle(w, m_indicatorFontMetrics.size(Qt::TextSingleLine, txt));
    }

    TabOrderEditor* q_ptr;
    TabOrderDialog* m_dialog;
    QFontMetrics m_indicatorFontMetrics;
    bool m_noWidgetsSelected;
};

TabOrderEditor::TabOrderEditor(TabOrderDialog* parent)
    : QWidget(parent)
    , d_ptr(new TabOrderEditorPrivate(this))
{
    Q_D(TabOrderEditor);
    d->m_dialog = parent;

    setVisible(true);
    raise();

    QFont indicatorFont;
    indicatorFont = font();
    indicatorFont.setPointSizeF(indicatorFont.pointSizeF() * 1.5);
    indicatorFont.setBold(true);
    d->m_indicatorFontMetrics = QFontMetrics(indicatorFont);
    setFont(indicatorFont);

    d->m_dialog->d_func()->ui.m_targetWidget->installEventFilter(this);

    d->updateGeometry(d->m_dialog->d_func()->ui.m_targetWidget->pos(), d->m_dialog->d_func()->ui.m_targetWidget->size());
    setAttribute(Qt::WA_MouseTracking, true);
}

TabOrderEditor::~TabOrderEditor()
{
    Q_D(TabOrderEditor);
    delete d;
}

void TabOrderEditor::paintEvent(QPaintEvent* e)
{
    Q_D(TabOrderEditor);
    constexpr int backgroundAlpha = 32;
    const auto dlg_d = d->m_dialog->d_func();

    QPainter p(this);

    int lastProcessedIndex = dlg_d->m_currentIndex - 1;
    if (!d->m_noWidgetsSelected && lastProcessedIndex < 0) {
        lastProcessedIndex = dlg_d->m_widgetList.count();
    }

    p.setClipRegion(e->region());
    const auto widgetCount = dlg_d->m_widgetList.count();
    for (int idx = 0; idx < widgetCount; ++idx) {
        const auto txt = QString::number(idx + 1);
        auto w = dlg_d->m_widgetList[idx];
        auto rect = dlg_d->indicatorRectangle(w, d->m_indicatorFontMetrics.size(Qt::TextSingleLine, txt));

        QColor color(Qt::darkGreen);
        if (idx == lastProcessedIndex) {
            color = Qt::red;
        } else if (idx > lastProcessedIndex) {
            color = Qt::darkBlue;
        }

        p.setPen(color);
        color.setAlpha(backgroundAlpha);
        p.setBrush(color);
        p.drawRect(rect);

        rect.moveLeft(rect.left() + 1);
        p.setPen(Qt::white);
        p.drawText(rect, txt, QTextOption(Qt::AlignCenter));
    }
}

bool TabOrderEditor::eventFilter(QObject* o, QEvent* e)
{
    Q_D(TabOrderEditor);
    if (o == d->m_dialog->d_func()->ui.m_targetWidget) {
        switch (e->type()) {
        case QEvent::Move:
            d->updateGeometry(d->m_dialog->d_func()->ui.m_targetWidget->pos(), d->m_dialog->d_func()->ui.m_targetWidget->size());
            break;
        case QEvent::Resize:
            d->updateGeometry(d->m_dialog->d_func()->ui.m_targetWidget->pos(), d->m_dialog->d_func()->ui.m_targetWidget->size());
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(o, e);
}

const QFontMetrics& TabOrderEditor::indicatorFontMetrics() const
{
    Q_D(const TabOrderEditor);
    return d->m_indicatorFontMetrics;
}

void TabOrderEditor::mouseMoveEvent(QMouseEvent* event)
{
    Q_D(TabOrderEditor);
    event->accept();
    if (d->m_dialog->d_func()->m_indicatorRegion.contains(event->pos())) {
        setCursor(Qt::PointingHandCursor);
    } else {
        setCursor(QCursor());
    }
}

void TabOrderEditor::mousePressEvent(QMouseEvent* event)
{
    Q_D(TabOrderEditor);
    const auto dlg_d = d->m_dialog->d_func();

    event->accept();

#if 0
    if (d->m_dialog->d_func()->m_indicatorRegion.contains(event->pos())) {
        if (QWidget *child = m_bg_widget->childAt(e->position().toPoint())) {
            QDesignerFormEditorInterface *core = m_form_window->core();
            if (core->widgetFactory()->isPassiveInteractor(child)) {

                QMouseEvent event(QEvent::MouseButtonPress,
                                  child->mapFromGlobal(e->globalPosition().toPoint()),
                                  e->button(), e->buttons(), e->modifiers());

                qApp->sendEvent(child, &event);

                QMouseEvent event2(QEvent::MouseButtonRelease,
                                   child->mapFromGlobal(e->globalPosition().toPoint()),
                                   e->button(), e->buttons(), e->modifiers());

                qApp->sendEvent(child, &event2);

                updateBackground();
            }
        }
        return;
    }
#endif

    if (event->button() != Qt::LeftButton)
        return;

    const int target_index = d->widgetIndexAt(event->pos());
    if (target_index == -1)
        return;

    d->m_noWidgetsSelected = false;

    if (event->modifiers() & Qt::ControlModifier) {
        dlg_d->m_currentIndex = target_index + 1;
        if (dlg_d->m_currentIndex >= dlg_d->m_widgetList.size()) {
            dlg_d->m_currentIndex = 0;
        }
        update();
        return;
    }

    if (dlg_d->m_currentIndex < 0) {
        return;
    }

    // swap the elements
    const auto w = dlg_d->m_widgetList.at(target_index);
    dlg_d->m_widgetList[target_index] = dlg_d->m_widgetList.at(dlg_d->m_currentIndex);
    dlg_d->m_widgetList[dlg_d->m_currentIndex] = w;

    // continue with next index
    ++dlg_d->m_currentIndex;
    if (dlg_d->m_currentIndex == dlg_d->m_widgetList.size())
        dlg_d->m_currentIndex = 0;

    update();
}

void TabOrderEditor::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_D(TabOrderEditor);
    const auto dlg_d = d->m_dialog->d_func();

    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (d->widgetIndexAt(event->pos()) >= 0) {
        return;
    }

    d->m_noWidgetsSelected = true;
    dlg_d->m_currentIndex = 0;
    update();
}

void TabOrderEditor::contextMenuEvent(QContextMenuEvent* e)
{
    Q_D(TabOrderEditor);
    const auto dlg_d = d->m_dialog->d_func();

    QMenu menu(this);
    menu.addSection(i18nc("@title:menu", "Tab Editor Options"));

    const int target_index = d->widgetIndexAt(e->pos());
    QAction* setIndex = menu.addAction(i18nc("@action:inmenu Move tab editor selector to this widget", "Start from here"));
    setIndex->setEnabled(target_index >= 0);
    QAction* resetIndex = menu.addAction(i18nc("@action:inmenu Set tab editor selector to first widget", "Restart"));

    menu.addSeparator();
    QAction* undoIndex = menu.addAction(i18nc("@action:inmenu Reset all changes", "Undo changes"));
    undoIndex->setEnabled(dlg_d->tabOrder() != dlg_d->m_currentOrder);
    QAction* defaultIndex = menu.addAction(i18nc("@action:inmenu Reset to application defaults", "Reset to default"));
    defaultIndex->setEnabled(dlg_d->tabOrder() != dlg_d->m_defaultOrder);

#if 0
    menu.addSeparator();
    QAction *showDialog = menu.addAction(i18nc("@action:inmenu Open tab order dialog", "Tab Order List..."));
    showDialog->setEnabled(dlg_d->m_widgetList.count() > 1);
#endif

    QAction* result = menu.exec(e->globalPos());
    if (result == resetIndex) {
        d->m_noWidgetsSelected = true;
        dlg_d->m_currentIndex = 0;
        update();
    } else if (result == setIndex) {
        d->m_noWidgetsSelected = true;
        dlg_d->m_currentIndex = target_index + 1;
        if (dlg_d->m_currentIndex >= dlg_d->m_widgetList.count()) {
            dlg_d->m_currentIndex = 0;
        }
        update();
    } else if (result == undoIndex) {
        d->m_noWidgetsSelected = true;
        dlg_d->m_currentIndex = 0;
        dlg_d->setTabOrder(dlg_d->m_currentOrder);
        update();
    } else if (result == defaultIndex) {
        d->m_noWidgetsSelected = true;
        dlg_d->m_currentIndex = 0;
        dlg_d->setTabOrder(dlg_d->m_defaultOrder);
        update();
#if 0
    } else if (result == showDialog) {
#endif
    }
}

void TabOrderEditor::keyPressEvent(QKeyEvent* event)
{
    qDebug() << "Keypress event" << event->key();
    QWidget::keyPressEvent(event);
}
