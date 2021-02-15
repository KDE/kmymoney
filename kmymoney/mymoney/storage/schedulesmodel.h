/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCHEDULESMODEL_H
#define SCHEDULESMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneyschedule.h"

class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT SchedulesModel : public MyMoneyModel<MyMoneySchedule>
{
  Q_OBJECT

public:
  struct Column {
    enum {
      Name,
      Type = Name,
      Account,
      Payee,
      Amount,
      NextDueDate,
      Frequency,
      PaymentMethod,
      // insert new values above this line
      MaxColumns
    } Columns;
  };

  enum ColorScheme {
    Overdue,
    Finished
  };

  explicit SchedulesModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  virtual ~SchedulesModel();

  static const int ID_SIZE = 6;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;
  Qt::ItemFlags flags(const QModelIndex & index) const override;

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override;

  /// @todo simplify this to a QList<MyMoneySchedule> scheduleList() which returns
  /// all schedules and move the filtering into a specific QSortFilterProxyModel.
  /// For now, we keep this as it is and leave it for another day
  QList<MyMoneySchedule> scheduleList(const QString& accountId,
                                      eMyMoney::Schedule::Type type,
                                      eMyMoney::Schedule::Occurrence occurrence,
                                      eMyMoney::Schedule::PaymentType paymentType,
                                      const QDate& startDate,
                                      const QDate& endDate,
                                      bool overdue) const;

  /// @todo simplify this to a QList<MyMoneySchedule> scheduleList() which returns
  /// all schedules and move the filtering into a specific QSortFilterProxyModel.
  /// For now, we keep this as it is and leave it for another day
  QList<MyMoneySchedule> scheduleListEx(int scheduleTypes,
    int scheduleOcurrences,
    int schedulePaymentTypes,
    QDate date,
    const QStringList& accounts) const;


  void load(const QMap<QString, MyMoneySchedule>& list);
  void setColorScheme(ColorScheme scheme, const QColor& color);

protected:
  void clearModelItems() override;
  void doAddItem(const MyMoneySchedule& schedule, const QModelIndex& parentIdx = QModelIndex()) override;

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // SCHEDULESMODEL_H

