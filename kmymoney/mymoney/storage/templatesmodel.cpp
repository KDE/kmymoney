/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// just make sure that the assertions always work in this model
#ifndef QT_FORCE_ASSERTS
#define QT_FORCE_ASSERTS
#endif

#include "templatesmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>
#include <QFont>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyenums.h"

#include "icons.h"

struct TemplatesModel::Private
{
  Q_DECLARE_PUBLIC(TemplatesModel)

  Private(TemplatesModel* qq, QObject* parent)
    : q_ptr(qq)
    , parentObject(parent)
  {
  }


  TemplatesModel*                  q_ptr;
  QObject*                        parentObject;
};

TemplatesModel::TemplatesModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<MyMoneyTemplate>(parent, QStringLiteral("TMPL"), TemplatesModel::ID_SIZE, undoStack)
  , d(new Private(this, parent))
{
  setObjectName(QLatin1String("TemplatesModel"));

  useIdToItemMapper(true);

  // force creation of empty template structure
  unload();
}

TemplatesModel::~TemplatesModel()
{
}

int TemplatesModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return Column::MaxColumns;
}

QVariant TemplatesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case Column::Type:
        return i18nc("@title:column Country/Hierarchy type", "Type");
      case Column::Description:
        return i18nc("@title:column Column heading for description", "Description");
      default:
        return QVariant();
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

QVariant TemplatesModel::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  const MyMoneyTemplate& tmpl = static_cast<TreeItem<MyMoneyTemplate>*>(idx.internalPointer())->constDataRef();

  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        case Column::Type:
          // make sure to never return any displayable text for the dummy entry
          return tmpl.title();

        case Column::Description:
          return tmpl.shortDescription();

        default:
          break;
      }
      break;

    case eMyMoney::Model::TemplatesCountryRole:
    case eMyMoney::Model::TemplatesTypeRole:
      return tmpl.title();

    case eMyMoney::Model::TemplatesDescriptionRole:
      return tmpl.shortDescription();

    case eMyMoney::Model::TemplatesLongDescriptionRole:
      return tmpl.longDescription();

    case eMyMoney::Model::TemplatesLocaleRole:
      return tmpl.locale();

    case Qt::TextAlignmentRole:
      return QVariant(Qt::AlignLeft | Qt::AlignVCenter);

    case eMyMoney::Model::IdRole:
      return tmpl.id();

  }
  return QVariant();
}

Qt::ItemFlags TemplatesModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemFlags();
  if (index.row() < 0 || index.row() >= rowCount(index.parent()))
    return Qt::ItemFlags();

  // we don't allow to select the country entries of the model
  return index.parent().isValid() ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable) : Qt::ItemIsEnabled;
}


bool TemplatesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if(!index.isValid()) {
    return false;
  }

  MyMoneyTemplate& tmpl = static_cast<TreeItem<MyMoneyTemplate>*>(index.internalPointer())->dataRef();

  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(index.column()) {
        case Column::Type:
          tmpl.setTitle(value.toString());
          return true;

        case Column::Description:
          tmpl.setShortDescription(value.toString());
          return true;

        default:
          break;
      }
      break;

    case eMyMoney::Model::TemplatesCountryRole:
      tmpl.setTitle(value.toString());
      return true;

    case eMyMoney::Model::TemplatesDescriptionRole:
      tmpl.setShortDescription(value.toString());
      return true;

    case eMyMoney::Model::TemplatesLongDescriptionRole:
      tmpl.setLongDescription(value.toString());
      return true;

    case eMyMoney::Model::TemplatesLocaleRole:
      tmpl.setLocale(value.toString());
      return true;

    default:
      if (role >= Qt::UserRole) {
        qDebug() << "setData(" << index.row() << index.column() << ")" << value << role;
      }
      break;
  }
  return QAbstractItemModel::setData(index, value, role);
}

void TemplatesModel::addItem(MyMoneyTemplate& tmpl, const QModelIndex& parentIdx)
{
  if (parentIdx.isValid()) {
    tmpl = MyMoneyTemplate(nextId(), tmpl);
    doAddItem(tmpl, parentIdx);
  }
}

