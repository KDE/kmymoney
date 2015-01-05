#include "gwenhywfarqtoperators.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

GWEN_STRINGLIST* GWEN_StringList_fromQStringList(const QStringList& input)
{
  GWEN_STRINGLIST *ret = GWEN_StringList_new();
  QString line;
  foreach( line, input ) {
    GWEN_StringList_AppendString(ret, line.toUtf8().constData(), false, false);
  }
  return ret;
}

GWEN_STRINGLIST* GWEN_StringList_fromQString( const QString& input )
{
  GWEN_STRINGLIST *ret = GWEN_StringList_new();
  GWEN_StringList_AppendString(ret, input.toUtf8().constData(), false, false);
  return ret;
}
