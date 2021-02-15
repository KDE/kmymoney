/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPECIALDATESMODEL_H
#define SPECIALDATESMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneymodel.h"

class /* no export here on purpose */ SpecialDateEntry
{
public:
  explicit SpecialDateEntry() {}
  explicit SpecialDateEntry(const QString& id, const SpecialDateEntry& other)
  : m_id(id)
  , m_txt(other.m_txt)
  , m_date(other.m_date)
  {}

  SpecialDateEntry(const QString& id, const QDate& date, const QString& txt)
  : m_id(id)
  , m_txt(txt)
  , m_date(date)
  {}

  inline const QString& id() const { return m_id; }
  inline const QString& txt() const { return m_txt; }
  inline const QDate& date() const { return m_date; }
  inline bool hasReferenceTo(const QString&) const { return false; }

  /**
   * @copydoc MyMoneyObject::referencedObjects
   */
  inline QSet<QString> referencedObjects() const { return {}; }

private:
  QString     m_id;
  QString     m_txt;
  QDate       m_date;
};

class QUndoStack;
/**
  */
class KMM_MYMONEY_EXPORT SpecialDatesModel : public MyMoneyModel<SpecialDateEntry>
{
  Q_OBJECT

public:
  explicit SpecialDatesModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  ~SpecialDatesModel();

  static const int ID_SIZE = 2;

  int columnCount (const QModelIndex & parent) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  void setOptions(bool showDateHeaders, const QDate& firstFiscalDate);

public Q_SLOTS:
  void load();

private:
  struct Private;
  QScopedPointer<Private> d;
};

#endif // SPECIALDATESMODEL_H

