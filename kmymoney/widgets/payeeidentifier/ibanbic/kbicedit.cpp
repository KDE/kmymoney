/*
    SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kbicedit.h"

#include <QApplication>
#include <QCompleter>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QAbstractItemView>

#include "kmymoneyplugin.h"
#include "bicvalidator.h"
#include "kmymoneyvalidationfeedback.h"
#include "plugins/ibanbicdata/ibanbicdataenums.h"

class bicItemDelegate : public QStyledItemDelegate
{
public:
    explicit bicItemDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}
    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const final override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const final override;

private:
    inline QFont getSmallFont(const QStyleOptionViewItem& option) const;
};

KBicEdit::KBicEdit(QWidget* parent)
    : KLineEdit(parent)
{
    QCompleter* completer = new QCompleter(this);
    if (auto plugin = pPlugins.data.value(QString::fromLatin1("ibanbicdata"), nullptr))
        if (auto model = plugin->requestData(QString(), eIBANBIC::DataType::bicModel).value<QAbstractItemModel *>())
            completer->setModel(model);

    m_popupDelegate = new bicItemDelegate(this);
    completer->popup()->setItemDelegate(m_popupDelegate);

    setCompleter(completer);

    bicValidator *const validator = new bicValidator(this);
    setValidator(validator);
}

KBicEdit::~KBicEdit()
{
    delete m_popupDelegate;
}

QFont bicItemDelegate::getSmallFont(const QStyleOptionViewItem& option) const
{
    QFont smallFont = option.font;
    smallFont.setPointSize(0.9*smallFont.pointSize());
    return smallFont;
}

QSize bicItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QFontMetrics metrics(option.font);
    QFontMetrics smallMetrics(getSmallFont(option));
    const QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

    // A bic has maximal 11 characters. So we guess, we want to display 11 characters. The name of the institution has to adapt to what is given
    return QSize(metrics.horizontalAdvance(QLatin1Char('X')) + 2 * margin,
                 metrics.lineSpacing() + smallMetrics.lineSpacing() + smallMetrics.leading() + 2 * margin);
}

/**
 * @todo enable eliding (use QFontMetrics::elidedText() )
 */
void bicItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Background
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    const int margin = style->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    const QRect textArea = QRect(opt.rect.x() + margin, opt.rect.y() + margin, opt.rect.width() - 2 * margin, opt.rect.height() - 2 * margin);

    // Paint name
    painter->save();
    QFont smallFont = getSmallFont(opt);
    QFontMetrics metrics(opt.font);
    QFontMetrics smallMetrics(smallFont);
    QRect nameRect = style->alignedRect(opt.direction, Qt::AlignBottom, QSize(textArea.width(), smallMetrics.lineSpacing()), textArea);
    painter->setFont(smallFont);
    style->drawItemText(painter, nameRect, Qt::AlignBottom, QApplication::palette(), true, index.model()->data(index, eIBANBIC::DisplayRole::InstitutionNameRole).toString(), option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Mid);
    painter->restore();

    // Paint BIC
    painter->save();
    QFont normal = painter->font();
    normal.setBold(true);
    painter->setFont(normal);
    QRect bicRect = style->alignedRect(opt.direction, Qt::AlignTop, QSize(textArea.width(), metrics.lineSpacing()), textArea);
    const QString bic = index.model()->data(index, Qt::DisplayRole).toString();
    style->drawItemText(painter, bicRect, Qt::AlignTop, QApplication::palette(), true, bic, option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text);

    painter->restore();
}
