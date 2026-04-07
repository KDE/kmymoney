/*
    SPDX-FileCopyrightText: 2026 Junie <junie@jetbrains.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HOMEMODEL_H
#define HOMEMODEL_H

#include "mymoneymoney.h"
#include <QAbstractListModel>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtQml>
#include <memory>

class MoneyFormatter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
public:
    explicit MoneyFormatter(QObject* parent = nullptr)
        : QObject(parent)
    {
    }
    Q_INVOKABLE QString format(const QString& amount, const QString& accountId = QString()) const;
};

class HomeSection : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(int type READ type CONSTANT)
    Q_PROPERTY(bool visible READ isVisible NOTIFY visibilityChanged)

public:
    explicit HomeSection(const QString& title, int type, QObject* parent = nullptr)
        : QObject(parent)
        , m_title(title)
        , m_type(type)
        , m_visible(true)
    {
    }
    virtual ~HomeSection() = default;

    QString title() const
    {
        return m_title;
    }
    int type() const
    {
        return m_type;
    }
    bool isVisible() const
    {
        return m_visible;
    }
    void setVisible(bool visible)
    {
        if (m_visible != visible) {
            m_visible = visible;
            Q_EMIT visibilityChanged();
        }
    }

Q_SIGNALS:
    void visibilityChanged();
    void dataChanged();

private:
    QString m_title;
    int m_type;
    bool m_visible;
};

class HomeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles { TitleRole = Qt::UserRole + 1, TypeRole, SectionObjectRole, VisibleRole };

    explicit HomeModel(QObject* parent = nullptr);
    ~HomeModel() override;

    Q_PROPERTY(bool isReady READ isReady NOTIFY isReadyChanged)
    bool isReady() const
    {
        return m_isReady;
    }
    void setReady(bool ready)
    {
        if (m_isReady != ready) {
            m_isReady = ready;
            qWarning() << "HomeModel::setReady(" << ready << ")";
            Q_EMIT isReadyChanged();
        }
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addSection(HomeSection* section);
    void clear();
    void refresh();

    enum SectionType { Accounts, Schedules, Forecast, NetWorthGraph, AssetsLiabilities, Budget, CashFlow };

Q_SIGNALS:
    void isReadyChanged();

private:
    QList<HomeSection*> m_sections;
    bool m_isReady = false;
};

class AccountsSection : public HomeSection
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_PROPERTY(QVariantList accounts READ accounts NOTIFY dataChanged)
    Q_PROPERTY(bool showCurrentBalance READ showCurrentBalance CONSTANT)
    Q_PROPERTY(bool showTotalBalance READ showTotalBalance CONSTANT)

public:
    struct AccountData {
        QString id;
        QString name;
        QString institution;
        QString balance;
        QString totalBalance;
        QString color;
        QString totalColor;
        int status; // e.g. number of transactions
        QString lastReconciliation;
    };

    enum AccountFilter { PreferredAccounts, PaymentAccounts, AllAccounts };

    AccountsSection(const QString& title, AccountFilter filter, QObject* parent = nullptr);

    QVariantList accounts() const;
    bool showCurrentBalance() const
    {
        return m_showCurrentBalance;
    }
    bool showTotalBalance() const
    {
        return m_showTotalBalance;
    }

    void setFilter(AccountFilter filter)
    {
        m_filter = filter;
        Q_EMIT dataChanged();
    }
    void setShowBalances(bool current, bool total)
    {
        m_showCurrentBalance = current;
        m_showTotalBalance = total;
        Q_EMIT dataChanged();
    }

private:
    AccountFilter m_filter;
    bool m_showCurrentBalance = true;
    bool m_showTotalBalance = true;
};

class SchedulesSection : public HomeSection
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_PROPERTY(QVariantList schedules READ schedules NOTIFY dataChanged)

public:
    SchedulesSection(const QString& title, QObject* parent = nullptr);
    QVariantList schedules() const;
};

class AssetsLiabilitiesSection : public HomeSection
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_PROPERTY(QVariantList assets READ assets NOTIFY dataChanged)
    Q_PROPERTY(QVariantList liabilities READ liabilities NOTIFY dataChanged)
    Q_PROPERTY(QString netWorth READ netWorth NOTIFY dataChanged)
    Q_PROPERTY(QString netWorthColor READ netWorthColor NOTIFY dataChanged)

public:
    AssetsLiabilitiesSection(const QString& title, QObject* parent = nullptr);
    QVariantList assets() const;
    QVariantList liabilities() const;
    QString netWorth() const;
    QString netWorthColor() const;
};

#endif
