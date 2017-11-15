/***************************************************************************
                          kmymoneymvccombo.h  -  description
                             -------------------
    begin                : Mon Jan 09 2010
    copyright            : (C) 2010 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Cristian Onet <cristian.onet@gmail.com>
                           Alvaro Soliverez <asoliverez@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYMVCCOMBO_H
#define KMYMONEYMVCCOMBO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_widgets_export.h"
#include "register.h"
#include "mymoneytransactionfilter.h"

class QSortFilterProxyModel;
class MyMoneyPayee;
class MyMoneySchedule;
class MyMoneyTag;
class MyMoneyTransactionFilter;

/**
  * @author Cristian Onet
  * This class will replace the KMyMoneyCombo class when all widgets will use the MVC
  */
class KMM_WIDGETS_EXPORT KMyMoneyMVCCombo : public KComboBox
{
  Q_OBJECT
  Q_PROPERTY(QString selectedItem READ selectedItem WRITE setSelectedItem STORED false)

public:
  KMyMoneyMVCCombo(QWidget* parent = 0);
  explicit KMyMoneyMVCCombo(bool editable, QWidget* parent = 0);
  ~KMyMoneyMVCCombo();

  /**
    * @sa KLineEdit::setPlaceholderText()
    */
  void setPlaceholderText(const QString& hint) const;

  /**
    * This method returns the id of the first selected item.
    *
    * @return reference to QString containing the id. If no item
    *         is selected the QString will be empty.
    */
  const QString& selectedItem() const;

  /**
    * This method selects the item with the respective @a id.
    *
    * @param id reference to QString containing the id
    */
  void setSelectedItem(const QString& id);

  /**
    * Protect an entry from selection. Protection is controlled by
    * the parameter @p protect.
    *
    * @param id id of item for which to modify the protection
    * @param protect if true, the entry specified by @p accId cannot be
    *                selected. If false, it can be selected.
    *                Defaults to @p true.
    */
  void protectItem(int id, bool protect);

  /**
    * Make the completion match on any substring or only
    * from the start of an entry.
    *
    * @param enabled @a true turns on substring match, @a false turns it off.
    *                The default is @a false.
    */
  void setSubstringSearch(bool enabled);

  /**
    * set setSubstringSearch(enabled) of all children of widget
    *
    * @param widget a valid pointer (not 0)
    */
  static void setSubstringSearchForChildren(QWidget *const widget, bool enabled = false);

  /**
    * Reimplemented for internal reasons, no API change
    */
  void setEditable(bool editable);

protected slots:
  void activated(int index);

protected:
  /**
    * reimplemented to support detection of new items
    */
  void focusOutEvent(QFocusEvent*);

  /**
   * check if the current text is contained in the internal list, if not ask the user if want to create a new item.
   */
  virtual void checkCurrentText();

  /**
    * set the widgets text area based on the item with the given @a id.
    */
  virtual void setCurrentTextById(const QString& id);

  /**
    * Overridden for internal reasons, no API change
    */
  void connectNotify(const QMetaMethod & signal);

  /**
    * Overridden for internal reasons, no API change
    */
  void disconnectNotify(const QMetaMethod & signal);

  /**
    * overridden for internal reasons, no API change
    */
  void setCurrentText(const QString& txt = QString()) {
    KComboBox::setItemText(KComboBox::currentIndex(), txt);
  }

signals:
  void itemSelected(const QString& id);
  void objectCreation(bool);
  void createItem(const QString&, QString&);
  void lostFocus();

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;

  /**
    * This is just a cache to be able to implement the old interface.
    */
  mutable QString m_id;
};

/**
  * This class implements a text based payee selector.
  * When initially used, the widget has the functionality of a KComboBox object.
  * Whenever a key is pressed, the set of loaded payees is searched for
  * payees names which match the currently entered text.
  *
  * If any match is found a list selection box is opened and the user can use
  * the up/down, page-up/page-down keys or the mouse to navigate in the list. If
  * a payee is selected, the selection box is closed. Other key-strokes are
  * directed to the parent object to manipulate the text.  The visible contents of
  * the selection box is updated with every key-stroke.
  *
  * This object is a replacement of the kMyMoneyPayee object and should be used
  * for new code.
  *
  * @author Thomas Baumgart
  */
class KMM_WIDGETS_EXPORT KMyMoneyPayeeCombo : public KMyMoneyMVCCombo

