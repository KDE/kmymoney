/***************************************************************************
                          tocitem.h  -  description
                             -------------------
    begin                : Sat Jul 03 2010
    copyright            : (C) Bernd Gonsior
    email                : bernd.gonsior@googlemail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef TOCITEM_H
#define TOCITEM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>
#include <QTreeWidgetItem>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

/**
 * Base class for items in reports table of contents (TOC).
 * It provides the type of the item (reportgroup or report)
 * and an operator for sorting.
 */
class TocItem : public QTreeWidgetItem
{
public:

  /** Type of TOC-item */
  enum ItemType {
    /** item represents a reportgroup */
    GROUP  = QTreeWidgetItem::UserType + 10,
    /** item represents a report */
    REPORT = QTreeWidgetItem::UserType + 20
  } type;

  /** Constructor.
   *
   * @param parent pointer to the parent QWidget
   * @param columns List of texts in columns
   */
  TocItem(QTreeWidget* parent, QStringList columns);

  /** Constructor.
   *
   * @param parent pointer to the parent QWidget
   * @param columns List of texts in columns
   */
  TocItem(QTreeWidgetItem* parent, QStringList columns);

  /** Indicates, whether the item represents a report or a reportgroup.
   *
   * @retval true  the item represents a report
   * @retval false the item represents a reportgroup
   */
  bool isReport();

private:

  /** Operator used to sort TocItems.
   * TOC has to be sorted in a quite special way:
   * @li @c reportgroups  numerically by group-number
   * @li @c reports       alphabetically by text of column 0
   *
   * Because the operator is defined @c const it is not possible,
   * to use a property of a class derived from @c QTreeWidgetItem.
   * So we use the @c QVariant data of @c QTreeWidgetItem in following way:
   *
   * QVariant contains a QStringList at position 0 with
   * role @c Qt::UserRole.
   * The first entry of this list is the item-type (report or
   * reportgroup).  The second entry is the item-type-specific sort-key, for
   * reports simply the text of column 0, for reportgroups the groupnumber as
   * string with leading zeros.
   *
   * Examples:
   * <pre>
   * reportgroup:
   *  list.at(0) = QString::number(TocItem::GROUP);
   *  list.at(1) = "001"
   *
   * report:
   *  list.at(0) = QString::number(TocItem::REPORT);
   *  list.at(1) = "<the-name-of-the-report>"
   * </pre>
   */
  bool operator<(const QTreeWidgetItem &other) const final override;
};

#endif
