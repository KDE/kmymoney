/*
    SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FANCYDATEGROUPMARKERS_H
#define FANCYDATEGROUPMARKERS_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "fancydategroupmarker.h"

namespace eWidgets {
enum class SortField;
namespace Transaction {
enum class Column;
}
namespace Register {
enum class DetailColumn;
}
}
namespace eMyMoney {
namespace Account {
enum class Type;
}
}

namespace KMyMoneyRegister
{
class Register;

class StatementGroupMarkerPrivate;
class KMM_OLDREGISTER_EXPORT StatementGroupMarker : public FancyDateGroupMarker
{
    Q_DISABLE_COPY(StatementGroupMarker)

public:
    explicit StatementGroupMarker(Register* getParent, eWidgets::eRegister::CashFlowDirection dir, const QDate& date, const QString& txt);
    ~StatementGroupMarker() override;

    eWidgets::eRegister::CashFlowDirection sortType() const override;
    int sortSamePostDate() const override;

private:
    Q_DECLARE_PRIVATE(StatementGroupMarker)
};


class KMM_OLDREGISTER_EXPORT SimpleDateGroupMarker : public FancyDateGroupMarker
{
    Q_DISABLE_COPY(SimpleDateGroupMarker)

public:
    explicit SimpleDateGroupMarker(Register* getParent, const QDate& date, const QString& txt);
    ~SimpleDateGroupMarker() override;

    void paintRegisterCell(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;
    int rowHeightHint() const override;
    const char* className() override;
};

class KMM_OLDREGISTER_EXPORT FiscalYearGroupMarker : public FancyDateGroupMarker
{
    Q_DISABLE_COPY(FiscalYearGroupMarker)

public:
    explicit FiscalYearGroupMarker(Register* getParent, const QDate& date, const QString& txt);
    ~FiscalYearGroupMarker() override;

    const char* className() override;
    int sortSamePostDate() const override;
};

} // namespace

#endif
