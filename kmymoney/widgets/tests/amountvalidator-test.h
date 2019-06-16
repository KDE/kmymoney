/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMOUNTVALIDATORTEST_H
#define AMOUNTVALIDATORTEST_H

#include <QObject>
#include <QLocale>
#include <QString>

class AmountValidator;

class AmountValidatorTest : public QObject
{
    Q_OBJECT

protected:
    QLocale         defaultLocale;
    QString         currentLocale;

private Q_SLOTS:
    void init();
    void cleanup();
    void testValidator_data();
    void testValidator();

private:
    void setLocale(const QString& name);
    void addAcceptableNumber(const QString& testCaseName, const QString& number);
    void addInvalidNumber(const QString& testCaseName, const QString& number);
};

#endif
