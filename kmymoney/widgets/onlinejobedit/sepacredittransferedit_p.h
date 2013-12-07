#ifndef SEPACREDITTRANSFEREDIT_P_H
#define SEPACREDITTRANSFEREDIT_P_H

#include <QtGui/QValidator>

/**
  * @brief Validator for IBAN
  * A check if this code actualy exists is not done
  */
class ibanValidator : public QValidator
{
  Q_OBJECT
public:
  explicit ibanValidator(QObject* parent = 0)
    : QValidator(parent) {}
  State validate(QString &string, int &pos) const;
};

/**
  * @brief Validator for BIC
  * A check if this code actualy exists is not done
  */
class bicValidator : public QValidator
{
  Q_OBJECT
public:
  explicit bicValidator(QObject* parent = 0)
    : QValidator(parent) {}
  State validate(QString &string, int &pos) const;
};

#endif // SEPACREDITTRANSFEREDIT_P_H
