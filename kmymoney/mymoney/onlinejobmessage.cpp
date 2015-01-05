#include "onlinejobmessage.h"

onlineJobMessage::onlineJobMessage()
  : m_type( error ),
    m_sender ( QString() ),
    m_message( QString() ),
    m_timestamp( QDateTime() )
{
}
