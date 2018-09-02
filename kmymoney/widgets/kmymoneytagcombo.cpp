/*
 * Copyright 2010-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2010-2016  Cristian Oneț <onet.cristian@gmail.com>
 * Copyright 2010       Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "kmymoneytagcombo.h"
#include "kmymoneymvccombo_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytag.h"

class KMyMoneyTagComboPrivate : public KMyMoneyMVCComboPrivate
{
  Q_DISABLE_COPY(KMyMoneyTagComboPrivate)

public:
  KMyMoneyTagComboPrivate() :
    KMyMoneyMVCComboPrivate()
  {
  }

  QList<QString> m_usedIdList;
  QList<QString> m_usedTagNameList;
  QList<QString> m_closedIdList;
  QList<QString> m_closedTagNameList;
};

KMyMoneyTagCombo::KMyMoneyTagCombo(QWidget* parent) :
    KMyMoneyMVCCombo(*new KMyMoneyTagComboPrivate, true, parent)
{
}

KMyMoneyTagCombo::~KMyMoneyTagCombo()
{
}

void KMyMoneyTagCombo::loadTags(const QList<MyMoneyTag>& list)
{
  Q_D(KMyMoneyTagCombo);
  clear();

  //add a blank item, since the field is optional
  addItem(QString(), QVariant(QString()));

  //add all not closed tags
  QList<MyMoneyTag>::const_iterator it;
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    if (!(*it).isClosed())
      addItem((*it).name(), QVariant((*it).id()));
    else {
      d->m_closedIdList.append((*it).id());
      d->m_closedTagNameList.append((*it).name());
    }
  }

  //sort the model, which will sort the list in the combo
  model()->sort(Qt::DisplayRole, Qt::AscendingOrder);

  //set the text to empty and the index to the first item on the list
  setCurrentIndex(0);
  clearEditText();
}

void KMyMoneyTagCombo::setUsedTagList(QList<QString>& usedIdList, QList<QString>& usedTagNameList)
{
  Q_D(KMyMoneyTagCombo);
  d->m_usedIdList = usedIdList;
  d->m_usedTagNameList = usedTagNameList;
  for (auto i = 0; i < d->m_usedIdList.size(); ++i) {
    int index = findData(QVariant(d->m_usedIdList.at(i)), Qt::UserRole, Qt::MatchExactly);
    if (index != -1) removeItem(index);
  }
}

void KMyMoneyTagCombo::checkCurrentText()
{
  Q_D(KMyMoneyTagCombo);
  if (!contains(currentText())) {
    if (d->m_closedTagNameList.contains(currentText())) {
      // Tell the user what's happened
      QString msg = QString("<qt>") + i18n("Closed tags cannot be used.") + QString("</qt>");
      KMessageBox::information(this, msg, i18n("Closed tag"), "Closed tag");
      setCurrentText();
      return;
    } else if (d->m_usedTagNameList.contains(currentText())) {
      // Tell the user what's happened
      QString msg = QString("<qt>") + i18n("The tag is already present.") + QString("</qt>");
      KMessageBox::information(this, msg, i18n("Duplicate tag"), "Duplicate tag");
      setCurrentText();
      return;
    }
    QString id;
    // annouce that we go into a possible dialog to create an object
    // This can be used by upstream widgets to disable filters etc.
    emit objectCreation(true);

    emit createItem(currentText(), id);

    // Announce that we return from object creation
    emit objectCreation(false);

    // update the field to a possibly created object
    //m_id = id;
    addEntry(currentText(), id);
    setCurrentTextById(id);
  }
}