{
  Q_OBJECT
public:
  KMyMoneyPayeeCombo(QWidget* parent = 0);

  void loadPayees(const QList<MyMoneyPayee>& list);
};

/**
  * This class implements a text based tag selector.
  * The widget has the functionality of a KMyMoneyPayeeCombo object.
  * Whenever a key is pressed, the set of loaded tags is searched for
  * tags names which match the currently entered text.
  *
  * @author Alessandro Russo
  */
class KMM_WIDGETS_EXPORT KMyMoneyTagCombo : public KMyMoneyMVCCombo
{
  Q_OBJECT
public:
  KMyMoneyTagCombo(QWidget* parent = 0);

  void loadTags(const QList<MyMoneyTag>& list);
  /** ids in usedIdList are escluded from the internal list
    * you should call loadTags before calling setUsedTagList because it doesn't readd
    * tag removed in previous call*/
  void setUsedTagList(QList<QString>& usedIdList, QList<QString>& usedTagNameList);

protected:
  /**
    * check if the current text is contained in the internal list, if not ask the user if want to create a new item.
    */
  virtual void checkCurrentText();

private:
  QList<QString> m_usedIdList;
  QList<QString> m_usedTagNameList;
  QList<QString> m_closedIdList;
  QList<QString> m_closedTagNameList;
};

/**
  * This class implements a tag label. It create a QFrame and inside it a QToolButton
  * with a 'X' Icon and a QLabel with the name of the Tag
  *
  * @author Alessandro Russo
  */
class KTagLabel : public QFrame
{
  Q_OBJECT
public:
  KTagLabel(const QString& id, const QString& name, QWidget* parent = 0);

signals:
  void clicked(bool);

private:
  QString m_tagId;
};

/**
  * This widget contain a KMyMoneyTagCombo widget and 0 or more KTagLabel widgets
  * call KMyMoneyTagCombo.loadTags with the correct list whenever a new KTagLabel is created or
  * deleted by removing or adding the relative tag
  *
  * @author Alessandro Russo
  */
class KMM_WIDGETS_EXPORT KTagContainer : public QWidget
{
  Q_OBJECT
public:
  KTagContainer(QWidget* parent = 0);

  void loadTags(const QList<MyMoneyTag>& list);
  KMyMoneyTagCombo* tagCombo() {
    return m_tagCombo;
  }
  const QList<QString> selectedTags();
  void addTagWidget(const QString& id);
  void RemoveAllTagWidgets();

protected slots:
  void slotRemoveTagWidget();
  void slotAddTagWidget();

private:
  KMyMoneyTagCombo *m_tagCombo;
  QList<KTagLabel*> m_tagLabelList;
  QList<QString> m_tagIdList;
  QList<QString> m_tagNameList;

  // A local cache of the list of all Tags, it's updated when loadTags is called
  QList<MyMoneyTag> m_list;
};

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible states for
  * reconciliation.
  */
class KMM_WIDGETS_EXPORT KMyMoneyReconcileCombo : public KMyMoneyMVCCombo
{
  Q_OBJECT
public:
  KMyMoneyReconcileCombo(QWidget *w = 0);

  void setState(eMyMoney::Split::State state);
  eMyMoney::Split::State state() const;
  void removeDontCare();

protected slots:
  void slotSetState(const QString&);
};

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible states for
  * actions (Deposit, Withdrawal, etc.).
  */
class KMM_WIDGETS_EXPORT KMyMoneyCashFlowCombo : public KMyMoneyMVCCombo
{
  Q_OBJECT
public:
  /**
    * Create a combo box that contains the entries "Pay to", "From" and
    * "  " for don't care.
    */
  explicit KMyMoneyCashFlowCombo(QWidget *w = 0, eMyMoney::Account type = eMyMoney::Account::Asset);

  void setDirection(KMyMoneyRegister::CashFlowDirection dir);
  KMyMoneyRegister::CashFlowDirection direction() const {
    return m_dir;
  }
  void removeDontCare();

protected slots:
  void slotSetDirection(const QString& id);

signals:
  void directionSelected(KMyMoneyRegister::CashFlowDirection);

private:
  KMyMoneyRegister::CashFlowDirection   m_dir;
};

/**
  * @author Thomas Baumgart
  * This class implements a combo box with the possible activities
  * for investment transactions (buy, sell, dividend, etc.)
  */
