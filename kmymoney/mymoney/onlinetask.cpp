#include "onlinetask.h"

#include "mymoneyfile.h"

ONLINETASK_META_INIT(onlineTask);

onlineTask::onlineTask()
{}

onlineTask::onlineTask(const onlineTask &other)
{
  Q_UNUSED(other);
}

/**void onlineJob::setState( jobState new_state )
{
  m_state = new_state;
}*/

onlineTask::settings::~settings()
{}

/** @todo give life */
void onlineTask::writeXML(QDomDocument &document, QDomElement &parent) const
{
  Q_UNUSED(document);
  Q_UNUSED(parent);
}
