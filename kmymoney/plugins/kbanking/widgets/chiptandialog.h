/*
    A tan input dialog for optical chipTan used in online banking
    SPDX-FileCopyrightText: 2014 Christian David <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

#ifndef CHIPTANDIALOG_H
#define CHIPTANDIALOG_H

#include <memory>

#include <QDialog>

namespace Ui
{
class chipTanDialog;
}

class chipTanDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString infoText READ infoText() WRITE setInfoText CONSTANT)
    Q_PROPERTY(QString hhdCode READ hhdCode() WRITE setHhdCode CONSTANT)
    Q_PROPERTY(int flickerFieldWidth READ flickerFieldWidth WRITE setFlickerFieldWidth CONSTANT)

public:
    explicit chipTanDialog(QWidget* parent = 0);
    ~chipTanDialog();

    enum Result { Accepted = 0, Rejected, InternalError };

    QString infoText();
    QString hhdCode();
    QString tan();
    int flickerFieldWidth();

public Q_SLOTS:
    void accept() final override;
    void reject() final override;

    void setInfoText(const QString&);
    void setHhdCode(const QString&);

    void setTanLimits(const int& minLength, const int& maxLength);
    void setFlickerFieldWidth(const int& width);
    void setFlickerFieldClockSetting(const int& width);

private Q_SLOTS:
    void tanInputChanged(const QString&);
    void flickerFieldWidthChanged(const int& width);
    void flickerFieldClockSettingChanged(const int& takt);

private:
    std::unique_ptr<Ui::chipTanDialog> ui;
    QString m_tan;
    bool m_accepted;

    void setRootObjectProperty(const char* property, const QVariant& value);
};

#endif // CHIPTANDIALOG_H