class KMM_WIDGETS_EXPORT KMyMoneyActivityCombo : public KMyMoneyMVCCombo
{
  Q_OBJECT
public:
  /**
    * Create a combo box that contains the entries "Buy", "Sell" etc.
    */
  KMyMoneyActivityCombo(QWidget *w = 0);

  void setActivity(eMyMoney::Split::InvestmentTransactionType activity);
  eMyMoney::Split::InvestmentTransactionType activity() const {
    return m_activity;
  }

protected slots:
  void slotSetActivity(const QString& id);

signals:
  void activitySelected(eMyMoney::Split::InvestmentTransactionType);

private:
  eMyMoney::Split::InvestmentTransactionType  m_activity;
};

class KMM_WIDGETS_EXPORT KMyMoneyGeneralCombo : public KComboBox
{
  Q_OBJECT
  Q_PROPERTY(int currentItem READ currentItem WRITE setCurrentItem STORED false)
public:
  KMyMoneyGeneralCombo(QWidget* parent = 0);
  virtual ~KMyMoneyGeneralCombo();

  void insertItem(const QString& txt, int id, int idx = -1);

  void setCurrentItem(int id);
  int currentItem() const;

  void removeItem(int id);

public slots:
  void clear();

signals:
  void itemSelected(int id);

protected:
  // prevent the caller to use the standard KComboBox insertItem function with a default idx
  void insertItem(const QString&);

protected slots:
  void slotChangeItem(int idx);

};

/**
 * This class implements a time period selector
 * @author Thomas Baumgart
 */
class KMM_WIDGETS_EXPORT KMyMoneyPeriodCombo : public KMyMoneyGeneralCombo
{
  Q_OBJECT
public:
  KMyMoneyPeriodCombo(QWidget* parent = 0);

  eMyMoney::TransactionFilter::Date currentItem() const;
  void setCurrentItem(eMyMoney::TransactionFilter::Date id);

  /**
   * This function returns the actual start date for the given
   * period definition given by @p id. For user defined periods
   * the returned value is QDate()
   */
  static QDate start(eMyMoney::TransactionFilter::Date id);

  /**
   * This function returns the actual end date for the given
   * period definition given by @p id. For user defined periods
   * the returned value is QDate()
   */
  static QDate end(eMyMoney::TransactionFilter::Date id);

  // static void dates(QDate& start, QDate& end, MyMoneyTransactionFilter::dateOptionE id);
};

/**
 * This class implements an occurrence selector
 * as a parent class for both OccurrencePeriod and Frequency combos
 *
 * @author Colin Wright
 */
class KMM_WIDGETS_EXPORT KMyMoneyOccurrenceCombo : public KMyMoneyGeneralCombo
{
  Q_OBJECT
public:
  KMyMoneyOccurrenceCombo(QWidget* parent = 0);

  eMyMoney::Schedule::Occurrence currentItem() const;
};

/**
 * This class implements an occurrence period selector
 *
 * @author Colin Wright
 */
class KMM_WIDGETS_EXPORT KMyMoneyOccurrencePeriodCombo : public KMyMoneyOccurrenceCombo
{
  Q_OBJECT
public:
  KMyMoneyOccurrencePeriodCombo(QWidget* parent = 0);

};

/**
 * This class implements a payment frequency selector
 * @author Thomas Baumgart
 */
class KMM_WIDGETS_EXPORT KMyMoneyFrequencyCombo : public KMyMoneyOccurrenceCombo
{
  Q_OBJECT

  Q_PROPERTY(QVariant data READ currentData WRITE setCurrentData STORED false)

public:
  KMyMoneyFrequencyCombo(QWidget* parent = 0);

  /**
   * This method returns the number of events for the selected payment
   * frequency (eg for yearly the return value is 1 and for monthly it
   * is 12). In case, the frequency cannot be converted (once, every other year, etc.)
   * the method returns 0.
   */
  int eventsPerYear() const;
  /**
   * This method returns the number of days between two events of
   * the selected frequency. The return value for months is based
   * on 30 days and the year is 360 days long.
   */
  int daysBetweenEvents() const;

  QVariant currentData() const;

  void setCurrentData(QVariant data);

Q_SIGNALS:
  void currentDataChanged(QVariant data);

protected slots:
  void slotCurrentDataChanged();

private:
  QVariant data;

};
#endif
