/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2004 Martin Preuss aquamaniac @users.sourceforge.net
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KBPICKSTARTDATE_H
#define KBPICKSTARTDATE_H

#include <QDateTime>
#include <QDialog>

class KBankingExt;

/**
  * Class derived from QBPickStartDate and modified to
  * be based on KDE widgets
  *
  * @author Martin Preuss
  * @author Thomas Baumgart
  */
class KBPickStartDate : public QDialog
{
    Q_OBJECT
public:
    KBPickStartDate(KBankingExt* qb,
                    const QDate &firstPossible,
                    const QDate &lastUpdate,
                    const QString& accountName,
                    int defaultChoice,
                    QWidget* parent = 0,
                    bool modal = false);
    ~KBPickStartDate();

    QDate date();

public Q_SLOTS:
    void slotHelpClicked();

private:
    /// \internal d-pointer class.
    struct Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif
